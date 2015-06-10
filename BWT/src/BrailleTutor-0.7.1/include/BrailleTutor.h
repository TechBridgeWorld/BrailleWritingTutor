#ifndef _LIBBT_BRAILLE_TUTOR_H_
#define _LIBBT_BRAILLE_TUTOR_H_
/*
 * Braille Tutor inferface library
 * BrailleTutor.h, started 25 May 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Headers for the BrailleTutor class, the fundamental interface to the
 * Braille Tutor hardware. Each BrailleTutor object instance connects to
 * and represents one BrailleTutor board connected to the computer.
 * The BrailleTutor class translates serial port (or USB serial port)
 * data into lower level Braille Tutor events (BaseIOEvent objects) which
 * are then passed to a registered BaseIOEventHandler (see Types.h).
 *
 * The target implementation of this class is multithreaded. See further
 * notes at BrailleTutor::setBaseIOEventHandler.
 */

#include "Types.h"

#include <deque>
#include <string>

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

namespace BrailleTutorNS {

// Predeclaration for class BrailleTutor. The BrailleTutorIO class is
// private to the implementation of the Braille Tutor interface library
// and is not included in interface header files.
class BrailleTutorIO;

//! Interface to a single Braille Tutor.

//! Instances of this class communicate with and translate input from
//! single Braille Tutor devices. Register a BaseIOEventHandler with
//! the instance to receive notification about new events arriving from
//! the Tutor.
class BrailleTutor : public boost::noncopyable {
public:
  //! Constructor

  //! Constructs the Braille Tutor instance, though actual internal setup
  //! may occur within the init() method due to Windows restrictions on the
  //! behavior of our thread library (Boost::Thread).
  BrailleTutor();

  //! Initializes internal resources

  //! Initializes internal resources. This method must be called once before
  //! using other BrailleTutor methods. Throws a BTException with error
  //! BT_EMISC if resources cannot be initialized, or BT_EALREADY if init()
  //! has been called already. Other methods will also throw BT_EMISC if
  //! init() has not yet been called.
  void init();

  //! Attempt autodetection of a Braille Tutor on one of the serial ports.

  //! This routine will try to autodetect a Braille Tutor on one of the
  //! serial ports. If found, the routine will then initialize the Tutor
  //! out of autodetect mode and into interactive mode. Can throw the
  //! following types of BTExceptions: BT_EIO (I/O error), BT_ENOENT (failed
  //! to detect a Tutor), BT_EMISC (BrailleTutor object not yet initialized),
  //! BT_ALREADY (Braille Tutor is already connected). On success, io_port
  //! will contain a system-specific name for the I/O port where the board
  //! was detected, and version will contain a board ROM revision number.
  void detect(std::string &io_port, unsigned int &version);

  //! Tells the BrailleTutor object about a Tutor ready on a specified port.

  //! Used in lieu of detect, this routine tells the BrailleTutor object
  //! that a Braille Tutor board with ROM revision version is ready in
  //! interactive mode on port io_port. This routine is necessary since
  //! some Braille Tutor ROM revisions have no way of putting the Tutor
  //! back in autodetect mode without power cycling. Can throw the following
  //! types of BTExceptions: BT_EIO (I/O error), BT_EMISC (BrailleTutor
  //! object not yet initialized), BT_ALREADY (Braille Tutor is already
  //! connected), BT_EINVAL (unsupported ROM version).
  void ready(const std::string &io_port, const unsigned int &version);

  //! Causes the Braille Tutor to emit a beep

  //! Causes the Braille Tutor to emit a tone at frequency freq and
  //! duration duration (specified in seconds). Both quantities will be
  //! rounded to the nearest value supported by the Tutor. Throws a BT_EDOM
  //! BTException for out-of-domain arguments (e.g. negative numbers).
  void beep(const double &freq, const double &duration);

  //! Returns the status of a binary I/O pin

  //! Returns the status of the pin'th binary I/O pin. Currently there is
  //! only one pin on the Braille Tutor, so the only valid value for pin
  //! is 0. Throws a BT_EINVAL BTException if pin designates a pin that
  //! does not exist.
  bool iopin(const unsigned int &pin);

  //! Sets the status of a binary I/O pin

  //! Sets the status of the pin'th binary I/O pin and returns the new value.
  //! Currently there is only one pin on the Braille Tutor, so the only
  //! value for pin is 0. Throws an BT_EINVAL BTExcepion if pin designates
  //! a pin that does not exist.
  bool iopin(const unsigned int &pin, const bool &state);

  //! Register a BaseIOEventHandler functor with this BrailleTutor object.

  //! Register a BaseIOEventHandler functor with this BrailleTutor object.
  //! The functor will be called whenever a new BaseIOEvent is added to the
  //! events list. Note that the BrailleTutor object does NOT keep its own
  //! copy of bioeh, so don't destroy bioeh until the BrailleTutor object
  //! is gone.
  void setBaseIOEventHandler(BaseIOEventHandler &bioeh);

  //! Call a BaseIOEventHandler functor on the events list immediately.

  //! Calls the furnished BaseIOEventHandler functor on the current events
  //! list. This will occur ASAP, but because another thread may be writing
  //! to the list, there's no guarantee that this will happen absolutely
  //! immediately.
  void pollBaseIOEvents(BaseIOEventHandler &bioeh);

  //! Attempt a "hard reset" of the Braille Tutor

  //! Attempts a "hard reset" of the Braille Tutor in order to return it to
  //! a known state: interactive mode, with the buzzer off and the I/O pin
  //! set to low. This function is useful if it's suspected that the system's
  //! own internal estimate of the Braille Tutor's current state is
  //! incorrect---which should hopefully never happen. Returns true if the
  //! system is reasonably assured that the hard reset has succeeded (should
  //! be all but guaranteed for all ROM revisions so far) and false otherwise.
  bool resetHard();

  //! Attempt a "soft reset" of the Braille Tutor

  //! Attempts a "soft reset" of the Braille Tutor in order to return it to
  //! a known state (in interactive mode) without affecting the behavior of
  //! the I/O pin or the buzzer. The success of this operation is not
  //! guaranteed for all ROM revisions. Returns true if the system thinks
  //! the reset has succeeded and false otherwise.
  bool resetSoft();

  //! Deconstructor and cleanup
  ~BrailleTutor();

  //! Cedes program control to tbe BrailleTutor I/O threads

  //! This routine waits for the BT I/O threads to terminate. Since this will
  //! never happen unless initiated deliberately (by deleting this
  //! BrailleTutor object) this routine never returns. It's therefore the
  //! conventional way to delegate program control away from the main()
  //! thread and just wait for events to dictate what happens.
  void join();

private:
  // Allow one of the thread classes access to our innards
  friend class FunctorNewEvents;

  //! Pointer to the current BaseIOEventHandler object
  BaseIOEventHandler *handler;

  //! Checks whether init has been called; otherwise throws an exception.
  void checkReady();

  //! BaseIOEvents accumulated by this BrailleTutor object
  std::deque<BaseIOEvent> out_events;

  //! A mutex controlling access to out_events
  boost::mutex out_events_mutex;

  //! The BrailleTutorIO class that's talking to the BrailleTutor for us

  //! Because details of the Braille Tutor I/O system might require different
  //! variables on different platforms, we separate it off into this separate
  //! object, which performs much of the actual duties of the BrailleTutor
  //! system.
  BrailleTutorIO *btio;
};

} // namespace BrailleTutorNS

#endif
