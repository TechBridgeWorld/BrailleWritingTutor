#ifndef _LIBBT_IO_EVENT_H_
#define _LIBBT_IO_EVENT_H_
/*
 * Braille Tutor interface library
 * IOEvent.h, started 24 May 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Headers for the IOEvent BaseIOEventHandler, which converts basic
 * Braille Tutor events (e.g. stylus insertions, etc.) into more
 * complex events (e.g. whole braille letters).
 */

#include "Dots.h"
#include "Types.h"
#include "Charset.h"

#include <deque>

#include <boost/thread.hpp>
#include <boost/utility.hpp>

namespace BrailleTutorNS {

//! A datatype describing Braille Tutor events of intermediate complexity.

//! This class describes several kinds of events for the Braille Tutor.
//! It includes low-level events (button presses, stylus
//! insertions/extractions) but also more complex events as well,
//! including combinations of dots within a braille cell and braille
//! letters. In addition to a timestamp, the class includes a
//! duration measure that indicates how long it took the user to complete
//! the event action, e.g. writing a letter. Note that multiple events
//! can describe the same action: the same gestures that can cause a
//! braille letter event can also yield multiple stylus events, for
//! example.
//!
//! For detecting high level events like writing a letter or any "glyph"
//! (i.e. combination of dots within a cell or on the buttons), the system
//! contains some heuristics to determine when the user has completed the
//! task (see IOEventParser for more information). The *_START and *_DONE
//! events signal the beginning and end of heuristically determined glyph
//! input periods.
//!
//! Note the existance of a DONE event, which signals that this BrailleTutor
//! instance is about to be destroyed (e.g. due to program quit). If your
//! IOEvent handler receives this event, it should handle it appropriately.
//! DONE events cannot be ignored.
struct IOEvent {
  //! IO Event type symbols

  //! IO Event type symbols, in order from lowest level to highest level.
  //! Events noted as instantaneous will always have duration 0.
  typedef enum {
      STYLUS_DOWN,	//!< Stylus insertion into a dot (instantaneous)
      STYLUS_UP,	//!< Stylus withdrawal from a dot (instantaneous)
      BUTTON_DOWN,	//!< Button press (instantaneous)
      BUTTON_UP,	//!< Button release (instantaneous)

      //! Complete stylus event: duration is from insertion to withdrawal.
      STYLUS,
      //! Complete button event; duration is from press to release.
      BUTTON,

      //! System thinks user is starting a new glyph in a cell (instantaneous)
      CELL_START,
      //! System thinks user has completed a glyph in a cell (in duration time)
      CELL_DONE,

      //! System thinks user is starting a new glyph on buttons (instantaneous)
      BUTTON_START,
      //! System thinks user has completed a glyph on buttons (in duration time)
      BUTTON_DONE,

      CELL_DOTS,	//!< User enters a dot pattern glyph in a cell
      BUTTON_DOTS,	//!< User enters a dot pattern glyph on buttons

      CELL_LETTER,	//!< User enters a letter in a cell
      BUTTON_LETTER,	//!< User enters a letter on buttons

      DONE		//!< The app is terminating; clean up!
  } Type;

  //! Event type
  Type type;

  // Anonymous union promotes space saving for heterogeneous event types
  union {
    unsigned short int cell;	//!< Cell in which the stylus event occurred.
    unsigned short int button;	//!< Button emitting the button event.
  };

  // Anonymous union promotes space saving for heterogeneous event types
  union {
    //! Cell dot in which the stylus event occurred.
    unsigned char dot;
    //! Bitmap of a dot pattern glyph; use DOT_* variables to decompose.
    unsigned char dots;
  };


  TimeInterval timestamp;	//!< Timestamp of the event
  TimeInterval duration;	//!< Duration of the event

  //! Letter or word interpretation of the user's dot glyph. It's OK to change
  //! the data in this GlyphMapping; it's a separate copy.
  GlyphMapping letter;

private:
  //! Private constructor

  //! This constructor is private because setting IOEvent values directly
  //! could yield inconsistent representations. Use the named constructors
  //! make*Event instead.
  IOEvent(const TimeInterval &my_timestamp, const TimeInterval &my_duration,
	  const Type &my_type, const unsigned short int &cell_or_button,
	  const unsigned char &dot_or_dots=INVALID_DOT,
	  const GlyphMapping &my_letter=GlyphMapping())
  : type(my_type), cell(cell_or_button), dot(dot_or_dots),
    timestamp(my_timestamp), duration(my_duration), letter(my_letter.dup()) { }

public:
  //! Named constructor for making STYLUS_DOWN events
  inline static IOEvent makeStylusDownEvent(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_cell,
				const unsigned char &my_dot)
  { if(my_dot == INVALID_DOT)
      throw BTException(BTException::BT_EINVAL,
			 "IOEvent::makeStylusDownEvent: invalid dot argument");
    return IOEvent(my_timestamp, TimeInterval(), STYLUS_DOWN,
		   my_cell, my_dot); }

  //! Named constructor for making STYLUS_UP events
  inline static IOEvent makeStylusUpEvent(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_cell,
				const unsigned char &my_dot)
  { if(my_dot == INVALID_DOT)
      throw BTException(BTException::BT_EINVAL,
			   "IOEvent::makeStylusUpEvent: invalid dot argument");
    return IOEvent(my_timestamp, TimeInterval(), STYLUS_UP, my_cell, my_dot); }

  //! Named constructor for creating BUTTON_DOWN events
  inline static IOEvent makeButtonDownEvent(
                                const TimeInterval &my_timestamp,
                                const unsigned short int &my_button)
  { return IOEvent(my_timestamp, TimeInterval(), BUTTON_DOWN, my_button,
		   my_button > 0 ? my_button-1 : INVALID_DOT); }

  //! Named constructor for creating BUTTON_UP events
  inline static IOEvent makeButtonUpEvent(
                                const TimeInterval &my_timestamp,
                                const unsigned short int &my_button)
  { return IOEvent(my_timestamp, TimeInterval(), BUTTON_UP, my_button,
		   my_button > 0 ? my_button-1 : INVALID_DOT); }

  //! Named constructor for creating STYLUS events
  inline static IOEvent makeStylusEvent(
    const TimeInterval &my_timestamp, const TimeInterval &my_duration,
    const unsigned short int &my_cell, const unsigned char &my_dot)
  { if(my_dot == INVALID_DOT)
      throw BTException(BTException::BT_EINVAL,
			     "IOEvent::makeStylusEvent: invalid dot argument");
    return IOEvent(my_timestamp, my_duration, STYLUS, my_cell, my_dot); }

  //! Named constructor for creating BUTTON events
  inline static IOEvent makeButtonEvent(
    const TimeInterval &my_timestamp, const TimeInterval &my_duration,
    const unsigned short int &my_button)
  { return IOEvent(my_timestamp, my_duration, BUTTON, my_button,
		   my_button > 0 ? my_button-1 : INVALID_DOT); }

  //! Named constructor for creating CELL_START events
  inline static IOEvent makeCellStartEvent(
    const TimeInterval &my_timestamp, const unsigned short int &my_cell)
  { return IOEvent(my_timestamp, TimeInterval(), CELL_START, my_cell); }

  //! Named constructor for creating CELL_DONE events
  inline static IOEvent makeCellDoneEvent(
    const TimeInterval &my_timestamp, const unsigned short int &my_cell,
    const TimeInterval &my_duration)
  { return IOEvent(my_timestamp, my_duration, CELL_DONE, my_cell); }

  //! Named constructor for creating BUTTON_START events
  inline static IOEvent makeButtonStartEvent(const TimeInterval &my_timestamp)
  { return IOEvent(my_timestamp, TimeInterval(), BUTTON_START,INVALID_BUTTON); }

  //! Named constructor for creating BUTTON_DONE events
  inline static IOEvent makeButtonDoneEvent(const TimeInterval &my_timestamp,
					    const TimeInterval &my_duration)
  { return IOEvent(my_timestamp, my_duration, BUTTON_DONE, INVALID_BUTTON); }

  //! Named constructor for creating CELL_DOTS events
  inline static IOEvent makeCellDotsEvent(
    const TimeInterval &my_timestamp, const TimeInterval &my_duration,
    const unsigned short int &my_cell, const unsigned char &my_dots)
  { return IOEvent(my_timestamp, my_duration, CELL_DOTS, my_cell, my_dots); }

  //! Named constructor for creating BUTTON_DOTS events
  inline static IOEvent makeButtonDotsEvent(
    const TimeInterval &my_timestamp, const TimeInterval &my_duration,
    const unsigned char &my_dots)
  { return IOEvent(my_timestamp, my_duration,
		   BUTTON_DOTS, INVALID_BUTTON, my_dots); }

  //! Named constructor for creating CELL_LETTER events
  inline static IOEvent makeCellLetterEvent(
    const TimeInterval &my_timestamp, const TimeInterval &my_duration,
    const unsigned short int &my_cell, const GlyphMapping &my_letter,
    const unsigned char &my_dots)
  { return IOEvent(my_timestamp, my_duration, CELL_LETTER, my_cell,
		   my_dots, my_letter); }

  //! Named constructor for creating BUTTON_LETTER events
  inline static IOEvent makeButtonLetterEvent(
    const TimeInterval &my_timestamp, const TimeInterval &my_duration,
    const GlyphMapping &my_letter, const unsigned char &my_dots)
  { return IOEvent(my_timestamp, my_duration, BUTTON_LETTER, INVALID_BUTTON,
		   my_dots, my_letter); }

  //! Named constructor for DONE events. Only to be called internally!
  inline static IOEvent makeDoneEvent()
  { return IOEvent(TimeInterval::now(), TimeInterval(), DONE,
		   INVALID_BUTTON, INVALID_DOT); }
};


//! An abstract type for a class that handles IOEvent events

//! Objects that inherit from this class may register themselves with an
//! IOEventParser object to receive and handle IOEvent events. Receiving an
//! empty queue is a signal that the BrailleTutor object is being destroyed,
//! and so any threads owned by this handler should terminate. This
//! particular class does nothing with the events.
struct IOEventHandler {
  //! Your own IOEventHandler will reimplement this noop.
  inline virtual void operator()(std::deque<IOEvent> &events) { }
  //! Virtual destructor for g++
  inline virtual ~IOEventHandler() { }
};

//! An IOEventHandler that always erases the contents of the events list.
struct IOEventHandlerZapper : public IOEventHandler {
  //! Empties the furnished events list.
  inline virtual void operator()(std::deque<IOEvent> &events)
  { events.clear(); }
  //! Virtual destructor for g++
  inline virtual ~IOEventHandlerZapper() { }
};

// Predeclaration for class IOEventParser. The IOEventParserCore class is
// private to the implementation of the Braille Tutor interface library
// and is not included in interface header files.
class IOEventParserCore;

//! Class for generating Braille Tutor events of intermediate complexity

//! A BaseIOEventHandler that takes basic Braille Tutor events (e.g.
//! button presses, stylus insertions/deletions) and creates events of
//! intermediate complexity, represented with IOEvent objects. Register
//! IOEventHandler functors with setIOEventHandler to process the events
//! generated by an IOEventHandler
class IOEventParser : public BaseIOEventHandler, public boost::noncopyable {
public:
  //! Constructor
  IOEventParser();

  //! Callback method for BrailleTutor object to supply events.
  virtual void operator()(std::deque<BaseIOEvent> &events);

  //! Set the time delay for the user to be "finished" with a glyph.

  //! If a time interval longer than delay (in seconds) elapses between
  //! the user's last button press/stylus withdrawal, the system deems the
  //! user to have completed the glyph. Will throw an BT_EDOM BTException if
  //! delay <= 0. Use a value of infinity (HUGE_VAL in cmath) for no delay.
  void setGlyphDelay(const TimeInterval &delay);

  //! Flush the current glyph

  //! Normally the IOEventParser will not analyze what the user has typed into
  //! a braille cell until the user starts using a different cell or until
  //! a timeout without input expires. This method skips the wait and
  //! processes the user's input right away. Note that if no glyph is under
  //! construction now, this routine creates no new events.
  void flushGlyph();
  
  void clearQueue();
  //! Tells the IOEventParser to start adding type of event to the event list.
  void wantEvent(const IOEvent::Type &type);
  //! Tells the IOEventParser to stop adding a type of event to the event list.
  void ignoreEvent(const IOEvent::Type &type);
  //! Tells the IOEventParser not to add any events to the event list.
  void clearEvents();

  //! Register an IOEventHandler functor with this IOEventParser.

  //! Register an IOEventHandler functor with this IOEventParser. The
  //! functor will be called whenever a new IOEvent is added to the
  //! events list. Note that the IOEventParser object does NOT keep its own
  //! copy of ioeh, so don't destroy ioeh until the IOEventParser is gone.
  void setIOEventHandler(IOEventHandler &ioeh);

  //! Call an IOEventHandler functor on the events list immediately.

  //! Calls the furnished IOEventHandler functor on the current events list.
  //! This will occur ASAP, but because another thread may be writing to
  //! the list, there's no guarantee that this will happen absolutely
  //! immediately.
  void pollIOEvents(IOEventHandler &ioeh);

  //! Set the current braille character set

  //! Sets the current character set object for transliting braille glyphs
  //! into Unicode strings. This class does not maintain a local copy of the
  //! character set object---do not destroy or alter the argument object
  //! while it's in use by an IOEventParser class.
  void setCharset(const Charset &my_charset=Charset::defaultCharset());

  //! Destructor
  virtual ~IOEventParser();

  std::deque<IOEvent> out_events;
private:
  // Allow one of the thread classes access to our innards
  friend class FunctorNewIOEvent;

  //! Pointer to the current IOEventHandler object
  IOEventHandler *handler;

  //! IOEvents accumulated by this IOEventParser object
 

  //! A mutex controlling access to out_events
  boost::mutex out_events_mutex;

  //! The actual event parser implementation

  //! Because details of the event parser are long, turgid, and possibly
  //! platform-dependent, we separate it off into this object, which performs
  //! much of the actual duties of the IOEventParser
  IOEventParserCore *iepc;
};


} // namespace BrailleTutorNS

#endif
