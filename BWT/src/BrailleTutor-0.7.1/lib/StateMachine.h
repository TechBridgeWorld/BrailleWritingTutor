#ifndef _LIBBT_STATE_MACHINE_H_
#define _LIBBT_STATE_MACHINE_H_
/*
 * Braille Tutor interface library
 * StateMachine.h, started 5/28/2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Templated classes for state machines---but not just traditional FSMs,
 * since the machines may also incorporate certain data structures for
 * maintaining state information, and since the decision about how to
 * transition away from a state for any input may be derived from arbitrary
 * code. These classes are not for theoretical purposes but for convenience
 * when modeling a process that is easy to draw with a state diagram.
 */

#include <map>
#include <deque>
#include <iterator>
#include <iostream>

#include "Types.h"

#include "boost/shared_ptr.hpp"

namespace BrailleTutorNS {

//! Templated abstract base class for a state machine state.

//! An abstract base class for an individual automaton state. The template
//! arguments are as follows:
//!   - inputT:		type for the input alphabet
//!   - outputT:	type for the output alphabet
//!   - dataT:		datatype for automaton's "extra" internal state
//!   - stateNameT:	type for representing the names of states
template <typename inputT, typename outputT,
	  typename dataT,  typename stateNameT>
struct SMState {
  //! Returns the name of this state
  virtual const stateNameT getName() const = 0;

  //! React to a particular input

  //! Given the input symbol in, emit an optional output symbol in out,
  //! alter the "extra" internal state in data, and indicate the destination
  //! of the next state in dest.
  virtual void operator()(inputT &in, outputT &out,
			  dataT &data, stateNameT &dest) = 0;

  //! Virtual copy constructor for SMState objects
  virtual SMState<inputT, outputT, dataT, stateNameT> *clone() const = 0;

  //! For G++
  inline virtual ~SMState() { }
};


//! Templated class for state machine processing

//! Stores state information and manages execution of a state machine.
//! The template arguments are as follows:
//!   - inputT:		type for the input alphabet
//!   - outputT:	type for the output alphabet
//!   - dataT:		datatype for automaton's "extra" internal state
//!   - stateNameT:	type for representing the names of states
template <typename inputT, typename outputT,
	  typename dataT,  typename stateNameT>
struct StateMachine {
private:
  //! Shorthand for the type of states used by this state machine
  typedef SMState<inputT, outputT, dataT, stateNameT> stateT;

  //! Collection of states in this automaton
  std::map<stateNameT, boost::shared_ptr<stateT> > states;
  typedef typename std::map<stateNameT,
			    boost::shared_ptr<stateT> >::iterator states_iter;
  typedef typename std::map<stateNameT,
		boost::shared_ptr<stateT> >::const_iterator states_const_iter;

  //! Extra internal state for the automaton

  //! This variable is always passed to whichever state is the active state
  //! when that state's operator() method is called. Depending on what it
  //! actually is, the automaton can use information stored inside to implement
  //! a classic Turing machine, a pushdown automaton, etc.
  dataT data;

  //! Pointer to the current state
  stateT *curr_state;

public:
  //! Submit a single input to the state machine and cycle the clock.
  inline void cycle(inputT &in, outputT &out)
  {
    stateNameT new_state_name;
#ifdef LIBBT_SM_DIAG_PRINT
std::cerr << '[' << curr_state->getName() << ' ' << std::flush;
#endif
    (*curr_state)(in, out, data, new_state_name);
    states_iter s_iter = states.find(new_state_name);
    if(s_iter == states.end()) {
#ifdef LIBBT_SM_DIAG_PRINT
std::cerr << " => NONEXISTANT STATE \"" << new_state_name << "\"]" << std::endl;
#endif
      throw BTException(BTException::BT_EMISC,
			"state machine tried to jump to a nonexistant state");
    }
    curr_state = s_iter->second.get();
#ifdef LIBBT_SM_DIAG_PRINT
std::cerr << " => " << curr_state->getName() << "] "
	  << (double) TimeInterval::now() << std::endl;
#endif
  }

  //! An alias for cycle
  inline void operator()(inputT &in, outputT &out) { cycle(in, out); }

  //! Retrieve the name of the current state
  inline void getCurrStateName(stateNameT &csn) const
  { csn = curr_state->getName(); }

  //! Retrieve a list of all of the state names
  template<typename OutputIterator>
  void getStateNames(OutputIterator out)
  {
    states_const_iter s_iter;
    for(s_iter=states.begin(); s_iter!=states.end(); ++s_iter)
      *out++ = s_iter->first;
  }

  //! Set the current state of the state machine
  void setState(const stateNameT &state_name)
  {
    states_iter s_iter = states.find(state_name);
    if(s_iter == states.end())
      throw BTException(BTException::BT_EMISC,
			"state machine tried to jump to a nonexistant state");
    curr_state = s_iter->second.get();
  }

  //! Retrieve one of the state machine's states
  const stateT &getState(const stateNameT &state_name) const
  {
    states_const_iter s_iter = states.find(state_name);
    if(s_iter == states.end())
      throw BTException(BTException::BT_EINVAL,
			std::string("state ")+state_name+" does not exist.");
    return *(s_iter->second);
  }

  //! Add a state to the state machine.

  //! Adds a state to the state machine. If there's no current state, the
  //! current state is set to this state; thus, usually the first state
  //! added to the system is the current state.
  inline void addState(const stateT &state)
  { states[state.getName()] = boost::shared_ptr<stateT>(state.clone());
    if(curr_state == NULL) setState(state.getName()); }

  //! Retrieve the data object for this automaton
  inline dataT &getData() { return data; }

  //! Constructor

  //! Constructs a state machine from a vector of smart pointers to states;
  //! with no argument, constructs an empty state machine (NB: do not use
  //! unless you want a null pointer dereference---add states with addState
  //! first).
  inline StateMachine(
    const std::vector<boost::shared_ptr<stateT> >
      &my_states=std::vector<boost::shared_ptr<stateT> >())
  : curr_state(NULL)
  { for(unsigned int i=0; i<my_states.size(); ++i) addState(*my_states[i]); }

  //! Assignment operator
  inline StateMachine &operator=(const StateMachine &s)
  {
    if(&s == this) return *this;

    // Copy their states (not very efficient---note multiple searches)
    std::deque<stateNameT> state_names;
    std::back_insert_iterator<std::deque<stateNameT> > inserter(state_names);
    s.getStateNames(inserter);
    while(!state_names.empty()) {
      addState(s.getState(state_names.front()));
      state_names.pop_front();
    }

    // Copy their data
    data = s.getData();

    // Set our state to the current state
    stateNameT csn;
    s.getCurrStateName(csn);
    setState(csn);

    return *this;
  }
};

} // namespace BrailleTutorNS

#endif
