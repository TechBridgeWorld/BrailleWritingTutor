/*
 * Braille Tutor interface library.
 * BT_rev0_StateMachine.cc, started 1 June 2007 by T. Stepleton (tss@ri.cmu.edu)
 *
 * Furnishes a state machine description of the version 0 Braille Tutor
 * hardware. See BT_StateMachines.h for details on these descriptions
 * generally. Refer to BT_rev0_StateMachine.png for a state diagram of
 * this state machine.
 */

#include <vector>
#include <cassert>
#include <sstream>
#include <stdint.h>

#include "StateMachine.h"
#include "BT_StateMachines.h"
#include "BT_rev0_StateMachine.h"

namespace BrailleTutorNS {

//! A BrailleTutor State Action that makes a stylus indication
struct r0_BTSA_makeStylusIndication : public BT_StateAction {
  unsigned short int cell;
  unsigned short int dot;
  inline virtual bool operator()(BT_StateArgs &a)
  {
    std::istringstream in(a.data->str);
    in >> cell >> dot;
    --dot; // Decrement dot for zero indexing
    if((cell == INVALID_CELL) || (dot == INVALID_DOT))
      BTSA_Error("Invalid stylus string: " + a.data->str,
		 BTException::BT_EINVAL)(a);

    a.out->push_back(
      BTSM_Indication::makeStylusIndication(TimeInterval::now(), cell, dot));
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "makeStylusIndication{" << cell << ',' << dot
	      << '}' << std::flush;
#endif
    return false;
  }
  inline ~r0_BTSA_makeStylusIndication() { }

  inline r0_BTSA_makeStylusIndication() : cell(INVALID_CELL),dot(INVALID_DOT) {}

  inline r0_BTSA_makeStylusIndication *clone() const
  { return new r0_BTSA_makeStylusIndication(*this); }
};

//! A BrailleTutor State Action that makes a button indication
struct r0_BTSA_makeButtonIndication : public BT_StateAction {
  unsigned short int button;
  inline virtual bool operator()(BT_StateArgs &a)
  { a.out->push_back(
      BTSM_Indication::makeButtonIndication(TimeInterval::now(), button));
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "makeButtonIndication{" << button << '}' << std::flush;
#endif
    return false; }
  inline ~r0_BTSA_makeButtonIndication() { }

  // Constructor---indicate button
  inline r0_BTSA_makeButtonIndication(const unsigned short int &my_button)
  : button(my_button) { }

  inline r0_BTSA_makeButtonIndication *clone() const
  { return new r0_BTSA_makeButtonIndication(*this); }
};

//! A BrailleTutor State Action that makes an I/O pin indication
struct r0_BTSA_makeIOPinIndication : public BT_StateAction {
  bool pinstate;
  inline virtual bool operator()(BT_StateArgs &a)
  { a.out->push_back(
      BTSM_Indication::makeIOPinInIndication(TimeInterval::now(),0,pinstate));
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "makeIOPinIndication{"
	      << (pinstate ? "high" : "low") << '}' << std::flush;
#endif
    return false; }
  inline ~r0_BTSA_makeIOPinIndication() { }

  // Constructor---indicate pinstate
  inline r0_BTSA_makeIOPinIndication(const bool &my_pinstate)
  : pinstate(my_pinstate) { }

  inline r0_BTSA_makeIOPinIndication *clone() const
  { return new r0_BTSA_makeIOPinIndication(*this); }
};


// Return a state machine description of the revision 0 Braille Tutor
// This state machine description is "reply-driven", meaning that it
// ignores data going from the CPU to the BT and instead listens only to
// the BT's echo and reply bytes. This results in a parsimonious state
// machine, though an ambiguity in the revision 0 reply command set makes
// a particular buzzer command off limits. See BT_rev0_StateMachine.h
// for more details.
BT_StateMachine BT_rev0_Description::makeStateMachine() {
  // The tutor state machine we'll return.
  BT_StateMachine btsm;

  // The following chain of actions is used frequently: it dumps out the
  // the CPU to BT byte stream (which we don't use) and pops the last
  // byte from the BT to CPU byte stream (which we're presumably done with).
  BTSA_Chain finally_chain = BTSA_Chain
			       (BTSA_Pop(BT2CPU))
			       (BTSA_ClearQueue(CPU2BT));

  ////////////////////////
  //// INITIALIZATION ////

  // First the start state: "start"
  BT_ModularState Start("Start",
    BTSA_Switch(BT2CPU).on('b', BTSA_Jump("St1"))
		       .otherwise(BTSA_SelfTrans())
		       .finally(finally_chain)
  );
  btsm.addState(Start);

  // Next the state waiting for the 't' in the "bt" init string
  BT_ModularState St1("St1",
    BTSA_Switch(BT2CPU).on('t', BTSA_Jump("Base"))
		       .otherwise(BTSA_Jump("Start"))
		       .finally(finally_chain)
  );
  btsm.addState(St1);

  ////////////////////
  //// BASE STATE ////

  // The mighty Base state, which dispatches CPU and BT inputs to different
  // processing subroutines (i.e. state clusters). First we have to define
  // actions for the stylus and the button events, since they are triggered
  // by multiple characters. We define a few more besides for the sake of
  // preserving the idiom.
  BTSA_Chain stylus_chain =
    BTSA_Chain
      (BTSA_ClearData())
      (BTSA_StoreStr(BT2CPU))
      (BTSA_Jump("Sy1"));
  BTSA_Chain button_chain =
    BTSA_Chain
      (BTSA_StoreChar(BT2CPU))
      (BTSA_Jump("Bu1"));
  BTSA_Chain buzzer_chain =
    BTSA_Chain
      (BTSA_StoreChar(BT2CPU))
      (BTSA_Jump("Bz1"));
  BTSA_Chain iopin_chain =
    BTSA_Chain
      (BTSA_StoreChar(BT2CPU))
      (BTSA_Jump("IO1"));
  // The state itself
  BT_ModularState Base("Base",
    BTSA_Switch(BT2CPU).on('0', stylus_chain).on('1', stylus_chain)
		       .on('2', stylus_chain).on('3', stylus_chain)
		       .on('4', stylus_chain).on('5', stylus_chain)
		       .on('6', stylus_chain).on('7', stylus_chain)
		       .on('8', stylus_chain).on('9', stylus_chain)
		       .on('a', button_chain).on('b', buzzer_chain)
		       .on('c', button_chain).on('d', button_chain)
		       .on('e', iopin_chain) .on('f', button_chain)
		       .on('g', button_chain)
		       .otherwise(BTSA_Jump("Unk"))
		       .finally(finally_chain)
  );
  btsm.addState(Base);

  // The unknown command state, waiting for the BT to echo the CPU's command
  BT_ModularState Unk("Unk",
    BTSA_Switch(BT2CPU).on('N', BTSA_Jump("Base"))
		       .finally(finally_chain)
  );
  btsm.addState(Unk);

  ////////////////////////
  //// BUZZER CONTROL ////

  // Skip over the frequency byte.
  BT_ModularState Bz1("Bz1",
    BTSA_Chain
      (finally_chain)
      (BTSA_Jump("Bz2"))
  );
  btsm.addState(Bz1);

  // If the duration byte is 'n', this was actually a press of Button 1.
  // Create an indication in that case; otherwise skip along to the 'n'.
  BT_ModularState Bz2("Bz2",
    BTSA_Switch(BT2CPU).on('n', BTSA_Chain
				  (BTSA_Jump("Base"))
				  (r0_BTSA_makeButtonIndication(1)))
		       .otherwise(BTSA_Jump("Bz3"))
		       .finally(finally_chain)
  );
  btsm.addState(Bz2);

  // Check BT's echo of the 'n' byte
  BT_ModularState Bz3("Bz3",
    BTSA_Switch(BT2CPU).on('n', BTSA_Jump("Bz4"))
		       .finally(finally_chain)
  );
  btsm.addState(Bz3);

  // Ignore BT's confirmation byte---just assume it worked
  BT_ModularState Bz4("Bz4",
    BTSA_Chain
      (finally_chain)
      (BTSA_Jump("Base"))
  );
  btsm.addState(Bz4);

  //////////////////////////////
  //// IO PIN CONTROL/QUERY ////

  // Wait for the BT's reply of 'e' or 'i' to the CPU's command
  // If the reply byte is ' ', however, this was actually a press of
  // Button 4. Jump to "Bu2" where an indication can be made.
  BT_ModularState IO1("IO1",
    BTSA_Switch(BT2CPU).on(' ', BTSA_Jump("Bu2"))
		       .on('i', BTSA_Jump("IQ1"))
		       .on('o', BTSA_Jump("IS1"))
		       .finally(finally_chain)
  );
  btsm.addState(IO1);

  // Wait for echo of '1' or '0' CPU byte
  BT_ModularState IS1("IS1",
    BTSA_Switch(BT2CPU).on('1', BTSA_Jump("Base"))
		       .on('0', BTSA_Jump("Base"))
		       .otherwise(BTSA_Jump("IS2"))
		       .finally(finally_chain)
  );
  btsm.addState(IS1);

  // Wait for BT's 'N' reply to incorrect I/O pin set command.
  BT_ModularState IS2("IS2",
    BTSA_Switch(BT2CPU).on('N', BTSA_Jump("Base"))
		       .finally(finally_chain)
  );
  btsm.addState(IS2);

  // Wait for BT's reply of pin high or low to pin query
  BT_ModularState IQ1("IQ1",
    BTSA_Switch(BT2CPU).on('0', r0_BTSA_makeIOPinIndication(false))
		       .on('1', r0_BTSA_makeIOPinIndication(true))
		       .finally(BTSA_Chain
				 (finally_chain)
				 (BTSA_Jump("Base")))
  );
  btsm.addState(IQ1);

  //////////////////////
  //// BUTTON INPUT ////

  // Await space from BT in button input code
  BT_ModularState Bu1("Bu1",
    BTSA_Switch(BT2CPU).on(' ', BTSA_Jump("Bu2"))
		       .finally(finally_chain)
  );
  btsm.addState(Bu1);

  // Await 'n' from BT in button input code, then transcribe button input
  BT_ModularState Bu2("Bu2",
    BTSA_Switch(BT2CPU).on('n', BTSA_Jump("Base"))
		       .finally(BTSA_Chain
			 (finally_chain)
			 (BTSA_Switch(DATABYTE)
			   .on('a', r0_BTSA_makeButtonIndication(0))
			   .on('b', r0_BTSA_makeButtonIndication(1))
			   .on('c', r0_BTSA_makeButtonIndication(2))
			   .on('d', r0_BTSA_makeButtonIndication(3))
			   .on('e', r0_BTSA_makeButtonIndication(4))
			   .on('f', r0_BTSA_makeButtonIndication(5))
			   .on('g', r0_BTSA_makeButtonIndication(6))))
  );
  btsm.addState(Bu2);

  //////////////////////
  //// STYLUS INPUT ////

  // Continue collecting cell ID number bytes.
  BT_ModularState Sy1("Sy1",
    BTSA_Switch(BT2CPU).on('0', BTSA_SelfTrans()).on('1', BTSA_SelfTrans())
		       .on('2', BTSA_SelfTrans()).on('3', BTSA_SelfTrans())
		       .on('4', BTSA_SelfTrans()).on('5', BTSA_SelfTrans())
		       .on('6', BTSA_SelfTrans()).on('7', BTSA_SelfTrans())
		       .on('8', BTSA_SelfTrans()).on('9', BTSA_SelfTrans())
		       .on(' ', BTSA_Jump("Sy2"))
		       .finally(BTSA_Chain
				  (BTSA_StoreStr(BT2CPU))
				  (finally_chain))
  );
  btsm.addState(Sy1);

  // Now collect pin number bytes
  BT_ModularState Sy2("Sy2",
    BTSA_Switch(BT2CPU).on('0', BTSA_SelfTrans()).on('1', BTSA_SelfTrans())
		       .on('2', BTSA_SelfTrans()).on('3', BTSA_SelfTrans())
		       .on('4', BTSA_SelfTrans()).on('5', BTSA_SelfTrans())
		       .on('6', BTSA_SelfTrans()).on('7', BTSA_SelfTrans())
		       .on('8', BTSA_SelfTrans()).on('9', BTSA_SelfTrans())
		       .on(' ', BTSA_Jump("Sy3"))
		       .finally(BTSA_Chain
				  (BTSA_StoreStr(BT2CPU))
				  (finally_chain))
  );
  btsm.addState(Sy2);

  // Finally, wait for the 'n' indicating completion of stylus input
  BT_ModularState Sy3("Sy3",
    BTSA_Switch(BT2CPU).on('n', BTSA_Jump("Base"))
		       .finally(BTSA_Chain
				 (r0_BTSA_makeStylusIndication())
				 (finally_chain))
  );
  btsm.addState(Sy3);

  // And the very last thing---return the constructed tutor
  return btsm;
}

} // namespace BrailleTutorNS
