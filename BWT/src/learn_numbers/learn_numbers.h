/*
 * learn_numbers.h
 *
 *  Created on: Mar 8, 2009
 *      Author: imran
 */

#ifndef LEARN_NUMBERS_H_
#define LEARN_NUMBERS_H_

#include "common/IBTApp.h"
#include "common/language_utils.h"
#include "common/utilities.h"

class LearnNumbers : public IBTApp
{
public:
	// added string to hold number mapping file path
  explicit LearnNumbers(IOEventParser&, const std::string&, SoundsUtil*, bool);
  ~LearnNumbers();

  void processEvent(IOEvent& event);

private:
  void LN_new();
  void LN_attempt(unsigned char);
  void playWriteTheNumberSounds(const DotSequence& d) const;

private:
  IOEventParser& iep; 
  SoundsUtil* su;
  const Voice math_s;
  bool nomirror;
  DotSequence target_sequence;//the dot sequence of the target number(between 0-9)
  DotSequence current_sequence;//holds the on-going dot-sequence the user is entering..
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishLearnNumbers : public LearnNumbers
{
public:
  explicit EnglishLearnNumbers(IOEventParser&);
  ~EnglishLearnNumbers()
  {

  }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicLearnNumbers : public LearnNumbers
{
public:
  explicit ArabicLearnNumbers(IOEventParser&);
  ~ArabicLearnNumbers()
  {

  }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchLearnNumbers : public LearnNumbers
{
public:
  explicit FrenchLearnNumbers(IOEventParser&);
  ~FrenchLearnNumbers()
  {

  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliLearnNumbers : public LearnNumbers
{
public:
  explicit KiswahiliLearnNumbers(IOEventParser&);
  ~KiswahiliLearnNumbers()
  {

  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2LearnNumbers : public LearnNumbers
{
public:
  explicit English2LearnNumbers(IOEventParser&);
  ~English2LearnNumbers()
  {

  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2LearnNumbers : public LearnNumbers
{
public:
  explicit Arabic2LearnNumbers(IOEventParser&);
  ~Arabic2LearnNumbers()
  {

  }
};


#endif /* LEARN_NUMBERS_H_ */
