/*
 * learn_dots.h
 *
 *  Created on: Dec 1, 2008
 *      Author: imran
 */

#ifndef LEARN_DOTS_H_
#define LEARN_DOTS_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"

class LearnDots : public IBTApp
{
public:
  explicit LearnDots(IOEventParser&, const std::string&, SoundsUtil*, bool);
  virtual ~LearnDots();
  void processEvent(IOEvent& e);

private:
  void LD_attempt(int);
  void LD_new();
private:
  SoundsUtil* su;
  enum observation
  {
    right, wrong
  };
  int target_dot;
  static const int sequence[][6];
  int seq;
  int pos;
  bool nomirror;
  KnowledgeTracer dot_skill[7];
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishLearnDots : public LearnDots
{
public:
  explicit EnglishLearnDots(IOEventParser&);
  ~EnglishLearnDots()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicLearnDots : public LearnDots
{
public:
  explicit ArabicLearnDots(IOEventParser&);
  ~ArabicLearnDots()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchLearnDots : public LearnDots
{
public:
  explicit FrenchLearnDots(IOEventParser&);
  ~FrenchLearnDots()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliLearnDots : public LearnDots
{
public:
  explicit KiswahiliLearnDots(IOEventParser&);
  ~KiswahiliLearnDots()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2LearnDots : public LearnDots
{
public:
  explicit English2LearnDots(IOEventParser&);
  ~English2LearnDots()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2LearnDots : public LearnDots
{
public:
  explicit Arabic2LearnDots(IOEventParser&);
  ~Arabic2LearnDots()
  {
  }
};

#endif /* LEARN_DOTS_H_ */
