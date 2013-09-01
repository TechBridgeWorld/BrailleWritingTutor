#ifndef _SHORT_STYLUS_SUPPRESSOR_H_
#define _SHORT_STYLUS_SUPPRESSOR_H_
/*
 * Braille Tutor interface library
 * ShortStylusSuppressor.h, started 6 November 2007 by Tom Stepleton.
 *							(tss@ri.cmu.edu)
 * Some BrailleTutors generate spurious stylus events---when hands are
 * rubbed across the Tutor, for example, or when a stylus is being inserted
 * or removed and makes weak electrical contact. These spurious events are
 * typically pretty short. A ShortStylusSuppressor will filter stylus-related
 * BaseIOEvents where the delay between STYLUS_DOWN and STYLUS_UP is too
 * brief to have been real. All other BaseIOEvents are passed through. Note
 * that in order for this to work, genuine stylus BaseIOEvents have to be
 * delayed until their authenticity can be recognized, which may result in
 * the sequence of BaseIOEvents emitted by this object being out of temporal
 * order.
 */

#include "Types.h"

#include <boost/thread.hpp>

namespace BrailleTutorNS {

// Predefinition
class ShortStylusSuppressorCore;

//! Filters out spurious stylus-related BaseIOEvents

//! Some BrailleTutors generate spurious stylus events---when hands are
//! rubbed across the Tutor, for example, or when a stylus is being inserted
//! or removed and makes weak electrical contact. These spurious events are
//! typically pretty short. A ShortStylusSuppressor will filter stylus-related
//! BaseIOEvents where the delay between STYLUS_DOWN and STYLUS_UP is too
//! brief to have been real. All other BaseIOEvents are passed through.
//! See additional note in ShortStylusSuppressor.h
class ShortStylusSuppressor
: public BaseIOEventHandler, public boost::noncopyable {
public:
  //! Constructor
  ShortStylusSuppressor(const TimeInterval &threshold=0.75);

  //! Register a BaseIOEventHandler with this ShortStylusSuppressor.
  void setBaseIOEventHandler(BaseIOEventHandler &bioeh);

  //! Call a BaseIOEventHandler functor on the events list immediately.
  void pollBaseIOEvents(BaseIOEventHandler &bioeh);

  //! Set the minimum insertion/withdrawal-time threshold for stylus events
  void setThreshold(const TimeInterval &threshold);

  //! BaseIOEvent handler callback
  virtual void operator()(std::deque<BaseIOEvent> &events);

  //! Destructor
  virtual ~ShortStylusSuppressor();

private:
  // Allow one of the thread classes to access our innards
  friend class FunctorNewEventsSSSC;

  //! Pointer to the current BaseIOEventHandler object
  BaseIOEventHandler *handler;

  //! BaseIOEvents accumulated by this ShortStylusSuppressor object
  std::deque<BaseIOEvent> out_events;

  //! A mutex controlling access to out_events
  boost::mutex out_events_mutex;

  //! The actual guts of this ShortStylusSuppressor implementation
  ShortStylusSuppressorCore *sssc;
};

} // namespace BrailleTutorNS

#endif
