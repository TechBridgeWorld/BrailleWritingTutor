#ifndef _LIBBT_BT_STATE_MACHINES_H_
#define _LIBBT_BT_STATE_MACHINES_H_
/*
 * BrailleTutor interface library
 * BT_StateMachines.h, started 1 June 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * The behavior of communication with a BrailleTutor device is tracked at a
 * low-level by a state machine (implemented using the StateMachine class).
 * These machines, which are coded specifically for individual Braille Tutor
 * ROM versions, generate tokens describing Tutor behavior from the underlying
 * bytes of the Tutor protocol (similar to lexing), which is monitored by
 * analyzing the bytes going to and from the tutor. This file provides a
 * few classes that represent standard actions for this sort of thing, such
 * as waiting on a character from the Tutor or the CPU. There is also a class
 * that maps abstract commands from the computer (e.g. buzzer commands)
 * to byte sequences for a particular BT rom version.
 *
 * Note that the internal representation of state machine names is
 * std::strings---this is perhaps not as fast as other approaches, but it
 * does make for lear error messages.
 *
 * See BT_rev0_StateMachine.h for examples of how to implement a state
 * machine for a particular BT device.
 */

#include <map>
#include <deque>
#include <string>
#include <sstream>

#include <stdint.h>

#include "Types.h"
#include "StateMachine.h"

#include "boost/shared_ptr.hpp"

namespace BrailleTutorNS {

////////////////////////////////////////
//// STATE MACHINE CLASS COMPONENTS ////
////////////////////////////////////////

//! Input symbol type for BrailleTutor state machines

//! Input "symbol" type is actually a structure containing two queues of
//! bytes---one representing the bytes going from the computer to the BT,
//! the other representing the stream from the BT to the computer. This
//! allows the state machine to handle asynchronous communication between
//! both systems---it can decide which stream to handle as appropriate.
//! The state machine will remove bytes from the queues as they are
//! processed---this is why input arguments to state machines aren't const!
struct BTSM_inputT {
  std::deque<uint8_t> cpu_to_bt;
  std::deque<uint8_t> bt_to_cpu;
  typedef std::deque<uint8_t>::iterator iterT;
  typedef std::deque<uint8_t>::const_iterator constIterT;
};

//! Output symbol element type for BrailleTutor state machines

//! Just as the BT state machine input "symbol" type is actually a couple of
//! queues, the output "symbol" is a queue... of these things. These structures
//! indicate some kind of input event is happening---the stylus is down now,
//! for instance---and so they repeat at the rate of the corresponding
//! messages from the BrailleTutor. More advanced routines will check
//! timestamps of these repeated indications and determine when stylus or
//! button events start and stop.
struct BTSM_Indication {
  //! Indication type symbols
  typedef enum { STYLUS,	//!< Stylus is currently inserted
		 BUTTON,	//!< Button is currently pushed
		 IOPIN_IN,	//!< High or low indication from an I/O pin
		 DONE		//!< BrailleTutor object is destructing
	} Type;

  //! Type of this indication
  Type type;

  // Anonymous union promotes space saving for heterogeneous event types
  union {
    unsigned short int cell;	//!< Cell in which the stylus event occurred.
    unsigned short int button;	//!< Button emitting the button event.
    unsigned short int iopin;	//!< I/O pin returning status query info.
  };

  // Another anonymous union
  union {
    unsigned char dot;	//!< Cell dot in which the stylus event occurred.
    bool pinstate;	//!< Whether the I/O pin is high or low.
  };

  TimeInterval timestamp;	//!< Timestamp of the event

private:
  //! Private constructor

  //! This constructor is private because setting BTSM_Indication values
  //! directly could yield inconsistent representations, such as button
  //! events with dot != INVALID_DOT. Use the named constructors instead.
  inline BTSM_Indication(const TimeInterval &my_timestamp,
			 const Type &my_type,
			 const unsigned short int &cell_button_or_iopin,
			 const unsigned char &dot=INVALID_DOT)
  : type(my_type), cell(cell_button_or_iopin), dot(dot),
    timestamp(my_timestamp) { }

  //! Private constructor

  //! A second private constructor is required to properly set the pinstate
  //! variable.
  inline BTSM_Indication(const TimeInterval &my_timestamp,
			 const unsigned short int &my_iopin,
			 const bool &my_pinstate)
  : type(IOPIN_IN), iopin(my_iopin), pinstate(my_pinstate),
    timestamp(my_timestamp) { }

  //! Private constructor

  //! A third private constructor creates DONE indications, needed to
  //! instruct threads to self-terminate.
  inline BTSM_Indication()
  : type(DONE), cell(INVALID_CELL), dot(INVALID_DOT),
    timestamp(TimeInterval::now()) { }

public:
  //! Named constructor for creating STYLUS indications
  inline static BTSM_Indication makeStylusIndication(
				    const TimeInterval &my_timestamp,
				    const unsigned short int &my_cell,
				    const unsigned char &my_dot)
  {
    if(my_dot == INVALID_DOT)
      throw BTException(BTException::BT_EINVAL,
		"BTSM_Indication::makeStylusDownEvent: invalid dot argument");
    return BTSM_Indication(my_timestamp, STYLUS, my_cell, my_dot);
  }

  //! Named constructor for creating BUTTON indications
  inline static BTSM_Indication makeButtonIndication(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_button)
  {
    // Note that we also assign the dot field when the button is within
    // the cell---this way code interested in Braille dots can treat
    // buttons and cell holes the same way.
    if(my_button == 0) return BTSM_Indication(my_timestamp, BUTTON, my_button);
    else return BTSM_Indication(my_timestamp, BUTTON, my_button, my_button-1);
  }

  //! Named constructor for creating IOPIN_IN indications
  inline static BTSM_Indication makeIOPinInIndication(
				const TimeInterval &my_timestamp,
				const unsigned short int &my_iopin,
				const bool &my_pinstate)
  { return BTSM_Indication(my_timestamp, my_iopin, my_pinstate); }

  //! Named constructor for creating DONE indications
  inline static BTSM_Indication makeDoneIndication()
  { return BTSM_Indication(); }

  //! Returns true if the argument indication is "the same" as this one

  //! Returns true if the argument event has the same type and type parameters
  //! as this event. The time is allowed to vary.
  inline bool sameIndication(const BTSM_Indication &i) const
  {
    if(type != i.type) return false;

    if(type == STYLUS)
      return (cell == i.cell) && (dot == i.dot);
    else if(type == BUTTON)
      return button == i.button;
    else if(type == IOPIN_IN)
      return (iopin == i.iopin) && (pinstate == i.pinstate);

    // Should never happen
    return false;
  }
};

//! Output type for BrailleTutor state machines

//! Just as the BT state machine input type is a pair of queues, the output
//! type is a queue of indications from the board. Indications say that
//! some kind of event is happening---the stylus is inserted, for instance.
//! The BrailleTutor object will process the contents of this output queue
//! as it has time.
typedef std::deque<BTSM_Indication> BTSM_outputT;

//! Data type for BrailleTutor state machines

//! Stores temporary information while the state machine parses input from
//! the BT and the CPU. Consists of a byte for waiting on just bytes and
//! a std::string to store characters for later parsing---e.g. to determine
//! from the BT input the cell or stylus hole into which a stylus is inserted.
struct BTSM_dataT {
  uint8_t byte;
  std::string str;
  inline BTSM_dataT() : byte(0) { }
};

//! Name type for BrailleTutor state machines---just strings
typedef std::string BTSM_stateNameT;

//! State abstract base type for BrailleTutor state machines.
typedef SMState<BTSM_inputT, BTSM_outputT,
		BTSM_dataT, BTSM_stateNameT> BT_State;
//! State machine abstract base type for BrailleTutor state machines.
typedef StateMachine<BTSM_inputT, BTSM_outputT,
		     BTSM_dataT, BTSM_stateNameT> BT_StateMachine;



///////////////////////////////////
//// BRAILLE TUTOR DESCRIPTION ////
///////////////////////////////////

//! Describes behavior and commands of a Braille Tutor

//! Classes derived from this abstract base class can create a state machine
//! describing a particular model of Braille Tutor, and also furnish
//! strings of uint8_t bytes that correspond to different commands
//! (presently: beep the speaker and set/get the I/O pin status).
struct BT_Description {
  //! Return a state machine description of a particular Braille Tutor
  virtual BT_StateMachine makeStateMachine() = 0;
  //! Return bytes for making the Tutor generate a beep noise
  virtual std::deque<uint8_t> makeBeepBytes(const double &freq,
					    const double &len) = 0;
  //! Return bytes for retrieving the I/O pin status
  virtual std::deque<uint8_t> makeGetIOPinBytes(const unsigned int &pin) = 0;
  //! Return bytes for setting the I/O pin status
  virtual std::deque<uint8_t> makeSetIOPinBytes(const unsigned int &pin,
						const bool &pinstate) = 0;
  //! Return bytes for putting BT into interactive mode from autodetect mode
  virtual std::deque<uint8_t> makeInteractiveModeBytes() = 0;

  //! "Magic whammy" bytes to send to the BT for reset.

  //! Returns bytes that will result in the BT assuming a knowable state.
  //! The name of this state is stored in the argument string.
  virtual std::deque<uint8_t> makeResetBytes(BTSM_stateNameT &dest) = 0;

  //! Virtual copy constructor for BT_Description objects
  virtual BT_Description *clone() const = 0;

  //! For G++
  inline virtual ~BT_Description() { }
};



////////////////////////////////
//// STATE CONSTRUCTION KIT ////
////////////////////////////////
// Containing various bits and pieces for easy construction of BT state
// machines.

//! A struct with pointers to all of the arguments to BT_State::operator()

//! Contains pointers to all of the arguments to BT_State::operator as well
//! as a pointer to the current state (allowing you to call getName()).
struct BT_StateArgs {
  BTSM_inputT		*in;
  BTSM_outputT		*out;
  BTSM_dataT		*data;
  BTSM_stateNameT	*dest;
  const BT_State	*node;
  inline BT_StateArgs(BTSM_inputT &my_in, BTSM_outputT &my_out,
    BTSM_dataT &my_data, BTSM_stateNameT &my_dest, const BT_State *my_node)
  : in(&my_in), out(&my_out), data(&my_data), dest(&my_dest), node(my_node) { }
};

//! A functor that modifies data structures pointed to by BT_StateArgs vars.

//! These functors essentially do whatever is done in the operator() methods
//! of BT_State objects; however, combined with the the architecture of
//! BT_ModularState, it allows us to construct BT_State objects from
//! building blocks made from BT_StateAction objects.
struct BT_StateAction {
  //! The method that performs the action on the input data.

  //! This method actually performs the BT_StateAction's action on the
  //! input data. The bool return value, if true, will cause a BTSA_Chain
  //! to stop going through the rest of the chain. This is useful for
  //! BT_StateAction objects that command some action in response to data
  //! waiting in an input queue. Some examples:
  //! (1) BTSA_StoreChar, BTSA_Pop will command a self-jump if their
  //!     designated input queue is empty---maybe by the next time around
  //!     data will be waiting there. Since these actions are often used
  //!     in tandom within a BTSA_Chain, there's no sense in doing the
  //!     second if you can't do the first. Returning true aborts the chain.
  //! (2) Some states can call for one set of actions if data is waiting on
  //!     one queue and another set if data is waiting on the other queue.
  //!     This is usually implemented by a pair of BTSA_TestEqual or
  //!     BTSA_Switch actions within the same BTSA_Chain. If data is waiting
  //!     in both queues, then only one of the actions should be exectued.
  //!     lest the other's outcome be "overwritten" and ignored. For this
  //!     reason, both BTSA_TestEqual and BTSA_Switch return true when data
  //!     is present allowing the test to take place.
  virtual bool operator()(BT_StateArgs&) = 0;

  inline virtual ~BT_StateAction() { }

  //! Virtual copy constructor for BT_StateAction objects
  virtual BT_StateAction *clone() const = 0;

  inline BT_StateAction() { }
};

//! Indicates location of data byte for some of thes BT_StateAction objects
typedef enum { CPU2BT,		//!< To/from the cpu_to_bt input queue
	       BT2CPU,		//!< To/from the bt_to_cpu input queue
	       DATABYTE,	//!< To/from BTSM_dataT::byte
	       DATASTR		//!< To/from BTSM_dataT::str
      } BTSA_bytesource;

#ifdef LIBBT_SM_DIAG_PRINT
//! Utility function asociates a string name with a BTSA_bytesource value
inline std::string bs_name(const BTSA_bytesource &bs)
{
  switch(bs) {
  case CPU2BT:		return std::string("CPU2BT");
  case BT2CPU:		return std::string("BT2CPU");
  case DATABYTE:	return std::string("DATABYTE");
  case DATASTR:		return std::string("DATASTR");
  default:
  { std::ostringstream name; name << "**UNKNOWN: " << bs << "**";
    return name.str(); }
  }
}
#endif

//! A BT_StateAction that prints a message to stderr. For debugging.
struct BTSA_Message : public BT_StateAction {
  std::string message;
  inline virtual bool operator()(BT_StateArgs &a) {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "Message{\"" << message << "\"}" << std::endl;
#else
    std::cerr << "STATE " << a.node->getName() << ": " << message << std::endl;
#endif
    return false;
  }
  inline BTSA_Message(const std::string &my_message) : message(my_message) { }
  inline virtual ~BTSA_Message() { }
  inline virtual BTSA_Message *clone() const { return new BTSA_Message(*this); }
};

//! A NOOP BT_StateAction
struct BTSA_NoOp : public BT_StateAction {
  inline virtual bool operator()(BT_StateArgs&) {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "NoOp{}" << std::flush;
#endif
    return false;
  }
  inline virtual ~BTSA_NoOp() { }
  inline virtual BTSA_NoOp *clone() const { return new BTSA_NoOp(*this); }
};

//! A BT_StateAction that calls for a jump to another state
struct BTSA_Jump : public BT_StateAction {
  BTSM_stateNameT dest;		//!< Destination of jump
  inline virtual bool operator()(BT_StateArgs &a) {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "Jump{" << dest << '}' << std::flush;
#endif
    *(a.dest) = dest;
    return false;
  }
  inline BTSA_Jump(const BTSM_stateNameT &my_dest) : dest(my_dest) { }
  inline virtual ~BTSA_Jump() { }
  inline virtual BTSA_Jump *clone() const { return new BTSA_Jump(*this); }
};

//! A BT_StateAction that calls for a self-transition
struct BTSA_SelfTrans : public BT_StateAction {
  inline virtual bool operator()(BT_StateArgs &a) {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "SelfTrans{}" << std::flush;
#endif
    *(a.dest)=a.node->getName();
    return false;
  }
  inline virtual ~BTSA_SelfTrans() { }
  inline virtual BTSA_SelfTrans *clone() const
  { return new BTSA_SelfTrans(*this); }
};

//! A BT_StateAction that executes a chain of BT_StateActions

//! Note that if one of the BT_StateAction objects in the chain returns true,
//! this object will cease processing the rest of the chain and return true
//! itself. Thus aborted chains will themselves terminate enclosing chains.
struct BTSA_Chain : public BT_StateAction {
  //! Actions chain
  std::deque<boost::shared_ptr<BT_StateAction> > actions;

  inline virtual bool operator()(BT_StateArgs &a)
  {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "Chain{" << std::flush;
#endif
    std::deque<boost::shared_ptr<BT_StateAction> >::iterator i;
    for(i=actions.begin(); i!=actions.end(); ++i) {
      if( (*(*i))(a) ) { // FUN syntax!
#ifdef LIBBT_SM_DIAG_PRINT
	std::cerr << ", *STOP*}" << std::flush;
#endif
	return true;
      }
#ifdef LIBBT_SM_DIAG_PRINT
	std::cerr << ", " << std::flush;
#endif
    }
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "END}" << std::flush;
#endif
    return false;
  }

  inline virtual ~BTSA_Chain() { }

  //! Appends actions to the actions chain
  inline BTSA_Chain &operator()(const BT_StateAction &act)
  { actions.push_back(boost::shared_ptr<BT_StateAction>(act.clone()));
    return *this; }

  //! Constructor requires specification of the chain's initial action
  inline BTSA_Chain(const BT_StateAction &act)
  { (*this)(act); }

  //! Assignment operator
  inline BTSA_Chain &operator=(const BTSA_Chain &b)
  {
    if(&b == this) return *this;
    actions.clear();
    std::deque<boost::shared_ptr<BT_StateAction> >::const_iterator i;
    for(i=b.actions.begin(); i!=b.actions.end(); ++i) (*this)(**i);
    return *this;
  }

  //! Copy constructor
  inline BTSA_Chain(const BTSA_Chain &b) { *this = b; }

  inline virtual BTSA_Chain *clone() const { return new BTSA_Chain(*this); }
};

//! A BT_StateAction that throws a BTException with much diagnostic info
struct BTSA_Error : public BT_StateAction {
  std::string info;		//!< User-furnished info string
  BTException::Type type;	//!< BTException type
  inline virtual bool operator()(BT_StateArgs &a) {
    std::ostringstream out;
    out << "State machine error: " << info << std::endl
	<< "  CURRENT STATE:  " << a.node->getName() << std::endl
	<< "  CPU input chars:";
    BTSM_inputT::constIterT i;
    for(i=a.in->cpu_to_bt.begin(); i!=a.in->cpu_to_bt.end(); ++i) dumpc(out,*i);
    out << std::endl << "  BT input chars: ";
    for(i=a.in->bt_to_cpu.begin(); i!=a.in->bt_to_cpu.end(); ++i) dumpc(out,*i);
    out << std::endl << "  Data character: "; dumpc(out,a.data->byte);
    out << std::endl << "  Data string:    " << a.data->str << std::endl;
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "ERROR{\"" << std::endl << out.str() << "\"}" << std::endl;
#endif
    throw BTException(type,out.str());
    return false; /* won't get here */
  }
  inline virtual ~BTSA_Error() { }

  //! Constructor takes in an informational string and optional exception type.
  inline BTSA_Error(const std::string &my_info, 
		    const BTException::Type &my_type)
  : info(my_info), type(my_type) { }

  //! Utility for dumping character values to output
  inline void dumpc(std::ostream &out, const uint8_t &c)
  { out << ' ';
    if((c <= 33) && (c >= 126)) out << '(' << (char) c << ')';
    else out << (char) c; }

  inline virtual BTSA_Error *clone() const { return new BTSA_Error(*this); }
};

//! A BT_StateAction that clears the data store
struct BTSA_ClearData : public BT_StateAction {
  inline virtual bool operator()(BT_StateArgs &a) {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "ClearData{}" << std::flush;
#endif
    a.data->str.clear();
    return false;
  }
  inline virtual ~BTSA_ClearData() { }
  inline virtual BTSA_ClearData *clone() const
  { return new BTSA_ClearData(*this); }
};

//! A BT_StateAction that clears one of the several input queues
struct BTSA_ClearQueue : public BT_StateAction {
  BTSA_bytesource from;
  inline virtual bool operator()(BT_StateArgs &a)
  {
    // Only input sources are supported---i.e. nothing in the data store
    if(from==CPU2BT)		a.in->cpu_to_bt.clear();
    else if(from==BT2CPU) 	a.in->bt_to_cpu.clear();
    else BTSA_Error("invalid queue in BTSA_ClearQueue",
		    BTException::BT_EINVAL)(a);

#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "ClearQueue(" << bs_name(from) << "){}" << std::flush;
#endif
    return false;
  }
  inline virtual ~BTSA_ClearQueue() { }

  //! Constructor takes a queue to clear
  inline BTSA_ClearQueue(const BTSA_bytesource &my_from) : from(my_from) { }

  inline virtual BTSA_ClearQueue *clone() const
  { return new BTSA_ClearQueue(*this); }
};

// This block of code contains the thing we should do if one of these
// character storage actions doesn't have data to store yet. After commanding
// a self-transiton for another shot at the character, it returns true so that
// any block waiting on the results of this action is stopped, i.e. the
// self-transition happens immediately.
#ifdef LIBBT_SM_DIAG_PRINT
#define _ACT_STORE_POP_NODATA \
{ \
  std::cerr << "*NO DATA, SELF TRANSITION*}" << std::flush; \
  *(a.dest)=a.node->getName(); \
  return true; \
}
#else
#define _ACT_STORE_POP_NODATA { *(a.dest)=a.node->getName(); return true; }
#endif

//! A BT_StateAction that stores a character into the data store 'byte' field

//! Stores a character from one of the input queues into the data store 'byte'
//! field. If no such character exists, forces a loop back to the current state.
//! See note in BT_StateAction.
struct BTSA_StoreChar : public BT_StateAction {
  BTSA_bytesource from;
  inline virtual bool operator()(BT_StateArgs &a)
  {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "StoreChar(" << bs_name(from) << "){";
#endif
    // Only input sources are supported---i.e. nothing in the data store
    if(from==CPU2BT) {
      if(a.in->cpu_to_bt.empty()) _ACT_STORE_POP_NODATA
      else a.data->byte = a.in->cpu_to_bt.front();
    }
    else if(from==BT2CPU) {
      if(a.in->bt_to_cpu.empty()) _ACT_STORE_POP_NODATA
      else a.data->byte = a.in->bt_to_cpu.front();
    }
    else BTSA_Error("invalid store destination for BTSA_StoreChar",
		    BTException::BT_EINVAL)(a);

#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << a.data->byte << '}' << std::flush;
#endif
    return false;
  }
  inline virtual ~BTSA_StoreChar() { }

  //! Constructor takes a source for the stored character.
  inline BTSA_StoreChar(const BTSA_bytesource &my_from) : from(my_from) { }

  inline virtual BTSA_StoreChar *clone() const
  { return new BTSA_StoreChar(*this); }
};

//! A BT_StateAction that appends a character to the data store 'str' field

//! Appends a character from one of the input queues to the data store 'str'
//! field. If no such character exists, forces a loop back to the current state.
//! See note in BT_StateAction.
struct BTSA_StoreStr : public BT_StateAction {
  BTSA_bytesource from;
  inline virtual bool operator()(BT_StateArgs &a)
  {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "StoreStr(" << bs_name(from) << "){";
#endif
    // Only input sources are supported---i.e. nothing in the data store
    if(from==CPU2BT) {
      if(a.in->cpu_to_bt.empty()) _ACT_STORE_POP_NODATA
      else a.data->str.push_back(a.in->cpu_to_bt.front());
    }
    else if(from==BT2CPU) {
      if(a.in->bt_to_cpu.empty()) _ACT_STORE_POP_NODATA
      else a.data->str.push_back(a.in->bt_to_cpu.front());
    }
    else BTSA_Error("invalid store destination for BTSA_StoreStr",
		    BTException::BT_EINVAL)(a);

#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << a.data->str[a.data->str.length()-1] << '}' << std::flush;
#endif
    return false;
  }
  inline virtual ~BTSA_StoreStr() { }

  //! Constructor takes a source for the stored character.
  inline BTSA_StoreStr(const BTSA_bytesource &my_from) : from(my_from) { }

  inline virtual BTSA_StoreStr *clone() const
  { return new BTSA_StoreStr(*this); }
};

//! A BT_StateAction that pops a character from an input queue

//! Pops a character from one of the input queues.
//! See note in BT_StateAction.
struct BTSA_Pop : public BT_StateAction {
  BTSA_bytesource from;
  inline virtual bool operator()(BT_StateArgs &a)
  {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "Pop(" << bs_name(from) << "){";
#endif
    // Only input sources are supported---i.e. nothing in the data store
    if(from==CPU2BT) {
      if(a.in->cpu_to_bt.empty()) _ACT_STORE_POP_NODATA
      else a.in->cpu_to_bt.pop_front();
    }
    else if(from==BT2CPU) {
      if(a.in->bt_to_cpu.empty()) _ACT_STORE_POP_NODATA
      else a.in->bt_to_cpu.pop_front();
    }
    else BTSA_Error("input queue not specified in BTSA_Pop",
                    BTException::BT_EINVAL)(a);

#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << '}' << std::flush;
#endif
    return false;
  }
  inline virtual ~BTSA_Pop() { }

  //! Constructor takes a source for the stored character.
  inline BTSA_Pop(const BTSA_bytesource &my_from) : from(my_from) { }

  inline virtual BTSA_Pop *clone() const { return new BTSA_Pop(*this); }
};

// This block of code contains the thing we should do if one of these
// testing actions doesn't have data to test yet. After commanding a
// self-transiton for another shot at the chosen input queue, it returns false
// so that any subsequent test waiting on a different queue, or any other
// action, will still happen, possibly overwriting the self transition.
#ifdef LIBBT_SM_DIAG_PRINT
#define _ACT_TEST_SWITCH_NODATA \
{ \
  std::cerr << "*NO DATA, SELF TRANSITION*}" << std::flush; \
  *(a.dest)=a.node->getName(); \
  return false; \
}
#else
#define _ACT_TEST_SWITCH_NODATA { *(a.dest)=a.node->getName(); return false; }
#endif

//! A BT_StateAction that compares characters for equality

//! Compares a character from the head of one of the input queues with the
//! character in the data store's byte field. Executes one action if the
//! characters are equal and another if not (default is to throw an exception).
//! See note in BT_StateAction.
struct BTSA_TestEqual : public BT_StateAction {
  BTSA_bytesource from;
  boost::shared_ptr<BT_StateAction> success, failure, lastitem;
  inline virtual bool operator()(BT_StateArgs &a)
  {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "TestEqual(" << bs_name(from) << "){";
#endif
    uint8_t comparand;
    // Only input sources are supported---i.e. nothing in the data store
    if(from==CPU2BT) {
      if(a.in->cpu_to_bt.empty()) _ACT_TEST_SWITCH_NODATA
      else comparand = a.in->cpu_to_bt.front();
    }
    else if(from==BT2CPU) {
      if(a.in->bt_to_cpu.empty()) _ACT_TEST_SWITCH_NODATA
      else comparand = a.in->bt_to_cpu.front();
    }
    else BTSA_Error("input queue not specified in BTSA_Pop",
                    BTException::BT_EINVAL)(a);

#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << comparand << "==" << a.data->byte << "? " << std::flush;
#endif
    if(comparand == a.data->byte) (*success)(a); else (*failure)(a);
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << ", " << std::flush;
#endif
    (*lastitem)(a);
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << '}' << std::flush;
#endif
    return true;
  }

  //! Specify the consequent of passing the equality test
  inline BTSA_TestEqual &then(const BT_StateAction &act)
  { success.reset(act.clone()); return *this; }

  //! Specify the consequent of failing the equality test
  inline BTSA_TestEqual &otherwise(const BT_StateAction &act)
  { failure.reset(act.clone()); return *this; }

  //! Specify an action to always do after testing
  inline BTSA_TestEqual &finally(const BT_StateAction &act)
  { lastitem.reset(act.clone()); return *this; }

  //! Constructor indicates consequence of an affirmative match.
  inline BTSA_TestEqual(const BTSA_bytesource &my_from,
			const BT_StateAction &my_success)
  : from(my_from)
  {
    then(my_success);
    otherwise(BTSA_Error("character match failure in BTSA_TestEqual",
	      		 BTException::BT_EINVAL)),
    finally(BTSA_NoOp());
  }

  //! Assignment operator
  inline BTSA_TestEqual &operator=(const BTSA_TestEqual &b)
  {
    if(&b == this) return *this;
    from = b.from;
    then(*b.success);
    otherwise(*b.failure);
    finally(*b.lastitem);
    return *this;
  }

  //! Copy constructor
  inline BTSA_TestEqual(const BTSA_TestEqual &b) { *this = b; }

  inline virtual BTSA_TestEqual *clone() const
  { return new BTSA_TestEqual(*this); }
};

//! A BT_StateAction that switches on an input queue character.

//! Performs configurable actions depending on what character is at the
//! head of an input queue or in the byte field of the data store..
//! See note in BT_StateAction.
struct BTSA_Switch : public BT_StateAction {
  BTSA_bytesource from;
  std::map<uint8_t, boost::shared_ptr<BT_StateAction> > actions;
  boost::shared_ptr<BT_StateAction> failure, lastitem;
  inline virtual bool operator()(BT_StateArgs &a)
  {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << "Switch(" << bs_name(from) << "){";
#endif
    uint8_t switcher = 0;
    // Pulling from the str field of the data store is not supported
    if(from==CPU2BT) {
      if(a.in->cpu_to_bt.empty()) _ACT_TEST_SWITCH_NODATA
      else switcher = a.in->cpu_to_bt.front();
    }
    else if(from==BT2CPU) {
      if(a.in->bt_to_cpu.empty()) _ACT_TEST_SWITCH_NODATA
      else switcher = a.in->bt_to_cpu.front();
    }
    else if(from==DATABYTE) {
      switcher = a.data->byte;
    }
    else BTSA_Error("invalid byte source specified in BTSA_Switch",
                    BTException::BT_EINVAL)(a);
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << switcher << "? " << std::flush;
#endif

    std::map<uint8_t, boost::shared_ptr<BT_StateAction> >::iterator i;
    i = actions.find(switcher);
    if(i == actions.end()) {
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << " OTHERWISE: " << std::flush;
#endif
      (*failure)(a);
    }
    else {
     (*(i->second))(a);
    }
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << ", " << std::flush;
#endif
    (*lastitem)(a);
#ifdef LIBBT_SM_DIAG_PRINT
    std::cerr << '}' << std::flush;
#endif

    return true;
  }
  inline virtual ~BTSA_Switch() { }

  //! Specify what to do on one match
  inline BTSA_Switch &on(const uint8_t &what, const BT_StateAction &action)
  { actions[what] = boost::shared_ptr<BT_StateAction>(action.clone());
    return *this; }

  //! Specify the consequent of no match
  inline BTSA_Switch &otherwise(const BT_StateAction &act)
  { failure.reset(act.clone()); return *this; }

  //! Specify an action to always do after matching
  inline BTSA_Switch &finally(const BT_StateAction &act)
  { lastitem.reset(act.clone()); return *this; }

  //! Constructor takes the source of the character to switch on.
  inline BTSA_Switch(const BTSA_bytesource &my_from)
  : from(my_from)
  {
    otherwise(BTSA_Error("character match failure in BTSA_Switch",
			 BTException::BT_EINVAL));
    finally(BTSA_NoOp());
  }

  //! Assignment operator
  inline BTSA_Switch &operator=(const BTSA_Switch &b)
  {
    if(&b == this) return *this;
    from = b.from;
    actions.clear();
    std::map<uint8_t, boost::shared_ptr<BT_StateAction> >::const_iterator i;
    for(i=b.actions.begin(); i!=b.actions.end(); ++i)
      on(i->first, *(i->second));
    otherwise(*b.failure);
    finally(*b.lastitem);
    return *this;
  }

  //! Copy constructor
  inline BTSA_Switch(const BTSA_Switch &b) { *this = b; }

  inline virtual BTSA_Switch *clone() const { return new BTSA_Switch(*this); }
};

//! Canonical state for waiting on a character

//! A configurable BrailleTutor state that waits for input from the BT or
//! the CPU to match the byte in BTSM_dataT. Depending on initialization,
//! it may throw a BTException on seeing an unexpected character or
struct BT_ModularState : public BT_State {
  //! Name of this state
  BTSM_stateNameT name;
  //! What this state does for operator()
  boost::shared_ptr<BT_StateAction> action;

  //! Retrieve the name of this state
  inline virtual const BTSM_stateNameT getName() const { return name; }

  inline virtual void operator()(BTSM_inputT &in, BTSM_outputT &out,
				 BTSM_dataT &data, BTSM_stateNameT &dest)
  { BT_StateArgs args(in, out, data, dest, this); (*action)(args); }

  inline virtual ~BT_ModularState() { }

  //! Constructor--takes in a name and an action
  inline BT_ModularState(const BTSM_stateNameT &my_name,
			 const BT_StateAction &my_action)
  : name(my_name),
    action(boost::shared_ptr<BT_StateAction>(my_action.clone())) { }

  //! Assignment operator
  inline BT_ModularState &operator=(const BT_ModularState &b)
  {
    if(&b == this) return *this;
    name = b.name;
    action.reset(b.action->clone());
    return *this;
  }

  //! Copy constructor
  inline BT_ModularState(const BT_ModularState &b) { *this = b; }

  inline BT_ModularState *clone() const { return new BT_ModularState(*this); }
};

} // namespace BrailleTutorNS

#endif
