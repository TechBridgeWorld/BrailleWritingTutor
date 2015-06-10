/*
 * Braille Tutor interface library
 * ShortStylusSuppressor.cc, started 6 November 2007 by Tom Stepleton.
 *                                                      (tss@ri.cmu.edu)
 * Code that attempts to debounce BrailleTutor stylus events.
 */

#include <list>
#include <deque>
#include <vector>

#include "Types.h"

#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "ShortStylusSuppressor.h"

namespace BrailleTutorNS {

/////////////////////////
//// TIME MANAGEMENT ////
/////////////////////////

//! Abstract superclass for a class that handles timeouts
struct TimeoutHandler {
  //! Will be called when a Timeout times out
  virtual void handleTimeout() = 0;

  //! For g++
  inline virtual ~TimeoutHandler() { }
};

//! This is the thread functor that actually waits out a timeout
struct FunctorTimeout {
  //! Reference to when the timeout is
  boost::xtime &timeout;
  //! Reference to the mutex for timeout
  boost::mutex &mutex_timeout;
  //! Reference to the condition variable for timeout
  boost::condition &cond_timeout;

  //! Reference to the timeout handler pointer
  TimeoutHandler*& handler;
  //! Mutex for the timeout handler
  boost::mutex &mutex_handler;
 
  //! Constructor---fill in references
  inline FunctorTimeout(boost::xtime &my_timeout,
			boost::mutex &my_mutex_timeout,
			boost::condition &my_cond_timeout,
			TimeoutHandler* &my_handler,
			boost::mutex &my_mutex_handler)
  : timeout(my_timeout), mutex_timeout(my_mutex_timeout),
    cond_timeout(my_cond_timeout), handler(my_handler),
    mutex_handler(my_mutex_handler) { }

  // Perform this functor's function
  inline void operator()()
  {
    // Grab the timeout variable. Yep, we do this right away! We'll be
    // spending most of our time waiting on a condition variable, though.
    boost::mutex::scoped_lock lock_t(mutex_timeout);

    // Loop forever
    for(;;) {
      // First make a copy of the timeout variable. Dunno if this is
      // necessary, but why risk it?
      boost::xtime timeout_copy = timeout;

      // If the timeout seconds field is 0, abort the thread.
      if((timeout_copy.sec == 0) && (timeout_copy.nsec == 0)) return;

      // Wait for the timeout to time out. If the wait returns true, we
      // were interrupted and there's a new timeout value waiting for
      // us. Otherwise, we've hit the end of the timeout and we need to
      // call the handler.
//std::cerr << "starting wait at " << TimeInterval::now() << std::endl;
      if(cond_timeout.timed_wait(lock_t, timeout_copy)) continue;
//std::cerr << "left wait at " << TimeInterval::now() << std::endl;

      // Now set a timeout waaaay off in the distant future
      timeout = muchLater();

      // Unlock the timeout variable while we call the handler,
      // then lock it back after that's done.
      boost::mutex::scoped_lock lock_h(mutex_handler);
      if(handler) {
	lock_t.unlock();
	handler->handleTimeout();
	lock_t.lock();
      }

      // And that's it. Loop 'round and get set for the next timeout.
    }
  }

  //! Returns a time that's way off from now.
  inline static boost::xtime muchLater()
  {
    // First get the time right now
    boost::xtime later;
    boost::xtime_get(&later, boost::TIME_UTC_);	// Gary: TIME_UTC to TIME_UTC_
    // Then add an interval. NB: This is exactly three weeks! Don't
    // let a program using this run for more than three weeks!
    later.sec += 1814400;
    return later;
  }
};

//! Calls a designated method when a specified timeout expires

//! Register a TimeoutHandler with a Timeout object and specify an interval
//! to wait. When this interval passes, the TimeoutHandler's handleTimeout()
//! method is called.
class Timeout : public boost::noncopyable {
private:
  //! When the timeout is
  boost::xtime timeout;
  //! Mutex for timeout
  boost::mutex mutex_timeout;
  //! Condition variable for timeout
  boost::condition cond_timeout;

  //! The timeout handler
  TimeoutHandler *handler;
  //! Mutex for the timeout handler
  boost::mutex mutex_handler;

  //! FunctorTimeout thread---actually does the waiting
  boost::scoped_ptr<boost::thread> t_ft;

public:
  //! Constructor: start thread
  inline Timeout()
  : timeout(FunctorTimeout::muchLater()), handler(NULL),
    t_ft(new boost::thread(
      FunctorTimeout(timeout, mutex_timeout, cond_timeout,
		     handler, mutex_handler))) { }

  //! Constructor: start thread and set TimeoutHandler
  inline Timeout(TimeoutHandler &my_handler)
  : timeout(FunctorTimeout::muchLater()), handler(&my_handler),
    t_ft(new boost::thread(
      FunctorTimeout(timeout, mutex_timeout, cond_timeout,
		     handler, mutex_handler))) { }

  //! Set the timeout handler. (Does not copy---sets a reference)
  inline void setTimeoutHandler(TimeoutHandler &my_handler)
  {
    // Grab handler mutex and change handler
    boost::mutex::scoped_lock lock_h(mutex_handler);
    handler = &my_handler;
  }

  //! Clear the timeout handler.
  inline void clearTimeoutHandler()
  {
    // Grab handler mutex and change handler
    boost::mutex::scoped_lock lock_h(mutex_handler);
    handler = NULL;
  }

  //! Set timeout---arg is how far in the future timeout happens
  inline void setTimeout(const TimeInterval &my_timeout)
  {
//std::cerr << "called to wait for " << my_timeout << std::endl;
    // First get the time right now
    boost::xtime time_now;
    boost::xtime_get(&time_now, boost::TIME_UTC_);

    // Grab timeout mutex
    boost::mutex::scoped_lock lock_t(mutex_timeout);

    // Figure out the future timeout time
    timeout = time_now;
    timeout.sec += my_timeout.secs;
    const unsigned int nsecs = my_timeout.msecs * 1000000;
    timeout.nsec += nsecs % 1000000000;
    timeout.sec  += nsecs / 1000000000;

    // Notify the waiting thread
    cond_timeout.notify_one();
  }

  //! Clear timeout---or set it to sometime in the distant future
  inline void clearTimeout()
  {
    // Grab timeout mutex
    boost::mutex::scoped_lock lock_t(mutex_timeout);

    // Set to the future
    timeout = FunctorTimeout::muchLater();

    // Notify the waiting thread
    cond_timeout.notify_one();
  }

  //! Destructor: kill thread
  inline ~Timeout()
  {
    { // ENCLOSING BLOCK: For grabbing timeout mutex
    boost::mutex::scoped_lock lock_t(mutex_timeout);

    // Set timeout to 0 and notify the wait thread
    timeout.sec = 0; timeout.nsec = 0;
    cond_timeout.notify_one();
    } // END ENCLOSING BLOCK

    // Wait for the thread to die
    if(t_ft) t_ft->join();
  }
};

///////////////////////////////////
//// ShortStylusSuppressorCore ////
///////////////////////////////////

//! The thread functor that calls the BaseIOEventHandler callback on new events
struct FunctorNewEventsSSSC {
  //! Reference to new events decoded by the decoder thread
  std::deque<BaseIOEvent> &new_bevents;
  //! Reference to mutex for new BaseIOEvent events
  boost::mutex &mutex_new_bevents;
  //! Reference to condition variable for new BaseIOEvent events
  boost::condition &cond_new_bevents;
  //! Reference to the ShortStylusSuppressor object we're manipulating
  ShortStylusSuppressor &sss;

  //! Constructor: fill in references
  inline FunctorNewEventsSSSC(std::deque<BaseIOEvent> &my_new_bevents,
			      boost::mutex &my_mutex_new_bevents,
			      boost::condition &my_cond_new_bevents,
			      ShortStylusSuppressor &my_sss)
  : new_bevents(my_new_bevents), mutex_new_bevents(my_mutex_new_bevents),
    cond_new_bevents(my_cond_new_bevents), sss(my_sss) { }

  //! Perform this functor's function.
  inline void operator()()
  {
    // For insight, see FunctorNewEvents in BrailleTutor.cc
    for(;;) {
      { // ENCLOSING BLOCK: For getting and transferring new events
      boost::mutex::scoped_lock lock_n(mutex_new_bevents);

      if(new_bevents.empty()) {
	cond_new_bevents.wait(lock_n);

	// This method of calling for quitting is deprecated
        if(new_bevents.empty())
	  new_bevents.push_back(BaseIOEvent::makeDoneEvent());
      }

      boost::mutex::scoped_lock lock_b(sss.out_events_mutex);

      std::deque<BaseIOEvent>::const_iterator i;
      for(i=new_bevents.begin(); i!=new_bevents.end(); ++i) {
	sss.out_events.push_back(*i);
	if(i->type == BaseIOEvent::DONE) {
	  if(sss.handler) (*sss.handler)(sss.out_events);
	  return;
	}
      }

      // Clear out new events queue
      new_bevents.clear();
      } // END ENCLOSING BLOCK

      // Calling the BaseIOEventHandler, if it exists
      if(sss.handler) {
	boost::mutex::scoped_lock lock_b(sss.out_events_mutex);
	(*sss.handler)(sss.out_events);
      }
    }
  }
};


//! The actual guts of a ShortStylusSuppressor
class ShortStylusSuppressorCore
: public boost::noncopyable, public TimeoutHandler {
private:
  //! ShortStylusSuppressor object whose guts we manipulate
  ShortStylusSuppressor &sss;

  //! Freshly filtered BaseIOEvents
  std::deque<BaseIOEvent> new_bevents;
  //! Mutex for new_bevents
  boost::mutex mutex_new_bevents;
  //! Condition variable for new_bevents
  boost::condition cond_new_bevents;

  //! Minimum insertion/extraction time threshold for debouncing
  TimeInterval threshold;
  //! Mutex for the time threshold
  boost::mutex mutex_threshold;

  //! Timeout that we use to measure the debouncing interval.
  Timeout timeout;

  //! The queue of stylus BaseIOEvents that we filter when debouncing

  //! A list so that iterators can still be valid after erasure.
  std::list<BaseIOEvent> stylus_bevents;

  //! Mutex for stylus events
  boost::mutex mutex_stylus_bevents;

  //! Thread for dispatching new BaseIOEvents to the interface
  boost::scoped_ptr<boost::thread> t_new_bevents;

  //! Pops "debounced" events off the list and adds them to the filtered
  //! BaseIOEvents queue.
  inline void findAndReportDebounced()
  {
    // Debounced events are those in the stylus_bevents queue that are older
    // than now minus the debouncing threshold
    TimeInterval survival_time = TimeInterval::now() - threshold;

    // Grab stylus events mutex
    boost::mutex::scoped_lock lock_s(mutex_stylus_bevents);

    // Some checks to keep us from wasting our time
    if(stylus_bevents.empty()) return;
    if(stylus_bevents.front().timestamp > survival_time) return;

    // Grab new BaseIOEvent queue mutex and stuff in new events
    unsigned int new_bevents_enqueued = 0;
    boost::mutex::scoped_lock lock_n(mutex_new_bevents);
    while((!stylus_bevents.empty()) &&
	  (stylus_bevents.front().timestamp <= survival_time)) {
/*
std::cerr << "<<popping out event " << stylus_bevents.front().type
	  << ", cell " << stylus_bevents.front().cell
	  << ", dot " << stylus_bevents.front().dot << ">>" << std::endl;
*/
      new_bevents.push_back(stylus_bevents.front());
      stylus_bevents.pop_front();
      ++new_bevents_enqueued;
    }

    // If new events are waiting, Notify event reporter
    if(new_bevents_enqueued > 0) cond_new_bevents.notify_one();
  }

public:
  //! Constructor---start thread
  inline ShortStylusSuppressorCore(ShortStylusSuppressor &my_sss,
				   const TimeInterval &my_threshold)
  : sss(my_sss), threshold(my_threshold), timeout(*this),
    t_new_bevents(
      new boost::thread(
	FunctorNewEventsSSSC(
	  new_bevents, mutex_new_bevents, cond_new_bevents, sss))) { }

  //! Destructor---call for death
  inline ~ShortStylusSuppressorCore()
  {
    { // Grab new events mutex and insert a DONE BaseIOEvent.
      // Note that by the time this happens, the functor may already be
      // dead from the BrailleTutor object's own DONE BaseIOEvent.
      // Oh well, no harm in repeating ourselves.
      boost::mutex::scoped_lock lock_n(mutex_new_bevents);
      new_bevents.push_back(BaseIOEvent::makeDoneEvent());

      // Notify event reporter that its deadly input is ready.
      cond_new_bevents.notify_one();
    }

    // Wait for the event reporter thread to die
    if(t_new_bevents) t_new_bevents->join();
  }

  //! Set the minimum insertion/withdrawal-time threshold for stylus events
  inline void setThreshold(const TimeInterval &my_threshold)
  {
    { // Grab threshold mutex and change threshold
    boost::mutex::scoped_lock lock_t(mutex_threshold);
    threshold = my_threshold;
    }

    // See if that's made any events reportable now
    findAndReportDebounced();
  }

  //! IOEventHandler functor method definition
  inline virtual void operator()(std::deque<BaseIOEvent> &events)
  {
    unsigned int new_bevents_enqueued = 0;

    { // ENCLOSING BLOCK: new and stylus bevent queue mutexes
    boost::mutex::scoped_lock lock_n(mutex_new_bevents);
    boost::mutex::scoped_lock lock_s(mutex_stylus_bevents);

    // Store stylus events in stylus_bevents for processing. The rest
    // get passed straight to the listener.
    std::deque<BaseIOEvent>::const_iterator e_iter;
    for(e_iter=events.begin(); e_iter!=events.end(); ++e_iter)
      if((e_iter->type == BaseIOEvent::STYLUS_DOWN) ||
	 (e_iter->type == BaseIOEvent::STYLUS_UP))
	stylus_bevents.push_back(*e_iter);
	else {
	  new_bevents.push_back(*e_iter);
	  ++new_bevents_enqueued;
	}

    // Clear out stored events
    events.clear();
    } // END ENCLOSING BLOCK

    // If we have new events, notify the event reporter about them
    if(new_bevents_enqueued > 0) cond_new_bevents.notify_one();

    // See if there are events to debounce or register
    handleTimeout();
  }

  //! Timeout handler---scans stylus_bevents and performs debouncing.

  //! Scans through events, from most recent to least, and eliminates event
  //! pairs that come from the same key/cell. If they appear together in the
  //! queue, it's assumed that they cover an interval too short to belong to
  //! a real event, so both events in the pair are deleted.
  inline virtual void handleTimeout()
  {
    // Pop "debounced" stylus events off the queue.
    findAndReportDebounced();

    // Acquire lock for the stylus events queue
    boost::mutex::scoped_lock lock_s(mutex_stylus_bevents);
//std::cerr << "bevents size: " << stylus_bevents.size() << std::endl;

    // Avoid unnecessary work---can't find an annihilating event pair if
    // there's only 0 or 1 events.
    // If there's one event, we move the timeout back though.
    if(stylus_bevents.empty()) return;
    if(stylus_bevents.size() == 1) { timeout.setTimeout(threshold); return; }

    // A binary vector indicating which events are to be deleted
    std::vector<bool> is_dead(stylus_bevents.size(), false);
    // A collection of iterators to delete
    std::vector<std::list<BaseIOEvent>::iterator> dead_iters;

    // This is a bit nasty. We go through the stylus event list from front
    // (latest) to back (earliest), looking for pairs of events occurring
    // in the same braille cell/dot. We mark these as dead and place
    // iterators pointing to them into an array so we can eliminate them.
    std::list<BaseIOEvent>::iterator sb_iter1 = stylus_bevents.begin();
    for(unsigned int i=0; i<stylus_bevents.size()-1; ++i) {
      if(is_dead[i]) { ++sb_iter1; continue; }
      std::list<BaseIOEvent>::iterator sb_iter2 = sb_iter1; ++sb_iter2;

      for(unsigned int j=0; j<stylus_bevents.size(); ++j) {
	if(is_dead[j]) { ++sb_iter2; continue; }
	if((sb_iter1->cell==sb_iter2->cell) && (sb_iter1->dot==sb_iter2->dot)) {
	  is_dead[i] = is_dead[j] = true;
	  dead_iters.push_back(sb_iter1);
	  dead_iters.push_back(sb_iter2);
	  j = stylus_bevents.size(); // abort iteration
	  continue;
	}
	++sb_iter2;
      }

      ++sb_iter1;
    }

    // Eliminate events that we marked for deletion
    std::vector<std::list<BaseIOEvent>::iterator>::iterator di_iter;
    for(di_iter=dead_iters.begin(); di_iter!=dead_iters.end(); ++di_iter)
/*
{
std::cerr << "[[ eliminating bevent type " << (*di_iter)->type
	  << ", cell " << (*di_iter)->cell
	  << ", dot " << (int) (*di_iter)->dot
	  << ", time " << (*di_iter)->timestamp << " ]]" << std::endl;
*/
      stylus_bevents.erase(*di_iter);
//}

    // Indicate a new scanning timeout. (Need threshold mutex first).
    boost::mutex::scoped_lock lock_t(mutex_threshold);
    timeout.setTimeout(threshold);
  }
};

///////////////////////////////////////
//// ShortStylusSuppressor METHODS ////
///////////////////////////////////////

// ShortStylusSuppressor constructor
ShortStylusSuppressor::ShortStylusSuppressor(const TimeInterval &threshold)
: handler(NULL), sssc(new ShortStylusSuppressorCore(*this, threshold)) { }

// Register a BaseIOEventHandler with this ShortStylusSuppressor.
void ShortStylusSuppressor::setBaseIOEventHandler(BaseIOEventHandler &bioeh)
{
  // We grab the mutex so that nobody calls the handler during the switch
  boost::mutex::scoped_lock lock(out_events_mutex);
  handler = &bioeh;
}

// Call a BaseIOEventHandler functor on the events list immediately.
void ShortStylusSuppressor::pollBaseIOEvents(BaseIOEventHandler &bioeh)
{
  boost::mutex::scoped_lock lock(out_events_mutex);
  bioeh(out_events);
}

// BaseIOEvent handler callback
void ShortStylusSuppressor::operator()(std::deque<BaseIOEvent> &events)
{
  if(sssc != NULL) sssc->operator()(events);
}

// Destructor
ShortStylusSuppressor::~ShortStylusSuppressor()
{
  if(sssc != NULL) delete sssc;
}

} // namespace BrailleTutorNS
