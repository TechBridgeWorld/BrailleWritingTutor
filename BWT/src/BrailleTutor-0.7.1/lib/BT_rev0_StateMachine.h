#ifndef _LIBBT_BT_REV_0_STATE_MACHINE_H_
#define _LIBBT_BT_REV_0_STATE_MACHINE_H_
/*
 * BrailleTutor interface library
 * BT_rev0_StateMachine.h, started 19 June 2007 by T. Stepleton (tss@ri.cmu.edu)
 *
 * Furnishes state machine description and beep and I/O pin commands for
 * the revision 0 Braille Tutor hardware and ROM.
 */

#include <deque>
#include <stdint.h>

#include "Types.h"
#include "BT_StateMachines.h"

#include "boost/shared_ptr.hpp"

namespace BrailleTutorNS {

//! Describes behavior and commands of the revision 0 Braille Tutor

//! NOTE: Due to the ambiguous reply protocol in the revision 0 tutor,
//! the state machine cannot handle a duration byte of 'n'---otherwise
//! certain buzzer commands and a button 1 press look identical. This routine
//! will extend all durations within the range [3385.0ms ,3315.0ms) to 3330ms,
//! the next available duration.
struct BT_rev0_Description : public BT_Description {
  //! Return a state machine description of the revision 0 Braille Tutor
  virtual BT_StateMachine makeStateMachine();

  //! Return bytes for making the revision 0 tutor beep
  inline virtual std::deque<uint8_t> makeBeepBytes(const double &freq,
						   const double &len)
  {
    if(freq <= 0.0)
      throw BTException(BTException::BT_EDOM,
			"beep frequencies must be positive");
    if(len < 0.0)
      throw BTException(BTException::BT_EDOM,
			"beep durations must be non-negative");

    std::deque<uint8_t> command;
    command.push_back('b');
    const uint8_t freqbyte = (uint8_t) ((31250.0 / freq) - 1.0);
    command.push_back(freqbyte);
    uint8_t lenbyte  = (uint8_t) (len * 30.0);
    // A lenbyte of 'n' will confuse our state machine. We increment the
    // lenbyte from 'n' to 'm' if necessary.
    if(lenbyte == (uint8_t) 'n') ++lenbyte;
    command.push_back(lenbyte);
    command.push_back('n');
    return command;
  }

  //! Return bytes for retrieving the I/O pin status on a revision 0 Tutor

  //! Returns the bytes for retrieving the I/O pin status on a revision 0
  //! Tutor. Since there's only one pin, any argument to pin that isn't 0
  //! will result in a BT_EINVAL exception.
  inline virtual std::deque<uint8_t> makeGetIOPinBytes(const unsigned int &pin)
  {
    if(pin != 0)
      throw BTException(BTException::BT_EINVAL,
			"revision 0 hardware has only one I/O pin (get)");

    std::deque<uint8_t> command;
    command.push_back('e');
    command.push_back('i');
    return command;
  }

  //! Return bytes for setting the I/O pin status on a revision 0 Tutor

  //! Returns the bytes for setting the I/O pin status on a revision 0
  //! Tutor. Since there's only one pin, any argument to pin that isn't 0
  //! will result in a BT_EINVAL exception.
  inline virtual std::deque<uint8_t> makeSetIOPinBytes(const unsigned int &pin,
						       const bool &pinstate)
  {
    if(pin != 0)
      throw BTException(BTException::BT_EINVAL,
			"revision 0 hardware has only one I/O pin (set)");

    std::deque<uint8_t> command;
    command.push_back('e');
    command.push_back('o');
    command.push_back(pinstate ? '1' : '0');
    return command;
  }

  //! Returns bytes for putting a revision 0 Tutor into interactive mode

  //! These bytes should only be used to move the BT from autodetect to
  //! interactive mode.
  inline virtual std::deque<uint8_t> makeInteractiveModeBytes()
  {
    std::deque<uint8_t> command;
    command.push_back('b');
    command.push_back('t');
    return command;
  }

  //! Return bytes for resetting to a known state on a revision 0 Tutor

  //! Returns the bytes that should usually (always?) return the tutor
  //! to the "base" state (named in dest).
  inline virtual std::deque<uint8_t> makeResetBytes(BTSM_stateNameT &dest)
  {
    std::deque<uint8_t> command;
    command.push_back(0); command.push_back(0); command.push_back(0);
    command.push_back(0); command.push_back(0); command.push_back(0);
    command.push_back(0); command.push_back(0); command.push_back(0);
    command.push_back('b');
    command.push_back('t');
    dest = "base";
    return command;
  }

  //! Virtual copy constructor for BT_rev0_Description objects
  inline virtual BT_rev0_Description *clone() const
  { return new BT_rev0_Description(*this); }
};

} // namespace BrailleTutorNS
#endif
