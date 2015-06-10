#ifndef _LIBBT_TYPES_H_
#define _LIBBT_TYPES_H_
/*
 * Braille Tutor interface library
 * Types.h, started 23 May 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Defines several basic types (and operators on those types) for the
 * Braille Tutor library.
 */

#include <cmath>
#include <deque>
#include <string>

namespace BrailleTutorNS {

//! An exception class for reporting errors and failure conditions
struct BTException {
  //! Braille Tutor library error type symbols

  //! Symbols indicating what type of error is indicated by this exception.
  //! Symbol interpretations generally match their POSIX counterparts
  //! (see errno(3) on Linux or intro(2) on MacOS X).
  typedef enum { BT_EMISC,	//!< Miscellaneous error---usually critical
		 BT_EINVAL,	//!< Invalid argument
		 BT_EDOM,	//!< Numerical argument out of function domain
		 BT_ENOENT,	//!< File or resource does not exist
		 BT_EBUSY,	//!< File or resource busy/in use
		 BT_EACCES,	//!< File or resource has access restrictions
		 BT_EIO,	//!< I/O error
		 BT_ETIMEDOUT,	//!< Operation timeout
		 BT_EALREADY,	//!< Operation already completed
		 BT_EUNKNOWN	//!< Unspecified error (should not use)
	  } Type;

  //! Error type
  Type type;

  //! Optional string with additional error information
  std::string why;

  //! Constructor without arguments
  inline BTException() : type(BT_EUNKNOWN) { }

  //! Constructor allowing specification of error type and extra information.
  inline BTException(const Type &my_type, const std::string &my_why="")
  : type(my_type), why(my_why) { }
};


//! A class for event timestamps and time intervals.

//! This class furnishes timestamps for Braille Tutor events, as well as a
//! representation for other time intervals. Time is represented as the sum
//! of seconds and milliseconds, and for timestamps the class indicates time
//! since some epoch (c.f. POSIX's struct timeb in ftime(3)). The specific
//! epoch is not specified and may change between different instances of the
//! same program; since timestamps are mainly used for computing relative
//! times, however, this is not expected to be a problem. Additional
//! operators support basic arithmetic operations (addition etc.) on
//! time values.
//!
//! Abstraction function: A time interval of xxx.yyy seconds is represented
//! in fixed-point 3-decimal place resolution with xxx in secs and yyy in msecs.
//!
//! Representation invariant: msecs is always in [0-999].
struct TimeInterval {
  //! Interval duration rounded down to the nearest smaller second.
  unsigned int secs;
  //! Number of milliseconds in interval duration - secs.
  unsigned short int msecs;

  //! Reexpress secs and msecs to satisfy the representation invariant
  inline void invariant() { secs += msecs / 1000; msecs %= 1000; }

  //! Cast operator automatically makes a floating point time value in seconds.
  inline operator double() const
  { return ((double) secs) + ((double) msecs) / 1000.0; }

  //! Constructor: create a zero length interval
  inline TimeInterval() : secs(0), msecs(0) { }

  //! Constructor: specify seconds and milliseconds explicitly
  inline TimeInterval(const unsigned int &my_secs,
		      const unsigned short int &my_msecs)
  : secs(my_secs), msecs(my_msecs) { invariant(); }

  //! Constructor: specify a floating point time value in seconds
  inline TimeInterval(const double &my_secs)
  : secs((unsigned int) floor(my_secs)),
    msecs((unsigned short int) rint(1000.0* (my_secs - (double) secs))) { }

  //! Named constructor: generate a timestamp as time since some epoch
  static TimeInterval now();

  //! Sleep (portably) for this time interval
  void sleep() const;
};

// TODO: Implement all without floating point conversion
//! Addition operator for TimeIntervals.
inline TimeInterval operator+(const TimeInterval &a,
			      const TimeInterval &b)
{ return TimeInterval( ((double) a) + ((double) b) ); }
//! Subtraction operator for TimeIntervals.
inline TimeInterval operator-(const TimeInterval &a,
			      const TimeInterval &b)
{ return TimeInterval( ((double) a) - ((double) b) ); }
//! Multiplication operator for TimeIntervals.
inline TimeInterval operator*(const TimeInterval &a,
			      const TimeInterval &b)
{ return TimeInterval( ((double) a) * ((double) b) ); }
//! Division operator for TimeIntervals.
inline TimeInterval operator/(const TimeInterval &a,
			      const TimeInterval &b)
{ return TimeInterval( ((double) a) / ((double) b) ); }
//! Less-than comparison for TimeIntervals
inline bool operator<(const TimeInterval &a, const TimeInterval &b)
{ return ((double) a) < ((double) b); }
//! Greater-than comparison for TimeIntervals
inline bool operator>(const TimeInterval &a, const TimeInterval &b)
{ return ((double) a) > ((double) b); }
//! Less-than-or-equal-to comparison for TimeIntervals
inline bool operator<=(const TimeInterval &a, const TimeInterval &b)
{ return ((double) a) <= ((double) b); }
//! Greater-than-or-equal-to comparison for TimeIntervals
inline bool operator>=(const TimeInterval &a, const TimeInterval &b)
{ return ((double) a) >= ((double) b); }


//! Placeholder/sentinel value for *IOEvent::dot during BUTTON* events
static const unsigned char INVALID_DOT = 0xff;
//! Placeholder/sentinel value for bad or unspecified dot patterns
static const unsigned char INVALID_DOTS = 0xff;
//! Placeholder/sentinel value for unspecified cell
static const unsigned short int INVALID_CELL = 0xffff;
//! Placeholder/sentinel value for unspecified button
static const unsigned short int INVALID_BUTTON = 0xffff;

//! A datatype describing basic stylus and button events.

//! This class describes the most basic Braille Tutor events: individual
//! button presses or stylus insertions/extractions. Supported events under
//! the present BT hardware include stylus "down" (insertion), stylus "up"
//! (extraction), button down (press), and button up (release).
struct BaseIOEvent {
  //! Base IO event type symbols
  typedef enum { STYLUS_DOWN,	//!< Stylus insertion into a dot
		 STYLUS_UP,	//!< Stylus withdrawal from a dot
		 BUTTON_DOWN,	//!< Button press
		 BUTTON_UP,	//!< Button release
		 DONE,
		 //!< BrailleTutor instance about to destruct; clean up!
		 FLUSH_GLYPH
		 //!< IOEventParser force flush (KLUDGE-INTERNAL USE ONLY)
	  } Type;
  // The DONE symbol is only to be used by the IOEventParser implementation

  //! Event type
  Type type;

  // Anonymous union promotes space saving for heterogenous event types
  union {
    unsigned short int cell;	//!< Cell in which the stylus event occurred.
    unsigned short int button;	//!< Button emitting the button event.
  };

  unsigned char dot;	//!< Cell dot in which the stylus event occurred.

  TimeInterval timestamp;	//!< Timestamp of the event

private:
  //! Private constructor

  //! This constructor is private because setting BaseIOEvent values
  //! directly could yield inconsistent representations. Use the named
  //! constructors make*Event instead.
  inline BaseIOEvent(const TimeInterval &my_timestamp, const Type &my_type,
		     const unsigned short int &cell_or_button,
		     const unsigned char &my_dot=INVALID_DOT)
  : type(my_type), cell(cell_or_button), dot(my_dot), timestamp(my_timestamp)
  { }

public:
  //! Named constructor for creating STYLUS_DOWN events
  inline static BaseIOEvent makeStylusDownEvent(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_cell,
				const unsigned char &my_dot)
  {
    if(my_dot == INVALID_DOT)
      throw BTException(BTException::BT_EINVAL,
		     "BaseIOEvent::makeStylusDownEvent: invalid dot argument");
    return BaseIOEvent(my_timestamp, STYLUS_DOWN, my_cell, my_dot);
  }

  //! Named constructor for creating STYLUS_UP events
  inline static BaseIOEvent makeStylusUpEvent(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_cell,
				const unsigned char &my_dot)
  {
    if(my_dot == INVALID_DOT)
      throw BTException(BTException::BT_EINVAL,
		       "BaseIOEvent::makeStylusUpEvent: invalid dot argument");
    return BaseIOEvent(my_timestamp, STYLUS_UP, my_cell, my_dot);
  }

  //! Named constructor for creating BUTTON_DOWN events
  inline static BaseIOEvent makeButtonDownEvent(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_button)
  {
    // Note that we also assign the dot field when the button is within
    // the cell---this way code interested in Braille dots can treat
    // buttons and cell holes the same way.
    if(my_button == 0) return BaseIOEvent(my_timestamp, BUTTON_DOWN, my_button);
    else return BaseIOEvent(my_timestamp, BUTTON_DOWN, my_button,
			    my_button > 0 ? my_button-1 : INVALID_DOT);
  }

  //! Named constructor for creating BUTTON_UP events
  inline static BaseIOEvent makeButtonUpEvent(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_button)
  {
    // See note at makeButtonDownEvent
    if(my_button == 0) return BaseIOEvent(my_timestamp, BUTTON_UP, my_button);
    else return BaseIOEvent(my_timestamp, BUTTON_UP, my_button,
			    my_button > 0 ? my_button-1 : INVALID_DOT);
  }

  //! Named constructor for DONE events. Only to be called internally!
  inline static BaseIOEvent makeDoneEvent()
  { return BaseIOEvent(TimeInterval::now(), DONE, INVALID_CELL, INVALID_DOT); }

  //! Named constructor for FLUSH_GLYPH events. Only to be called internally!
  inline static BaseIOEvent makeFlushGlyphEvent()
  { return BaseIOEvent(TimeInterval::now(),
		       FLUSH_GLYPH, INVALID_CELL, INVALID_DOT); }
};


//! An abstract type for a class that handles BaseIOEvent events

//! Objects that inherit from this class may register themselves with
//! the Braille Tutor library to receive and handle BaseIOEvent events.
//! Receiving an empty queue is a signal that the BrailleTutor object
//! is being destroyed, and so any threads owned by this handler should
//! terminate. This particular class does nothing with the events.
struct BaseIOEventHandler {
  //! Your own BaseIOEventHandler will reimplement this noop.
  inline virtual void operator()(std::deque<BaseIOEvent> &events) { }

  //! Virtual destructor for g++
  inline virtual ~BaseIOEventHandler() { }
};

//! A BaseIOEventHandler that always erases the contents of the events list.
struct BaseIOEventHandlerZapper : public BaseIOEventHandler {
  //! Empties the furnished events list
  inline virtual void operator()(std::deque<BaseIOEvent> &events)
  { events.clear(); }

  //! Virtual destructor for g++
  inline virtual ~BaseIOEventHandlerZapper() { }
};

} // namespace BrailleTutorNS

#endif
