#ifndef KNOWLEDGE_TRACER_H_
#define KNOWLEDGE_TRACER_H_

#include <exception>
#include <map>
#include <iterator>
#include <exception>

typedef int event; //events to be passed in are ints

//used internally for map
struct  event_lt{
  bool operator()(event e1, event e2){
    return e1 < e2;
  }
};

class KnowledgeTracer
{
 public:
  KnowledgeTracer();

  /*
   * constructs a KnowledgeTracer object to track an independant, binary skill.
   * `initial' is our initial estimate of the probability (between 0 and 1) that the skill is known.
   */
  KnowledgeTracer(double initial);

  /*
   * Registers a type of event with the KnowledgeTracer.
   * An event must be registered before it is observed.
   * ID is an event (int) that will be used to refer to this event in reporting observations to the KT.
   *  (the user is invited to enum some events for their convenience)
   * PGS is the probability, given that the skill is known, that the event will be observed.
   * PGU is the probability, given that the skill is NOT known, that the event will be observed.
   */
  KnowledgeTracer& registerEvent(event ID, double PGS, double PGU);

  /*
   * Takes ID of event observed.
   * Updates KT's estimate of the chance that the skill is known.
   * If the ID has not been registered, throws the ID.
   * This exception is a programmer error, not a valid exception state.
   */
  void observe(event ID);

  /*
   * Returns the KT's current estimate of the chance that the skill is known
   * (between 0 and 1)
   */
  double estimate();

  /*
   * Resets the KT in the event that an activity has been finished in order to 
   * allow the tutor to repeat the activity with a blank slate
   */
  void reset();

 private:

  double prob_skilled;

  std::map<event, double, event_lt> PGS_map;
  std::map<event, double, event_lt> PGU_map;
};
#endif
