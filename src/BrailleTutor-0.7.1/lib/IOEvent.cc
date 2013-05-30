/*
 * Braille Tutor interface library
 * IOEvent.cc, started 15 July 2007 by Tom Stepleton (tss@ri.cmu.edu) (finally)
 *
 * Implementation of the IOEvent BaseIOEventHandler, which converts basic
 * Braille Tutor events (.g. stylus insertions, etc.) into more
 * complex events (e.g. whole braille letters).
 */

#include <set>
#include <queue>
#include <cassert>
#include <climits>

#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "Charset.h"
#include "IOEvent.h"

namespace BrailleTutorNS {

////////////////////////////////////
//// CORE I/O EVENT PARSER CODE ////
////////////////////////////////////

//// PARSER THREADS ////

//! The thread functor that turns BaseIOEvent events into IOEvent events
struct FunctorIOEventDecoder {
  //! Reference to BaseIOEvent events to turn into IOEvent events
  std::deque<BaseIOEvent> &in_bevents;
  //! Reference to the IOEvent events we made
  std::deque<IOEvent> &new_events;
  //! Reference to mutex for incoming BaseIOEvent events
  boost::mutex &mutex_in_bevents;
  //! Reference to mutex for outgoing IOEvent events
  boost::mutex &mutex_new_events;
  //! Reference to condition variable for new data present in in_bevents
  boost::condition &cond_in_bevents;
  //! Reference to condition variable for new IOEvent events
  boost::condition &cond_new_events;

  //! Reference to time delay for user cell glyph completion
  TimeInterval &glyph_delay;
  //! Reference to mutex for the glyph delay
  boost::mutex &mutex_glyph_delay;

  //! Reference to a pointer to the current character set
  const Charset* &charset;
  //! Reference to mutex for the charset pointer
  boost::mutex &mutex_charset;

  //! Reference to the set of events we should bother decoding
  std::set<IOEvent::Type> &watchset;
  //! Reference to mutex for the event watch set
  boost::mutex &mutex_watchset;

  //! Contains the pushdown events for active buttons or braille dots

  //! Contains the BaseIOEvent saying when "active" buttons or dots (i.e.
  //! those that a finger is currently on or a stylus is currently in)
  //! were pushed down. We use a deque because very few buttons/dots are
  //! likely to be active at once---searching should be very fast.
  std::deque<BaseIOEvent> actives;

  //! Datatype saying where and whether a dot glyph is under construction
  typedef enum { NONE,		//!< No glyph is currently under construction
		 BUTTONS,	//!< Glyph under construction on the buttons
		 CELL,		//!< Glyph under construction in a braille cell
	  } GlyphLoc;

  //! Indicates whether and where a dot glyph is currently under construction
  GlyphLoc glyph_where;

  //! If a cell's being used to make a dot glyph, says which cell
  unsigned short int glyph_cell;

  //! Time when the glyph under construction was begun
  TimeInterval glyph_began;
  //! Time of the last dot entry for the glyph under construction
  TimeInterval glyph_last;

  //! Dots for the current glyph under development
  unsigned char glyph_dots;

  //! Constructor
  inline FunctorIOEventDecoder(std::deque<BaseIOEvent> &my_in_bevents,
			       std::deque<IOEvent> &my_new_events,
			       boost::mutex &my_mutex_in_bevents,
			       boost::mutex &my_mutex_new_events,
			       boost::condition &my_cond_in_bevents,
			       boost::condition &my_cond_new_events,
			       TimeInterval &my_glyph_delay,
			       boost::mutex &my_mutex_glyph_delay,
			       const Charset* &my_charset,
			       boost::mutex &my_mutex_charset,
			       std::set<IOEvent::Type> &my_watchset,
			       boost::mutex &my_mutex_watchset)
  : in_bevents(my_in_bevents), new_events(my_new_events),
    mutex_in_bevents(my_mutex_in_bevents),
    mutex_new_events(my_mutex_new_events),
    cond_in_bevents(my_cond_in_bevents), cond_new_events(my_cond_new_events),
    glyph_delay(my_glyph_delay), mutex_glyph_delay(my_mutex_glyph_delay),
    charset(my_charset), mutex_charset(my_mutex_charset),
    watchset(my_watchset), mutex_watchset(my_mutex_watchset),
    glyph_where(NONE), glyph_cell(INVALID_CELL), glyph_dots(0) { }

  //! Perform this functor's function
  inline void operator()()
  {
    // Loop forever---grab BaseIOEvent when available.
    for(;;) {
      // Will be true if we timed out waiting for new BaseIOEvent to come---
      // in that case, if no pin or button is active where the glyph is
      // being made, we indicate that the glyph under construction is done.
      bool timedout = false;

      // True iff new IOEvent events were made in this iteration
      bool made_new_events = false;

      // True iff we've been commanded by a special BaseIOEvent to flush
      // the current glyph
      bool flush_glyph = false;

      { // ENCLOSING BLOCK: For grabbing in_bevents mutex
      boost::mutex::scoped_lock lock_b(mutex_in_bevents);

      // If there are no events waiting, wait for new events to arise.
      // Note special handling if there is already a glyph underway.
      if(in_bevents.empty()) {
	if(glyph_where == NONE) cond_in_bevents.wait(lock_b);
	else {
	  // Add in timeout. First we need to know the xtime for when the
	  // timeout happents
	  boost::xtime time_end;
	  boost::xtime_get(&time_end, boost::TIME_UTC_);
	  time_end.sec += glyph_delay.secs;
	  const unsigned int nsecs = glyph_delay.msecs * 1000000;
	  time_end.nsec += nsecs % 1000000000;
	  time_end.sec  += nsecs / 1000000000;
	  // Then wait for new indications
	  timedout = !cond_in_bevents.timed_wait(lock_b, time_end);
	}
      }

      // If the input BaseIOEvent is empty, it could mean two things: either
      // we just timed out or we're closing up shop. In the latter case, we
      // signal a DONE IOEvent to the other thread to indicate that it's time
      // to close up shop. A DONE type BaseIOEvent will also tell us to quit.
      // Note that using empty event queues for notifying a thread that it's
      // time to quit is deprecated.
      if(in_bevents.empty()) {
	if(!timedout) {
	  boost::mutex::scoped_lock lock_n(mutex_new_events);
	  new_events.push_back(IOEvent::makeDoneEvent());
	  cond_new_events.notify_one();
	  return;
	}
      }
      else {
	// Grab mutexes
	boost::mutex::scoped_lock lock_n(mutex_new_events);
	boost::mutex::scoped_lock lock_w(mutex_watchset);
	boost::mutex::scoped_lock lock_c(mutex_charset);
	std::deque<BaseIOEvent>::const_iterator ib_iter;

	// Will be useful for determining whether new events were made
	const unsigned int old_new_events_size = new_events.size();

        // Check for DONE BaseIOEvent; if so, push a DONE IOEvent and then
        // quit.
	for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	  if(ib_iter->type == BaseIOEvent::DONE) {
	    new_events.push_back(IOEvent::makeDoneEvent());
	    cond_new_events.notify_one();
	    return;
	  }

	// Check for FLUSH_GLYPH event; if so, activate the "timeout" portion
	// of this routine---the one that reports the current glyph. Note that
	// if lots of events pile up, this approach might cause trouble: one
	// possible scenario involves the user building up an event queue like
	// this:
	//    press button 1
	//    press button 2
	//    flush glyph
	//    stylus c1d1
	//    stylus c1d2
	// The user actually wanted the button glyph flushed, but then decided
	// to begin working on a new glyph with the stylus. The button glyph
	// is flushed anyway, but with the flush glyph flag on, the system
	// will go ahead and flush the stylus glyph too regardless of whether
	// the user is done. This is something that we will mark as FIXME, but
	// it shouldn't be an issue unless the user is fast enough to build
	// up many events in the queue (or unless the computer is REALLY slow).
	for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	  if(ib_iter->type == BaseIOEvent::FLUSH_GLYPH)
	    { flush_glyph = true; break; }

	// First easy ones: if the user is subscribed to events that represent
	// the same thing as BaseIOEvent objects
	if(watchset.find(IOEvent::STYLUS_DOWN) != watchset.end())
	  for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	    if(ib_iter->type == BaseIOEvent::STYLUS_DOWN)
	      new_events.push_back(
		IOEvent::makeStylusDownEvent(
		  ib_iter->timestamp, ib_iter->cell, ib_iter->dot));

	if(watchset.find(IOEvent::STYLUS_UP) != watchset.end())
	  for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	    if(ib_iter->type == BaseIOEvent::STYLUS_UP)
	      new_events.push_back(
		IOEvent::makeStylusUpEvent(
		  ib_iter->timestamp, ib_iter->cell, ib_iter->dot));

	if(watchset.find(IOEvent::BUTTON_DOWN) != watchset.end())
	  for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	    if(ib_iter->type == BaseIOEvent::BUTTON_DOWN)
	      new_events.push_back(
		IOEvent::makeButtonDownEvent(
		  ib_iter->timestamp, ib_iter->button));

	if(watchset.find(IOEvent::BUTTON_UP) != watchset.end())
	  for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	    if(ib_iter->type == BaseIOEvent::BUTTON_UP)
	      new_events.push_back(
		IOEvent::makeButtonUpEvent(
		  ib_iter->timestamp, ib_iter->button));

	// Next, intermediate difficulty: STYLUS and BUTTON events
	if(watchset.find(IOEvent::STYLUS) != watchset.end())
	  for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter){
	    // Stylus up? Make indication
	    if(ib_iter->type == BaseIOEvent::STYLUS_UP) {
	      std::deque<BaseIOEvent>::iterator a_iter;
	      for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter)
		if((a_iter->type == BaseIOEvent::STYLUS_DOWN) &&
		   (a_iter->cell==ib_iter->cell) &&
		   (a_iter->dot==ib_iter->dot)) {
		  new_events.push_back(
		    IOEvent::makeStylusEvent(a_iter->timestamp,
		      ib_iter->timestamp - a_iter->timestamp,
		      a_iter->cell, a_iter->dot));
		  break;
		}
	    }
	  }

	if(watchset.find(IOEvent::BUTTON) != watchset.end())
	  for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter){
	    // Button up? Make indication
	    if(ib_iter->type == BaseIOEvent::BUTTON_UP) {
	      std::deque<BaseIOEvent>::iterator a_iter;
	      for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter)
		if((a_iter->type == BaseIOEvent::BUTTON_DOWN) &&
		   (a_iter->button == ib_iter->button)) {
		  new_events.push_back(
		    IOEvent::makeButtonEvent(a_iter->timestamp,
		      ib_iter->timestamp - a_iter->timestamp, a_iter->button));
		  break;
		}
	    }
	  }

	// We maintain the actives list regardless of whether anyone needs
	// it---it's easier that way, especially with the glyph timeout code.
	// First handle the _UP events---find their _DOWN twins in actives
	// and delete them.
	for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	  if(ib_iter->type == BaseIOEvent::STYLUS_UP) {
	    std::deque<BaseIOEvent>::iterator a_iter;
	    for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter)
	      if((a_iter->type == BaseIOEvent::STYLUS_DOWN) &&
		 (a_iter->cell==ib_iter->cell) &&
		 (a_iter->dot==ib_iter->dot)) break;
	    assert(a_iter != actives.end());
	    actives.erase(a_iter);
	  }
	for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	  if(ib_iter->type == BaseIOEvent::BUTTON_UP) {
	    std::deque<BaseIOEvent>::iterator a_iter;
	    for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter)
	      if((a_iter->type == BaseIOEvent::BUTTON_DOWN) &&
		 (a_iter->button == ib_iter->button)) break;
	    assert(a_iter != actives.end());
	    actives.erase(a_iter);
	  }
	// Next we add DOWN events to the actives list
	for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter)
	  if((ib_iter->type == BaseIOEvent::STYLUS_DOWN) ||
	     (ib_iter->type == BaseIOEvent::BUTTON_DOWN))
	    actives.push_back(*ib_iter);

	// Next, high difficulty: DOTS and LETTER events
	const bool want_cell_start =
	  (watchset.find(IOEvent::CELL_START) != watchset.end());
	const bool want_button_start =
	  (watchset.find(IOEvent::BUTTON_START) != watchset.end());
	const bool want_cell_done =
	  (watchset.find(IOEvent::CELL_DONE) != watchset.end());
	const bool want_button_done =
	  (watchset.find(IOEvent::BUTTON_DONE) != watchset.end());
	const bool want_cell_dots =
	  (watchset.find(IOEvent::CELL_DOTS) != watchset.end());
	const bool want_button_dots =
	  (watchset.find(IOEvent::BUTTON_DOTS) != watchset.end());
	const bool want_cell_letter =
	  (watchset.find(IOEvent::CELL_LETTER) != watchset.end());
	const bool want_button_letter =
	  (watchset.find(IOEvent::BUTTON_LETTER) != watchset.end());

	for(ib_iter=in_bevents.begin(); ib_iter!=in_bevents.end(); ++ib_iter) {
	  // Handle glyphmaking on the buttons
	  if((ib_iter->type == BaseIOEvent::BUTTON_DOWN) &&
	     (ib_iter->dot != INVALID_DOT)) {
	    // See if the user is starting a new glyph on the buttons
	    if(glyph_where != BUTTONS) {
	      // If an old glyph was being made on a cell, signal completion
	      if(glyph_where == CELL) {
		const TimeInterval duration = glyph_last - glyph_began;
		if(want_cell_done)
		  new_events.push_back(
		    IOEvent::makeCellDoneEvent(glyph_last,glyph_cell,duration));
		if(want_cell_dots)
		  new_events.push_back(
		    IOEvent::makeCellDotsEvent(glyph_began, duration,
					       glyph_cell, glyph_dots));
		if(want_cell_letter)
		  new_events.push_back(
		    IOEvent::makeCellLetterEvent(glyph_began,
		      duration, glyph_cell,
		      charset->mir()[glyph_dots], glyph_dots));
	      }
	      // Then prepare glyph tracking variables
	      glyph_where = BUTTONS;
	      glyph_began = glyph_last = ib_iter->timestamp;
	      glyph_dots = dot_mask(ib_iter->dot);
	      // Signal new glyph under construction
	      if(want_button_start)
		new_events.push_back(
		  IOEvent::makeButtonStartEvent(ib_iter->timestamp));
	    }
	    // User is continuing a glyph on the buttons
	    else {
	      glyph_dots |= dot_mask(ib_iter->dot);
	      glyph_last = ib_iter->timestamp;
	    }
	  }
	  // Handle glyphmaking in a braille cell
	  else if(ib_iter->type == BaseIOEvent::STYLUS_DOWN) {
	    // See if the user is starting a new glyph after timing out
	    if(glyph_where == NONE) {
	      // Prepare glyph tracking variables
	      glyph_where = CELL;
	      glyph_began = glyph_last = ib_iter->timestamp;
	      glyph_cell = ib_iter->cell;
	      glyph_dots = dot_mask(ib_iter->dot);
	      // Signal new glyph under construction
	      if(want_cell_start)
		new_events.push_back(
		  IOEvent::makeCellStartEvent(ib_iter->timestamp, glyph_cell));
	    }
	    // See if user was making a glyph on buttons beforehand
	    else if(glyph_where == BUTTONS) {
	      // Signal completion of old glyph
	      const TimeInterval duration = glyph_last - glyph_began;
	      if(want_button_done)
		new_events.push_back(
		  IOEvent::makeButtonDoneEvent(glyph_last, duration));
	      if(want_button_dots)
		new_events.push_back(
		  IOEvent::makeButtonDotsEvent(glyph_began,
					       duration, glyph_dots));
	      if(want_button_letter)
		new_events.push_back(
		  IOEvent::makeButtonLetterEvent(glyph_began, duration,
		    charset->mir()[glyph_dots], glyph_dots));
	      // Then prepare glyph tracking variables
	      glyph_where = CELL;
	      glyph_began = glyph_last = ib_iter->timestamp;
	      glyph_cell = ib_iter->cell;
	      glyph_dots = dot_mask(ib_iter->dot);
	      // Signal new glyph under construction
	      if(want_cell_start)
		new_events.push_back(
		  IOEvent::makeCellStartEvent(ib_iter->timestamp, glyph_cell));
	    }
	    // See if user is starting a glyph in a new cell
	    else if(glyph_cell != ib_iter->cell) {
	      // Signal completion of old glyph
	      const TimeInterval duration = glyph_last - glyph_began;
	      if(want_cell_done)
		new_events.push_back(
		  IOEvent::makeCellDoneEvent(glyph_last, glyph_cell, duration));
	      if(want_cell_dots)
		new_events.push_back(
		  IOEvent::makeCellDotsEvent(glyph_began, duration,
					     glyph_cell, glyph_dots));
	      if(want_cell_letter)
		new_events.push_back(
		  IOEvent::makeCellLetterEvent(glyph_began,duration,glyph_cell,
		    charset->mir()[glyph_dots], glyph_dots));
	      // Then prepare glyph tracking variables
	      glyph_where = CELL;
	      glyph_began = glyph_last = ib_iter->timestamp;
	      glyph_cell = ib_iter->cell;
	      glyph_dots = dot_mask(ib_iter->dot);
	      // Signal new glyph under construction
	      if(want_cell_start)
		new_events.push_back(
		  IOEvent::makeCellStartEvent(ib_iter->timestamp, glyph_cell));
	    }
	    // User is continuing a glyph in this cell
	    else {
	      glyph_dots |= dot_mask(ib_iter->dot);
	      glyph_last = ib_iter->timestamp;
	    }
	  }
	  // Handle button up timekeeping for glyphmaking
	  else if((ib_iter->type == BaseIOEvent::BUTTON_UP) &&
		  (ib_iter->dot != INVALID_DOT) &&
		  (glyph_where == BUTTONS))
	    glyph_last = ib_iter->timestamp;
	  // Handle stylus up timekeeping for glyphmaking
	  else if((ib_iter->type == BaseIOEvent::STYLUS_UP) &&
		  (glyph_where == CELL))
	    glyph_last = ib_iter->timestamp;
	}

	// Clear out input events---we've seen 'em all now
	in_bevents.clear();
	// See if new events were made
	if(old_new_events_size != new_events.size()) made_new_events = true;
      }

      // If we've timed out or been told to flush the current glyph, let's
      // check to see if there's a glyph underway and if so, signal that
      // it's done.
      if((timedout || flush_glyph) && (glyph_where != NONE)) {
	// grab watchset, new_events mutexes
	boost::mutex::scoped_lock lock_n(mutex_new_events);
	boost::mutex::scoped_lock lock_w(mutex_watchset);
	boost::mutex::scoped_lock lock_c(mutex_charset);

	// Will be useful for determining whether new events were made
	const unsigned int old_new_events_size = new_events.size();

	// First scan the actives lists to check whether any buttons or
	// cell pins are still active
	bool button_active = false;
	bool stylus_active = false;
	std::deque<BaseIOEvent>::const_iterator a_iter;
	for(a_iter=actives.begin(); a_iter!=actives.end(); ++a_iter) {
	  if((a_iter->type == BaseIOEvent::BUTTON_DOWN) &&
	     (a_iter->dot != INVALID_DOT)) button_active = true;
	  if(a_iter->type == BaseIOEvent::STYLUS_DOWN) stylus_active = true;
	  if(button_active && stylus_active) break;
	}

	if((glyph_where == BUTTONS) && (!button_active)) {
	  const TimeInterval duration = glyph_last - glyph_began;
	  if(watchset.find(IOEvent::BUTTON_DONE) != watchset.end())
	    new_events.push_back(
	      IOEvent::makeButtonDoneEvent(glyph_last, duration));
	  if(watchset.find(IOEvent::BUTTON_DOTS) != watchset.end())
	    new_events.push_back(
	      IOEvent::makeButtonDotsEvent(glyph_began, duration, glyph_dots));
	  if(watchset.find(IOEvent::BUTTON_LETTER) != watchset.end())
	    new_events.push_back(
	      IOEvent::makeButtonLetterEvent(glyph_began, duration,
		charset->mir()[glyph_dots], glyph_dots));
	}
	else if((glyph_where == CELL) && (!stylus_active)) {
	  const TimeInterval duration = glyph_last - glyph_began;
	  if(watchset.find(IOEvent::CELL_DONE) != watchset.end())
	    new_events.push_back(
	      IOEvent::makeCellDoneEvent(glyph_last, glyph_cell, duration));
	  if(watchset.find(IOEvent::CELL_DOTS) != watchset.end())
	    new_events.push_back(
	      IOEvent::makeCellDotsEvent(glyph_began, duration,
					 glyph_cell, glyph_dots));
	  if(watchset.find(IOEvent::CELL_LETTER) != watchset.end())
	    new_events.push_back(
	      IOEvent::makeCellLetterEvent(glyph_began, duration, glyph_cell,
		charset->mir()[glyph_dots], glyph_dots));
	}

	// Set glyph tracking variables to "no glyph"
	if(!(button_active | stylus_active)) {
	  glyph_where = NONE;
	  glyph_dots = (unsigned char) 0x00;
	}

	// See if events were made
	if(old_new_events_size != new_events.size()) made_new_events = true;
      }

      } // END ENCLOSING BLOCK: Releasing in_bevents mutex

      // If new events were made, send up notification
      if(made_new_events) cond_new_events.notify_one();
    }
  }
};

//! The thread functor that calls the IOEventHandler callback on new events
struct FunctorNewIOEvent {
  //! Reference to new events decoded by the decoder thread
  std::deque<IOEvent> &new_events;
  //! Reference to mutex for new IOEvent events
  boost::mutex &mutex_new_events;
  //! Reference to condition variable for new IOEvent events
  boost::condition &cond_new_events;
  //! Reference to the IOEventParser object we're manipulating
  IOEventParser &iep;

  //! Constructor---fill in references
  inline FunctorNewIOEvent(std::deque<IOEvent> &my_new_events,
			    boost::mutex &my_mutex_new_events,
			    boost::condition &my_cond_new_events,
			    IOEventParser &my_iep)
  : new_events(my_new_events), mutex_new_events(my_mutex_new_events),
    cond_new_events(my_cond_new_events), iep(my_iep) { }

  //! Perform this functor's function
  inline void operator()()
  {
    // Loop forever---grab new events when available
    for(;;) {
      // A local copy of incoming events
      std::deque<IOEvent> local_new_events;

      { // ENCLOSING BLOCK: For getting new events
      // Grab lock on new_events queue and wait for new input
      boost::mutex::scoped_lock lock_n(mutex_new_events);

      // If there are new events now, process them immediately. Otherwise
      // wait for new ones to come.
      if(new_events.empty()) {
	cond_new_events.wait(lock_n);

	// If there are no new events after a wait, we should quit. We
	// schedule a new DONE event and proceed normally. NOTE: Using empty
	// event queues to direct quitting through this mechanism is deprecated.
	if(new_events.empty())
	  new_events.push_back(IOEvent::makeDoneEvent());
      }

      // Copy the events to a local deque of events
      std::back_insert_iterator<std::deque<IOEvent> >
	lne_inserter(local_new_events);
      std::copy(new_events.begin(), new_events.end(), lne_inserter);

      // Clear out new events queue
      new_events.clear();
      } // END ENCLOSING BLOCK

      { // ENCLOSING BLOCK: For transferring new events.
      // Grab lock on IOEventParser events queue
      boost::mutex::scoped_lock lock_i(iep.out_events_mutex);

      // Move new events to the out_events queue; if we hit a DONE event, then
      // pass the events leading up to it on to the handler and then quit.
      std::deque<IOEvent>::const_iterator i;
      for(i=local_new_events.begin(); i!=local_new_events.end(); ++i) {
	iep.out_events.push_back(*i);
	if(i->type == IOEvent::DONE) {
	  if(iep.handler) (*iep.handler)(iep.out_events);
	  return;
	}
      }
      } // END ENCLOSING BLOCK

      // Calling the IOEventHandler, if it exists
      if(iep.handler) {
	boost::mutex::scoped_lock lock_i(iep.out_events_mutex);
	(*iep.handler)(iep.out_events);
      }
    }
  }
};

//// IOEventParserCore definition ////

//! Actual functional implementation of the IOEventParser
class IOEventParserCore : public boost::noncopyable {
public:
  //! The actual implementation of IOEventParser::operator()
  void operator()(std::deque<BaseIOEvent> &events);

  //! The actual implementation of IOEventParser::setGlyphDelay
  inline void setGlyphDelay(const TimeInterval &delay);

  //! The actual implementation of IOEventParser::flushGlyph
  inline void flushGlyph();

  //! The actual implementation of IOEventParser::wantEvent
  inline void wantEvent(const IOEvent::Type &type);

  //! The actual implementation of IOEventParser::ignoreEvent
  inline void ignoreEvent(const IOEvent::Type &type);

  //! The actual implementation of IOEventParser::clearEvents
  inline void clearEvents();

  //! The actual implementation of IOEventParser::setCharset
  inline void setCharset(const Charset &my_charset);

  //! Constructor.

  //! The constructor starts the decoder and event dispatcher threads
  IOEventParserCore(IOEventParser &my_iep);

  //! Destructor. Commands thread death.
  ~IOEventParserCore();

private:
  //! IOEventParser object whose guts we manipulate
  IOEventParser &iep;

  //! BaseIOEvent events received for parsing
  std::deque<BaseIOEvent> in_bevents;
  //! Freshly parsed IOEvent events
  std::deque<IOEvent> new_events;

  //! Mutex for in_bevents
  boost::mutex mutex_in_bevents;
  //! Mutex for new_events
  boost::mutex mutex_new_events;

  //! Condition variable for in_bevents
  boost::condition cond_in_bevents;
  //! Mutex for in_bevents
  boost::condition cond_new_events;

  //! Timeout for determining when a user has finished entering a glyph
  TimeInterval glyph_delay;
  //! Mutex for the glyph delay
  boost::mutex mutex_glyph_delay;

  //! Pointer to the current character set
  const Charset *charset;
  //! Mutex for the charset pointer
  boost::mutex mutex_charset;

  //! Pointer to the current character set
  std::set<IOEvent::Type> watchset;
  //! Mutex for the charset pointer
  boost::mutex mutex_watchset;

  //! IOEvent decoder thread
  boost::scoped_ptr<boost::thread> t_fied;
  //! New IOEvent reporter thread
  boost::scoped_ptr<boost::thread> t_fnie;
};


///////////////////////////////////
//// IOEventParserCore METHODS ////
///////////////////////////////////

// Copies incoming BaseIOEvent to our own queue and notifies the
// decoder thread
void IOEventParserCore::operator()(std::deque<BaseIOEvent> &events)
{
  boost::mutex::scoped_lock lock_b(mutex_in_bevents);
  in_bevents.insert(in_bevents.end(), events.begin(), events.end());
  events.clear();
  cond_in_bevents.notify_one();
}

// Alter the glyph delay.
void IOEventParserCore::setGlyphDelay(const TimeInterval &my_glyph_delay)
{ // Grab lock on the glyph_delay variable and change it
  boost::mutex::scoped_lock lock_gd(mutex_glyph_delay);
  glyph_delay = my_glyph_delay; }

// Force interpretation of the current glyph under construction
void IOEventParserCore::flushGlyph()
{
  boost::mutex::scoped_lock lock_i(mutex_in_bevents);
 
  // The decoder uses a custom FLUSH_GLYPH indication to learn that it's
  // finished. KLUDGE.
  in_bevents.push_back(BaseIOEvent::makeFlushGlyphEvent());

  // Notify threads that their deadly input is ready.
  cond_in_bevents.notify_one();
}

// Indicate that an event should be monitored
void IOEventParserCore::wantEvent(const IOEvent::Type &type)
{ boost::mutex::scoped_lock lock_w(mutex_watchset); watchset.insert(type); }

//! Indicate that an event should be ignored
void IOEventParserCore::ignoreEvent(const IOEvent::Type &type)
{ boost::mutex::scoped_lock lock_w(mutex_watchset);
  std::set<IOEvent::Type>::iterator w_iter = watchset.find(type);
  if(w_iter != watchset.end()) watchset.erase(w_iter); }

//! Indicate that all events should be ignored
void IOEventParserCore::clearEvents()
{ boost::mutex::scoped_lock lock_w(mutex_watchset); watchset.clear(); }

//! Change the current character set
void IOEventParserCore::setCharset(const Charset &my_charset)
{ boost::mutex::scoped_lock lock_c(mutex_charset); charset = &my_charset; }

// IOEventParserCore constructor
IOEventParserCore::IOEventParserCore(IOEventParser &my_iep)
: iep(my_iep),
  glyph_delay(5U), // five second default glyph delay
  charset(&Charset::defaultCharset()),
  t_fied(
   new boost::thread(
     FunctorIOEventDecoder(in_bevents, new_events,
			   mutex_in_bevents, mutex_new_events,
			   cond_in_bevents, cond_new_events,
			   glyph_delay, mutex_glyph_delay,
			   charset, mutex_charset, watchset, mutex_watchset))),
  t_fnie(
   new boost::thread(
     FunctorNewIOEvent(new_events, mutex_new_events, cond_new_events, iep)))
{ }

// IOEventParserCore destructor
IOEventParserCore::~IOEventParserCore()
{
  {
    // Grab mutexes for input and output events
    boost::mutex::scoped_lock lock_n(mutex_new_events);
    boost::mutex::scoped_lock lock_i(mutex_in_bevents);

    // The convention of the threads (one of them) is that if the input
    // queues they're waiting on contain nothing, then they should quit.
    in_bevents.clear();
    new_events.clear();

    // Indicate to the decoder and the new IO event functor that we are
    // finished. The former would tell the latter anyway, but here we cut out
    // the middleman.
    in_bevents.push_back(BaseIOEvent::makeDoneEvent());
    new_events.push_back(IOEvent::makeDoneEvent());

    // Notify threads that their deadly input is ready.
    cond_in_bevents.notify_one();
    cond_new_events.notify_one();
  }

  // Now we wait for everyone to die
  if(t_fied) t_fied->join();
  if(t_fnie) t_fnie->join();
}

///////////////////////////////
//// IOEventParser METHODS ////
///////////////////////////////

// IOEventParser constructor
IOEventParser::IOEventParser()
: handler(NULL), iepc(new IOEventParserCore(*this)) { }

// Callback method for new BaseIOEvent
void IOEventParser::operator()(std::deque<BaseIOEvent> &events)
{ if(iepc != NULL) (*iepc)(events); }

// Sets time delay for user to be finished with a glyph
void IOEventParser::setGlyphDelay(const TimeInterval &delay)
{ if(iepc != NULL) iepc->setGlyphDelay(delay); }

// Flushes the current glyph
void IOEventParser::flushGlyph()
{ if(iepc != NULL) iepc->flushGlyph(); }

// Adds an IOEvent type to the event watchset
void IOEventParser::wantEvent(const IOEvent::Type &type)
{ if(iepc != NULL) iepc->wantEvent(type); }

// Removes an IOEvent type from the event watchset
void IOEventParser::ignoreEvent(const IOEvent::Type &type)
{ if(iepc != NULL) iepc->ignoreEvent(type); }

// Removes all IOEvent types from the event watchset
void IOEventParser::clearEvents() { if(iepc != NULL) iepc->clearEvents(); }

// Register an IOEventHandler functor with this IOEventParser
void IOEventParser::setIOEventHandler(IOEventHandler &ioeh)
{
  // We grab the mutex so that nobody calls the handler during the switch
  boost::mutex::scoped_lock lock(out_events_mutex);
  handler = &ioeh;
}

// Call an IOEventHandler on the events list immediately
void IOEventParser::pollIOEvents(IOEventHandler &ioeh)
{
  boost::mutex::scoped_lock lock(out_events_mutex);
  ioeh(out_events);
}

// Sets the current braille character set
void IOEventParser::setCharset(const Charset &my_charset)
{ if(iepc != NULL) iepc->setCharset(my_charset); }

// IOEventParser deconstructor
IOEventParser::~IOEventParser() { if(iepc != NULL) delete iepc; }

} // namespace BrailleTutorNS
