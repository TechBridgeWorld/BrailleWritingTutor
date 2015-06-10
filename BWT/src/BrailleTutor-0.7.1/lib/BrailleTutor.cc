/*
 * BrailleTutor.cc, started 19 June 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Multithreaded Braille Tutor interface library. This file contains the
 * code that talks directly to the serial port and translates bytes to
 * BaseIOEvents.
 */

///////////////////////////
//// BRAILLE TUTOR I/O ////
///////////////////////////

#include <deque>
#include <vector>
#include <cstring>
#include <cassert>
#include <climits>
#include <stdint.h>
#include <iterator>

#include "Types.h"
#include "serial_io.h"
#include "BrailleTutor.h"
#include "BT_StateMachines.h"
#include "BT_rev0_StateMachine.h"

#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace BrailleTutorNS {

///////////////////////////////
//// HARDWARE DESCRIPTIONS ////
///////////////////////////////

//! Makes a vector with BT_Description objects describing BT hardware versions.
static std::vector<boost::shared_ptr<BT_Description> > _make_bt_descriptions()
{
  std::vector<boost::shared_ptr<BT_Description> > descs;
  // Revision 0 tutor
  descs.push_back(boost::shared_ptr<BT_Description>(new BT_rev0_Description()));
  // MORE DESCRIPTIONS GO HERE

  return descs;
}

static std::vector<boost::shared_ptr<BT_Description> > bt_descriptions =
  _make_bt_descriptions();


////////////////////////////////
//// BRAILLE TUTOR I/O CODE ////
////////////////////////////////

//! Communicates with Braille Tutor and generates BaseIOEvent events
class BrailleTutorIO : public boost::noncopyable {
public:
  //! The actual implementation of BrailleTutor::detect
  void detect(std::string &my_serial_port, unsigned int &version);

  //! The actual implementation of BrailleTutor::ready
  void ready(const std::string &my_serial_port, const unsigned int &version);

  //! The actual implementation of BrailleTutor::beep
  void beep(const double &freq, const double &duration);

  //! The actual implementation of BrailleTutor::iopin(pin)
  bool iopin(const unsigned int &pin);

  //! The actual implementation of BrailleTutor::iopin(pin, state)
  bool iopin(const unsigned int &pin, const bool &state);

  //! The actual implementation of BrailleTutor::resetHard()
  bool resetHard();

  //! The actual implementation of BrailleTutor::resetSoft()
  bool resetSoft();

  //! Constructor.

  //! The constructor starts the decoder and event dispatcher threads;
  //! other threads wait until a serial connection has been established
  //! with ready() or detect().
  BrailleTutorIO(BrailleTutor &my_bt);

  //! Destructor. Commands thread death and removes the serial port lockfile
  ~BrailleTutorIO();

  //! Wait for all of the threads to terminate
  void join();

private:
  //! BrailleTutor object whose guts we manipulate
  BrailleTutor &bt;

  //! BrailleTutor hardware description
  boost::scoped_ptr<BT_Description> desc;

  //! State machine model of the Braille Tutor
  boost::scoped_ptr<BT_StateMachine> model;

  //! Bytes to/from the state machine model

  //! Contains the bytestream from the Braille Tutor to the CPU and a
  //! copy of the bytestream going the other way. We maintain a copy since
  //! two parts of this class need those bytes: the state machine and
  //! the code writing bytes to the actual hardware.
  BTSM_inputT model_input;
  //! Bytes actually written out to the BT hardware. See note on model_input.
  std::deque<uint8_t> real_cpu_to_bt;
  //! Event indications from the state machine
  BTSM_outputT indications;
  //! The deque of new BaseIOEvent events decoded from the indications
  std::deque<BaseIOEvent> new_events;
  //! The pin involved in the last I/O pin query
  unsigned int last_pin;
  //! The pinstate discovered in the last I/O pin query
  bool last_pinstate;

  //! Mutex for the model input variable
  boost::mutex mutex_model_input;
  //! Mutex for the queue of bytes actually going out to the BT
  boost::mutex mutex_real_cpu_to_bt;
  //! Mutex for BT event indications
  boost::mutex mutex_indications;
  //! Mutex for new BaseIOEvent events
  boost::mutex mutex_new_events;
  //! Mutex for the serial port
  boost::mutex mutex_serial;
  //! Mutex for the I/O pin query stuff
  boost::mutex mutex_iopin_query;

  //! Condition variable for new data present in model_input
  boost::condition cond_model_input;
  //! Condition variable for new data present in real_cpu_to_bt
  boost::condition cond_real_cpu_to_bt;
  //! Condition variable for new data present in indications
  boost::condition cond_indications;
  //! Condition variable for new BaseIOEvent events
  boost::condition cond_new_events;
  //! Condition variable for results from the last I/O pin query
  boost::condition cond_iopin_query;

  //! Thread for writing bytes out to the serial port
  boost::scoped_ptr<boost::thread> t_serial_writer;
  //! Thread for reading bytes in from the serial port
  boost::scoped_ptr<boost::thread> t_serial_reader;
  //! Thread for the state machine model
  boost::scoped_ptr<boost::thread> t_model;
  //! Thread for translating indications to BrailleTutor BaseIOEvent events
  boost::scoped_ptr<boost::thread> t_decoder;
  //! Thread for dispatching new BaseIOEvent events to the interface
  boost::scoped_ptr<boost::thread> t_new_events;

  //! I/O handle for the serial port
  serial_handle serial_fd;
  //! Port name used to open the serial port
  std::string serial_port;

  //! Handles new command bytes from the CPU to the BT

  //! Inserts new command bytes for the BT into the two relevant queues:
  //! the model input queue and the queue of actual bytes to send out to
  //! the serial port.
  template <typename InputIterator>
  inline void addCommandBytes(InputIterator begin, InputIterator end)
  {
    // Get locks for both the model input and the real output queue.
    // We do both at once to avoid e.g. the actual output sending replies to
    // the model input before the model input can figure out what they were
    // all about.
    boost::mutex::scoped_lock lock_m(mutex_model_input);
    boost::mutex::scoped_lock lock_r(mutex_real_cpu_to_bt);

    // Now add the bytes
    model_input.cpu_to_bt.insert(model_input.cpu_to_bt.end(), begin, end);
    real_cpu_to_bt.insert(real_cpu_to_bt.end(), begin, end);

    // Now indicate that there's new bytes
    cond_model_input.notify_one();
    cond_real_cpu_to_bt.notify_one();
  }

  //! Returns true if this object is connected to a Braille Tutor
  inline void checkReady() {
    if(serial_fd == INVALID_SERIAL_HANDLE)
      throw BTException(BTException::BT_EIO,
			"BrailleTutor object is not connected yet");
  }
};


//// I/O THREADS ////

//! The thread functor that writes bytes out to the serial port
struct FunctorSerialWriter {
  //! Reference to queue of bytes to write to output
  std::deque<uint8_t> &cpu_to_bt;
  //! Reference to mutex for output bytes queue
  boost::mutex &mutex_cpu_to_bt;
  //! Reference to mutex for the serial port
  boost::mutex &mutex_serial;
  //! Reference to condition variable indicating bytes to write
  boost::condition &cond;
  //! I/O handle for the serial port
  serial_handle serial_fd;

  //! Constructor: fills in references
  inline FunctorSerialWriter(std::deque<uint8_t> &my_cpu_to_bt,
			     boost::mutex &my_mutex_cpu_to_bt,
			     boost::mutex &my_mutex_serial,
			     boost::condition &my_cond,
			     serial_handle &my_serial_fd)
  : cpu_to_bt(my_cpu_to_bt), mutex_cpu_to_bt(my_mutex_cpu_to_bt),
    mutex_serial(my_mutex_serial), cond(my_cond), serial_fd(my_serial_fd)
  { }

  //! Perform this functor's function
  inline void operator()()
  {
    // Loop forever---write bytes when available
    for(;;) {
      // grab mutex for byte buffer
      boost::mutex::scoped_lock lock_q(mutex_cpu_to_bt);

      // If there is no data, wait until data is available
      if(cpu_to_bt.empty()) {
	cond.wait(lock_q);
	// no data means quit!
	if(cpu_to_bt.empty()) return;
      }

      // grab serial port mutex
      boost::mutex::scoped_lock lock_s(mutex_serial);

      // Just quit if the serial descriptor is invalid---means the port
      // is most likely closed. Kill this thread.
      if(serial_fd == INVALID_SERIAL_HANDLE) return;

      // Write out all bytes
      serial_write(serial_fd, cpu_to_bt.begin(), cpu_to_bt.end());

      // Bytes are written. Clear byte queue.
      cpu_to_bt.clear();
    }
  }
};

//! The thread functor that reads bytes in from the serial port
struct FunctorSerialReader {
  //! Bytes to/from the state machine model (we want the bt_to_cpu queue)
  BTSM_inputT &model_input;
  //! Reference to mutex for the model_input variable
  boost::mutex &mutex_model_input;
  //! Reference to mutex for the serial port
  boost::mutex &mutex_serial;
  //! Reference to condition variable indicating new data in model_input
  boost::condition &cond;
  //! I/O handle for the serial port
  serial_handle serial_fd;

  //! Constructor: fills in references
  inline FunctorSerialReader(BTSM_inputT &my_model_input,
			     boost::mutex &my_mutex_model_input,
			     boost::mutex &my_mutex_serial,
			     boost::condition &my_cond,
			     serial_handle &my_serial_fd)
  : model_input(my_model_input), mutex_model_input(my_mutex_model_input),
    mutex_serial(my_mutex_serial), cond(my_cond), serial_fd(my_serial_fd)
  { }

  //! Perform this functor's function
  inline void operator()()
  {
    // We poll the serial port every 30 milliseconds---a sloppy solution but
    // likely to be portable and not too resource-intensive. Here's the
    // interval for polling:
    const TimeInterval poll_interval(0, 30);

    // Loop forever---check for and read bytes occasionally
    for(;;) {
      // Sleep for polling
      poll_interval.sleep();

      // grab model_input mutex
      boost::mutex::scoped_lock lock_i(mutex_model_input);
      // grab serial port mutex
      boost::mutex::scoped_lock lock_s(mutex_serial);

      // Just quit if the serial descriptor is invalid---means the port
      // is most likely closed. Kill this thread.
      if(serial_fd == INVALID_SERIAL_HANDLE) return;

      // Create insertion iterator for model_input.bt_to_cpu
      std::back_insert_iterator<std::deque<uint8_t> >
	inserter(model_input.bt_to_cpu);
      // Read and insert bytes into input queue. If we read bytes, notify
      // the model thread.
      if(serial_read(serial_fd, inserter) > 0) cond.notify_one();
    }
  }
};

//! The thread functor that manages the state machine model
struct FunctorModel {
  //! Reference to the state machine model
  BT_StateMachine &model;
  //! Reference to the model's copy of the I/O streams
  BTSM_inputT &model_input;
  //! Reference to the event indications output queue
  BTSM_outputT &indications;
  //! Reference to the model_input mutex
  boost::mutex &mutex_model_input;
  //! Reference to the event indications queue
  boost::mutex &mutex_indications;
  //! Condition variable for new data present in model_input
  boost::condition &cond_model_input;
  //! Condition variable for new data present in indications
  boost::condition &cond_indications;

  //! Constructor: fills in references
  inline FunctorModel(BT_StateMachine &my_model, BTSM_inputT &my_model_input,
		      BTSM_outputT &my_indications,
		      boost::mutex &my_mutex_model_input,
		      boost::mutex &my_mutex_indications,
		      boost::condition &my_cond_model_input,
		      boost::condition &my_cond_indications)
  : model(my_model), model_input(my_model_input), indications(my_indications),
    mutex_model_input(my_mutex_model_input),
    mutex_indications(my_mutex_indications),
    cond_model_input(my_cond_model_input), cond_indications(my_cond_indications)
  {
    // Clear out the model's data store
    model.getData() = BTSM_dataT();
  }

  //! Perform this functor's function
  inline void operator()()
  {
    // We keep track of the last sizes of the model inputs. If the current
    // sizes are different from the last sizes, then we know there's a change
    // we haven't processed, and we don't bother waiting to be notified
    // for new changes. Note startup values force one initial runthrough.
    unsigned int lastsize_ctb = UINT_MAX;
    unsigned int lastsize_btc = UINT_MAX;

    // Loop forever--wait on model input and generate indications
    for(;;) {
      // grab model_input mutex
      boost::mutex::scoped_lock lock_i(mutex_model_input);

      // If the queue sizes have changed since our coming back over the top
      // of the for loop and grabbing the mutex, we don't bother waiting for
      // a new notification---we just process the info.
      if((lastsize_ctb == model_input.cpu_to_bt.size()) ||
	 (lastsize_btc == model_input.bt_to_cpu.size()))
      {
	cond_model_input.wait(lock_i); // Wait for input change

	// If we were notified and there's no input, this is a signal that
	// we're to quit.
	if(model_input.bt_to_cpu.empty() && model_input.cpu_to_bt.empty())
	  return;
      }

      // grab indications mutex
      boost::mutex::scoped_lock lock_n(mutex_indications);

      // Count the current size of the I/O indications
      const unsigned int presize_inds = indications.size();

      // Stuff the model inputs into the state machine's face until it
      // completely exhausts one of the queues. After that, stuff until the
      // queue sizes don't change
      for(;;) {
	const unsigned int presize_ctb = model_input.cpu_to_bt.size();
	const unsigned int presize_btc = model_input.bt_to_cpu.size();
	model.cycle(model_input, indications);
	if(model_input.cpu_to_bt.empty() &&
	   model_input.bt_to_cpu.empty()) break;
	// The queue sizes didn't change. What happened?
	if((presize_ctb == model_input.cpu_to_bt.size()) &&
	   (presize_btc == model_input.bt_to_cpu.size())) {
	  // If both queues still have stuff in them, it's an error in the
	  // state machine, since it should be able to do *something*
	  if((presize_ctb > 0) && (presize_btc > 0))
	    throw BTException(BTException::BT_EMISC,
	      "frozen internal state machine model of the Braille Tutor");
	  else break;
	}
      }

      // If the I/O indications size changed, alert that there are more
      // indications.
      if(presize_inds != indications.size()) cond_indications.notify_one();

      // Save the sizes of the queues after that last round of processing
      lastsize_ctb = model_input.cpu_to_bt.size();
      lastsize_btc = model_input.bt_to_cpu.size();
    }
  }
};

//! The thread functor that turns I/O indications into BaseIOEvent events
struct FunctorDecoder {
  //! Reference to event indications from the state machine
  BTSM_outputT &indications;
  //! Reference to new events decoded by this object
  std::deque<BaseIOEvent> &new_events;
  //! Reference to the pin involved in the last I/O pin query
  unsigned int &last_pin;
  //! Reference to the pinstate discovered in the last I/O pin query
  bool &last_pinstate;
  //! Reference to mutex for the BT event indications
  boost::mutex &mutex_indications;
  //! Reference to mutex for new BaseIOEvent events
  boost::mutex &mutex_new_events;
  //! Reference to mutex for the I/O pin query stuff
  boost::mutex &mutex_iopin_query;
  //! Reference to condition variable for new data present in indications
  boost::condition &cond_indications;
  //! Reference to condition variable for new BaseIOEvent events
  boost::condition &cond_new_events;
  //! Reference to condition variable for pin query results
  boost::condition &cond_iopin_query;

  //! Contains the most recent indications for active buttons or braille dots

  //! We use a deque because very few buttons/dots are likely to be active at
  //! once---searching should be pretty fast.
  std::deque<BTSM_Indication> actives;

  //! Constructor---fill in references
  inline FunctorDecoder(BTSM_outputT &my_indications,
			std::deque<BaseIOEvent> &my_new_events,
			unsigned int &my_last_pin,
			bool &my_last_pinstate,
			boost::mutex &my_mutex_indications,
			boost::mutex &my_mutex_new_events,
			boost::mutex &my_mutex_iopin_query,
			boost::condition &my_cond_indications,
			boost::condition &my_cond_new_events,
			boost::condition &my_cond_iopin_query)
  : indications(my_indications), new_events(my_new_events),
    last_pin(my_last_pin), last_pinstate(my_last_pinstate),
    mutex_indications(my_mutex_indications),
    mutex_new_events(my_mutex_new_events),
    mutex_iopin_query(my_mutex_iopin_query),
    cond_indications(my_cond_indications),
    cond_new_events(my_cond_new_events),
    cond_iopin_query(my_cond_iopin_query) { }

  //! Perform this functor's function
  inline void operator()()
  {
    // Here again we "poll" the indications buffer every 30 milliseconds---if
    // there is an active button or dot. If we reach the end of the polling
    // delay, then the button or dot has been unpushed.
    const TimeInterval poll_interval(0, 30);

    // Loop forever---grab indications when available
    for(;;) {
      // Will hold the current time after we've grabbed the indications mutex.
      TimeInterval now;

      // True iff new BaseIOEvent events were made in this iteration
      bool made_new_events = false;

      // Will be true if we timed out waiting for new events to come---in
      // that case we have to check the actives list for buttons or dots
      // that aren't pushed anymore.
      bool timedout = false;

      { // ENCLOSING BLOCK: For grabbing indications mutex
      boost::mutex::scoped_lock lock_i(mutex_indications);
      if(actives.empty()) cond_indications.wait(lock_i);
      else {
	// Add in timeout. First we need to know the xtime for when the
	// timeout happens
	boost::xtime time_end;
	boost::xtime_get(&time_end, boost::TIME_UTC_); // Was TIME_UTC but changed to TIME_UTC_ in Boost 1.50  and greater
	time_end.sec += poll_interval.secs;
	const unsigned int nsecs = poll_interval.msecs * 1000000;
	time_end.nsec += nsecs % 1000000000;
	time_end.sec  += nsecs / 1000000000;
	// Then wait for new indications
	timedout = !cond_indications.timed_wait(lock_i, time_end);
      }

      // Find the current time
      now = TimeInterval::now();

      // New indications? Add them to the actives
      BTSM_outputT::const_iterator i_iter;
      std::deque<BTSM_Indication>::iterator a_iter;
      for(i_iter=indications.begin(); i_iter!=indications.end(); ++i_iter) {
	// If this indication is the DONE indication, pass the DONE on to
	// the BaseIOEvent handler and quit.
	if(i_iter->type == BTSM_Indication::DONE) {
	  boost::mutex::scoped_lock lock(mutex_new_events);
	  new_events.push_back(BaseIOEvent::makeDoneEvent());
	  cond_new_events.notify_one();
	  return;
	}

	// If this indication is I/O pin related, fill in the I/O pin
	// query variables and notify anyone curious that new info is there.
	// There should be little risk in acquiring the I/O pin query lock,
	// since nobody holds it for long.
	if(i_iter->type == BTSM_Indication::IOPIN_IN) {
	  boost::mutex::scoped_lock lock(mutex_iopin_query);
	  last_pin = i_iter->iopin;
	  last_pinstate = i_iter->pinstate;
	  cond_iopin_query.notify_one();
	  continue;
	}

	// Remaining code is for cell dot and button indications. First,
	// see if this indication exists. If it is, replace that indication
	// with this newer one.
	bool found = false;
	// For stylus events we have additional processing to eliminate
	// spurious stylus events originating from poor contact between the
	// stylus and the slate holes. These events make it appear that
	// a second stylus has been inserted into a slate hole near the
	// actual insertion point, which is not something the BT can
	// actually detect. Thus we filter out new stylus indications if
	// a stylus indication is already present in the actives queue.
	if(i_iter->type == BTSM_Indication::STYLUS)
	  for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter) {
	    if(a_iter->sameIndication(*i_iter)) {
	      found = true;
	      *a_iter = *i_iter;
	      break;
	    }
	    else if(a_iter->type == BTSM_Indication::STYLUS)
	    { found = true; break; }
	  }
	// For non-stylus events, handling is straightforward
	else
	  for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter)
	    if(a_iter->sameIndication(*i_iter)) {
	      found = true;
	      *a_iter = *i_iter;
	      break;
	    }

	// We didn't find an indication matching this one, so add it anew
	// to the indications list and generate a *_DOWN BaseIOEvent.
	if(!found) {
	  // Save the indication to the actives list
	  actives.push_back(*i_iter);

	  // Creating a new *_DOWN event for the new guy. First we indicate
	  // that a new event was made...
	  made_new_events = true;

	  // ...then we attain the mutex for the new events queue...
	  boost::mutex::scoped_lock lock(mutex_new_events);

	  // then we make the new event
	  if(i_iter->type == BTSM_Indication::STYLUS)
	    new_events.push_back(
	      BaseIOEvent::makeStylusDownEvent(now, i_iter->cell, i_iter->dot));
	  else if(i_iter->type == BTSM_Indication::BUTTON)
	    new_events.push_back(
	      BaseIOEvent::makeButtonDownEvent(now, i_iter->button));
	}
      }
      // Clear out the new indications
      indications.clear();
      } // END ENCLOSING BLOCK: Releasing indications mutex

      // If we've timed out, let's check for dead guys in the actives list---
      // we kill these and create *_UP BaseIOEvents
      if(timedout) {
	// Will hold indices of items to erase.
	std::deque<unsigned int> deads;

	// Here we loop over the actives list
        unsigned int index=0;
        std::deque<BTSM_Indication>::iterator a_iter;
	for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter) {
	  // Check if this event has timed out; if so, dead.
	  if((a_iter->timestamp + poll_interval * TimeInterval(6)) < now) {
	    // Mark indication as condemned.
	    deads.push_front(index);

	    // Creating a new *_UP event for the doomed guy. First we indicate
	    // that a new event was made...
	    made_new_events = true;

	    // ...then we attain the mutex for the new events queue...
	    boost::mutex::scoped_lock lock(mutex_new_events);

	    // then we make the new event.
	    if(a_iter->type == BTSM_Indication::STYLUS)
	      new_events.push_back(
		BaseIOEvent::makeStylusUpEvent(now, a_iter->cell, a_iter->dot));
	    else if(a_iter->type == BTSM_Indication::BUTTON)
	      new_events.push_back(
		BaseIOEvent::makeButtonUpEvent(now, a_iter->button));
	  }
	  ++index;
	}

	// Here we kill all of the condemend actives
	std::deque<unsigned int>::const_iterator d_iter;
	for(d_iter=deads.begin(); d_iter!=deads.end(); ++d_iter)
	  actives.erase(actives.begin() + *d_iter);
      }

      // If we made new events, alert the event thread that more events
      // are ready to go
      if(made_new_events) cond_new_events.notify_one();
    }
  }
};

//! The thread functor that calls the BaseIOEventHandler callback on new events
struct FunctorNewEvents {
  //! Reference to new events decoded by the decoder thread
  std::deque<BaseIOEvent> &new_events;
  //! Reference to mutex for new BaseIOEvent events
  boost::mutex &mutex_new_events;
  //! Reference to condition variable for new BaseIOEvent events
  boost::condition &cond_new_events;
  //! Reference to the BrailleTutor object we're manipulating
  BrailleTutor &bt;

  //! Constructor---fill in references
  inline FunctorNewEvents(std::deque<BaseIOEvent> &my_new_events,
			  boost::mutex &my_mutex_new_events,
			  boost::condition &my_cond_new_events,
			  BrailleTutor &my_bt)
  : new_events(my_new_events), mutex_new_events(my_mutex_new_events),
    cond_new_events(my_cond_new_events), bt(my_bt) { }

  //! Perform this functor's function
  inline void operator()()
  {
    // Loop forever---grab new events when available
    for(;;) {
      // A local copy of incoming events
      std::deque<BaseIOEvent> local_new_events;

      { // ENCLOSING BLOCK: For getting new events
      // Grab lock on new_events queue.
      boost::mutex::scoped_lock lock_n(mutex_new_events);

      // If events are present in the queue, handle them immediately.
      // Otherwise, just wait for new events.
      if(new_events.empty()) {
        cond_new_events.wait(lock_n);

        // If there are no new events, we should quit. We schedule a new DONE
        // event and proceed normally. NOTE: Using empty event queues to direct
        // quitting through this mechanism is deprecated.
        if(new_events.empty()) 
	  new_events.push_back(BaseIOEvent::makeDoneEvent());
      }

      // Copy the events to a local deque of events
      std::back_insert_iterator<std::deque<BaseIOEvent> >
	lne_inserter(local_new_events);
      std::copy(new_events.begin(), new_events.end(), lne_inserter);

      // Clear out new events queue
      new_events.clear();
      } // END ENCLOSING BLOCK

      { // ENCLOSING BLOCK: For transferring new events.
      // Grab lock on BT events queue
      boost::mutex::scoped_lock lock_b(bt.out_events_mutex);

      // Move new events to the out_events queue; if we hit a DONE event, then
      // pass the events leading up to it on to the handler and then quit.
      std::deque<BaseIOEvent>::const_iterator i;
      for(i=local_new_events.begin(); i!=local_new_events.end(); ++i) {
	bt.out_events.push_back(*i);
	if(i->type == BaseIOEvent::DONE) {
	  if(bt.handler) (*bt.handler)(bt.out_events);
	  return;
	}
      }
      } // END ENCLOSING BLOCK

      // Calling the BaseIOEventHandler, if it exists
      if(bt.handler) {
	boost::mutex::scoped_lock lock_b(bt.out_events_mutex);
	(*bt.handler)(bt.out_events);
      }
    }
  }
};

////////////////////////////////
//// BrailleTutorIO METHODS ////
////////////////////////////////

// Tries to detect a BrailleTutor in autodetect mode on one of the system's
// serial ports. If found, opens the port, commands an initialization of
// the BT to interactive mode, and returns the string representation of
// the discovered serial port as well as the BT ROM version number
void BrailleTutorIO::detect(std::string &my_serial_port, unsigned int &version)
{
  // Lock access to the serial port
  boost::mutex::scoped_lock lock_s(mutex_serial);

  // If we already have a BT open, throw an exception
  if(serial_fd != INVALID_SERIAL_HANDLE)
    throw BTException(BTException::BT_EALREADY,
		      std::string("in detect(): Tutor already connected on ") +
		      serial_port);

  // Get some suggestions about which serial port to use
  std::deque<std::string> suggestions(serial_suggest_ports());

  // Try each suggestion one by one
  while(!suggestions.empty()) {
    try { 
#ifndef NDEBUG
      std::cerr << suggestions.front() << "... " << std::endl;
#endif
      serial_open(suggestions.front(), serial_fd);
#ifndef NDEBUG
      std::cerr << "done." << std::endl;
#endif
    }
    catch(const BTException &e) {
      if((e.type == BTException::BT_EBUSY) ||
	 (e.type == BTException::BT_ENOENT) ||
	 (e.type == BTException::BT_EACCES) ||
	 (e.type == BTException::BT_EALREADY))
      { suggestions.pop_front(); continue; }
      throw;
    }

    // We have an open serial port. Wait a second and try to detect the
    // "nnnn" of serial input.
    TimeInterval(1).sleep();
    std::deque<uint8_t> nnns;
    std::back_insert_iterator<std::deque<uint8_t> > inserter(nnns);
    serial_read(serial_fd, inserter);

    // TODO: This detection method is exclusively for the rev 0 BT. In the
    // future, collect a second's worth of bytes and query each BT
    // description until we find a match.

    // Sometimes the serial port has some 0 bytes at the head of the data
    // stream. We first filter out any characters at the beginning of the
    // nnns list that isn't 'n'.
/*
std::cerr << "Things in nnns:";
for(unsigned int i=0;i<nnns.size();++i)
  std::cerr << ' ' << (unsigned int) nnns[i];
std::cerr << std::endl;
*/
    while(!nnns.empty())
      if(nnns.front() == 'n') break;
      else nnns.pop_front();

    // Fewer than 4 "nnnn" in one second? Not our guy.
    if(nnns.size() < 4) {
      serial_close(serial_fd, suggestions.front());
      suggestions.pop_front();
      continue;
    }

    // Make sure that all of the values are "n".
    bool all_ns = true;
    while(!nnns.empty()) {
      if(nnns.front() != 'n') {
	serial_close(serial_fd, suggestions.front());
	suggestions.pop_front();
	all_ns = false;
	break;
      }
      nnns.pop_front();
    }

    // If it really was all "n"s, quit the detection loop.
    if(all_ns) break;
  }

  // See whether we actually opened a serial port
  if(serial_fd == INVALID_SERIAL_HANDLE)
    throw(BTException(BTException::BT_ENOENT,
		      "failed to detect a waiting Braille Tutor device"));

  // We've found the serial port with the working tutor
  my_serial_port = serial_port = suggestions.front();
  // We have "detected" that this BT is a rev 0 BT. TODO: Real detection
  version = 0;
  // Initialize local copy of the BT description and the BT model
  desc.reset(bt_descriptions[version]->clone());
  model.reset(new BT_StateMachine(desc->makeStateMachine()));

  // Start the model thread
  t_model.reset(
    new boost::thread(
      FunctorModel(*model, model_input, indications,
		   mutex_model_input, mutex_indications,
		   cond_model_input, cond_indications)));

  // Start the serial threads
  t_serial_writer.reset(
    new boost::thread(
      FunctorSerialWriter(real_cpu_to_bt, mutex_real_cpu_to_bt, mutex_serial,
			  cond_real_cpu_to_bt, serial_fd)));
  t_serial_reader.reset(
    new boost::thread(
      FunctorSerialReader(model_input, mutex_model_input, mutex_serial,
			  cond_model_input, serial_fd)));

  // Finally, command the BT to initialize
  std::deque<uint8_t> intmode_bytes = desc->makeInteractiveModeBytes();
  addCommandBytes(intmode_bytes.begin(), intmode_bytes.end());
}

// Indicates that a BrailleTutor with the specified ROM revision is
// ready in interactive mode on a specified port. This implementation calls
// resetSoft() after connecting to the port.
void BrailleTutorIO::ready(const std::string &my_serial_port,
			   const unsigned int &version)
{
  // Lock access to the serial port
  boost::mutex::scoped_lock lock_s(mutex_serial);

  // If we already have a BT open, throw an exception
  if(serial_fd != INVALID_SERIAL_HANDLE)
    throw BTException(BTException::BT_EALREADY,
		      std::string("in ready(): Tutor already connected on ") +
		      serial_port);

  // Check version argument
  if(version >= bt_descriptions.size())
    throw BTException(BTException::BT_EINVAL,
		      "invalid Braille Tutor version number");

  // Initialize local copy of the BT description and the BT model
  desc.reset(bt_descriptions[version]->clone());
  model.reset(new BT_StateMachine(desc->makeStateMachine()));

  // Open serial port
  serial_open(my_serial_port, serial_fd);

  // Save the name of the I/O port
  serial_port = my_serial_port;

  // Start the model thread
  t_model.reset(
    new boost::thread(
      FunctorModel(*model, model_input, indications,
		   mutex_model_input, mutex_indications,
		   cond_model_input, cond_indications)));

  // May as well command a soft reset here, since that doesn't need the serial
  // threads
  resetSoft();

  // Start the serial threads
  t_serial_writer.reset(
    new boost::thread(
      FunctorSerialWriter(real_cpu_to_bt, mutex_real_cpu_to_bt, mutex_serial,
			  cond_real_cpu_to_bt, serial_fd)));
  t_serial_reader.reset(
    new boost::thread(
      FunctorSerialReader(model_input, mutex_model_input, mutex_serial,
			  cond_model_input, serial_fd)));
}

// Command a beep
void BrailleTutorIO::beep(const double &freq, const double &duration)
{
  checkReady();
  std::deque<uint8_t> beep_bytes = desc->makeBeepBytes(freq, duration);
  addCommandBytes(beep_bytes.begin(), beep_bytes.end());
}

// Retrieve pin state
bool BrailleTutorIO::iopin(const unsigned int &pin)
{
  checkReady();

  // Acquire a pinstate lock first to avoid race condition (our receiving
  // a notification before we wait() for it.
  boost::mutex::scoped_lock lock_p(mutex_iopin_query);

  // First ask the Tutor what the pinstate is
  std::deque<uint8_t> pin_bytes = desc->makeGetIOPinBytes(pin);
  addCommandBytes(pin_bytes.begin(), pin_bytes.end());

  // Wait for the pinstate response.
  cond_iopin_query.wait(lock_p);

  // Return the query result
  assert(last_pin == pin);
  return last_pinstate;
}

// Command a new pin setting
bool BrailleTutorIO::iopin(const unsigned int &pin, const bool &state)
{
  checkReady();

  std::deque<uint8_t> pin_bytes = desc->makeSetIOPinBytes(pin, state);
  addCommandBytes(pin_bytes.begin(), pin_bytes.end());
  return state;
}

// Hard reset of the BT state, where we try to get it into a known state
bool BrailleTutorIO::resetHard()
{
  // First a soft reset, then turn off buzzer and set I/O pin to low
  if(!resetSoft()) return false;
  beep(440.0, 0.0);
  iopin(0, false);
  return true; // no exceptions; it must have worked!
}

// Soft reset of the BT state, where we try to get it into a known state
// Hopefully other operations running at the same time won't run into
// deadlocks (I think an iopin query might). FIXME
bool BrailleTutorIO::resetSoft()
{
  checkReady();

  // The first thing we do is grab all mutexes for BT I/O.
  boost::mutex::scoped_lock lock_m(mutex_model_input);
  boost::mutex::scoped_lock lock_r(mutex_real_cpu_to_bt);
  boost::mutex::scoped_lock lock_s(mutex_serial);

  // OK. Now we clear out all I/O queues
  model_input.cpu_to_bt.clear();
  model_input.bt_to_cpu.clear();
  real_cpu_to_bt.clear();

  // Now we retrieve the magic whammy reset bytes, as well as the name of
  // the state they take us to.
  BTSM_stateNameT dest;
  std::deque<uint8_t> reset_bytes = desc->makeResetBytes(dest);
  // Now we send bytes directly to the serial port.
  serial_write(serial_fd, reset_bytes.begin(), reset_bytes.end());

  // Now wait three seconds for the BT hardware to digest it.
  TimeInterval(3,0).sleep();

  // Now dump all bytes coming in from the BT---it'll be bitching about
  // a confusing command sequence.
  std::deque<uint8_t> junkbytes;
  std::back_insert_iterator<std::deque<uint8_t> > inserter(junkbytes);
  serial_read(serial_fd, inserter);
  
  // Now reset the state machine model.
  model->setState(dest);
  model->getData() = BTSM_dataT();

  // No exceptions? I guess we succeeded.
  return true;
}

// Wait for all the threads to terminate. This will actually never happen in
// this implementation (in other words, one of your other threads should call
// exit() when you're ready to quit the program) but it's still considered
// proper form to call this routine.
void BrailleTutorIO::join()
{
  if(t_serial_writer.get()) t_serial_writer->join();
  if(t_serial_reader.get()) t_serial_reader->join();
  if(t_model.get()) t_model->join();
  if(t_decoder.get()) t_decoder->join();
  if(t_new_events.get()) t_new_events->join();
}

// BrailleTutorIO constructor
BrailleTutorIO::BrailleTutorIO(BrailleTutor &my_bt)
: bt(my_bt), last_pin(UINT_MAX), last_pinstate(false),
  serial_fd(INVALID_SERIAL_HANDLE)
{
  // Start the decoder and dispatcher threads
  t_decoder.reset(
    new boost::thread(
      FunctorDecoder(indications, new_events, last_pin, last_pinstate,
		     mutex_indications, mutex_new_events, mutex_iopin_query,
		     cond_indications, cond_new_events, cond_iopin_query)));
  t_new_events.reset(
    new boost::thread(
      FunctorNewEvents(new_events, mutex_new_events, cond_new_events, bt)));
}

// BrailleTutorIO destructor
BrailleTutorIO::~BrailleTutorIO()
{
  // Probably not the best way to do this.

  // Close the serial port
  {
    boost::mutex::scoped_lock lock_s(mutex_serial);   // grab serial port mutex
    // Close serial port and set serial_fd to the invalid serial handle
    if(serial_fd != INVALID_SERIAL_HANDLE) serial_close(serial_fd, serial_port);
  }

  // Wait for serial port reader to die
  if(t_serial_reader) t_serial_reader->join();

  {
    // Now grab every single mutex except serial and out_events.
    boost::mutex::scoped_lock lock_m(mutex_model_input);
    boost::mutex::scoped_lock lock_r(mutex_real_cpu_to_bt);
    boost::mutex::scoped_lock lock_i(mutex_indications);
    boost::mutex::scoped_lock lock_n(mutex_new_events);
    boost::mutex::scoped_lock lock_q(mutex_iopin_query);

    // The convention of the threads (most of them) is that if the input
    // queues they're wating on contain nothing, then they should quit.
    model_input.bt_to_cpu.clear();
    model_input.cpu_to_bt.clear();
    real_cpu_to_bt.clear();
    indications.clear();
    new_events.clear();

    // Pass DONE indications to threads to indicate that we're quitting.
    // The indications decoder would tell the new events functor anyway, but
    // here we cut out the middleman.
    indications.push_back(BTSM_Indication::makeDoneIndication());
    new_events.push_back(BaseIOEvent::makeDoneEvent());

    // Notify threads that their deadly input is ready.
    cond_model_input.notify_one();
    cond_real_cpu_to_bt.notify_one();
    cond_indications.notify_one();
    cond_new_events.notify_one();

    // Hopefully it's safe not to worry about iopin queries.
  }

  // Now we wait for everyone to die.
  if(t_serial_writer) t_serial_writer->join();
  if(t_model) t_model->join();
  if(t_decoder) t_decoder->join();
  if(t_new_events) t_new_events->join();
}

//////////////////////////////
//// BrailleTutor METHODS ////
//////////////////////////////

// BrailleTutor constructor
BrailleTutor::BrailleTutor()
: handler(NULL), btio(NULL) { }

// Initializes internal resources---mainly constructs the btio object
void BrailleTutor::init()
{
  if(btio != NULL)
    throw BTException(BTException::BT_EALREADY,
		      "BrailleTutor object already initialized");
  btio = new BrailleTutorIO(*this);
}

// Checks whether init has been called; otherwise throws an exception
void BrailleTutor::checkReady()
{
  if(btio == NULL)
    throw BTException(BTException::BT_EMISC,
		      "BrailleTutor object is not initialized");
}

// Attempts autodetection of a Braille Tutor on one of the serial ports.
void BrailleTutor::detect(std::string &io_port, unsigned int &version)
{
  checkReady();
  btio->detect(io_port, version);
}

// Tells the BrailleTutor object about a Tutor ready on a specified port
void BrailleTutor::ready(const std::string &io_port,
			 const unsigned int &version)
{
  checkReady(); // different "ready"
  btio->ready(io_port, version);
}

// Causes the BrailleTutor to emit a beep
void BrailleTutor::beep(const double &freq, const double &duration)
{
  checkReady();
  btio->beep(freq, duration);
}

// Returns the status of a binary I/O pin
bool BrailleTutor::iopin(const unsigned int &pin)
{
  checkReady();
  return btio->iopin(pin);
}

// Sets the status of a binary I/O pin
bool BrailleTutor::iopin(const unsigned int &pin, const bool &state)
{
  checkReady();
  return btio->iopin(pin, state);
}

// Set a new BaseIOEventHandler
void BrailleTutor::setBaseIOEventHandler(BaseIOEventHandler &bioeh)
{
  // We grab the mutex so that nobody calls the handler during the switch
  boost::mutex::scoped_lock lock(out_events_mutex);
  handler = &bioeh;
}

// Call the base I/O event handler on the current event list
void BrailleTutor::pollBaseIOEvents(BaseIOEventHandler &bioeh)
{
  boost::mutex::scoped_lock lock(out_events_mutex);
  bioeh(out_events);
}

// Attempt a "hard reset" of the Braille Tutor
bool BrailleTutor::resetHard()
{
  checkReady();
  return btio->resetHard();
}

// Attempt a "soft reset" of the Braille Tutor
bool BrailleTutor::resetSoft()
{
  checkReady();
  return btio->resetSoft();
}

// Deconstructor
BrailleTutor::~BrailleTutor()
{
  if(btio != NULL) delete btio;
}

// Cedes program control to the BrailleTutor I/O threads
void BrailleTutor::join()
{
  checkReady();
  btio->join();
}

} // namespace BrailleTutorNS
