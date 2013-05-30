#include <iostream>
#include "KnowledgeTracer.h"

/*
 * PGS: Probability given skilled
 * PGU: Probability given unskilled
 */

KnowledgeTracer::KnowledgeTracer(){

  prob_skilled = .01;

}

KnowledgeTracer::KnowledgeTracer(double initial){

  prob_skilled = initial;

}

KnowledgeTracer& KnowledgeTracer::registerEvent(event ID, double PGS, double PGU){

  PGS_map[ID] = PGS;
  PGU_map[ID] = PGU;
  return *this; //for method chaining 

}

void KnowledgeTracer::observe(event ID){

  if(PGS_map.find(ID) == PGS_map.end()) //if ID wasn't found
    throw ID;

  //use Bayes' thm to update our estimate:
  prob_skilled = prob_skilled * PGS_map[ID]/(PGS_map[ID]*prob_skilled + PGU_map[ID]*(1-prob_skilled));

}

void KnowledgeTracer::reset(){
  prob_skilled = .01;
}

double KnowledgeTracer::estimate(){
  return prob_skilled;
}
