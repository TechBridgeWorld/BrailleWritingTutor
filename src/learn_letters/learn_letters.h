/*
 * learn_letters.h
 *
 *  Created on: Dec 3, 2008
 *      Author: imran
 */

#ifndef LEARN_LETTERS_H_
#define LEARN_LETTERS_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"
#include "common/multicharacter.h"

class LearnLetters : public IBTApp
{
public:
  explicit
      LearnLetters(IOEventParser&, const std::string&, SoundsUtil*, const std::vector<std::string>, const std::vector<std::string> g0, const std::vector<
          std::string>, const std::vector<std::string>, const std::vector<std::string>, const std::vector<std::string>, bool);
  ~LearnLetters();
  void processEvent(IOEvent& e);

private:
  void LL_attempt(int);
  void LL_new();
  float group_skill(int);
  int getGroupSize(int);
  bool nomirror;

private:
  SoundsUtil* su;
  multi* multicell; 
  enum observation
  {
    right, wrong
  };
  int cell_position; // for multi-cell characters
  //IOEventParser& iep; //So flushGlyph() can be called
  const std::vector<std::string> alphabet;
  const std::vector<std::string> group0;
  const std::vector<std::string> group1;
  const std::vector<std::string> group2;
  const std::vector<std::string> group3;
  const std::vector<std::string> group4;

  int target_group; //which of those groups 0-4 will be drawn from
  int target_index; //alphabetic index of target letter
  DotSequence target_sequence;
  DotSequence current_sequence;
  std::vector<KnowledgeTracer> letter_skill;

  /*
   * NOTE:Some aspects of the code are dirty (eg: all those vectors..) because I wanted to change as little
   * of the original code as possible while introducing multi-lingual support to this apps.
   */
  /*
   * NOTE: In order to comply with the legacy code, it is important that the vectors created by the createGroupXLetters()
   * functions correspond to the alphabet vector. To see what I mean, observe how EnglishLearnLetters implements those functions.
   */

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishLearnLetters : public LearnLetters
{
public:
  explicit EnglishLearnLetters(IOEventParser&);
  ~EnglishLearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


class KannadaLearnLetters : public LearnLetters
{
public:
  explicit KannadaLearnLetters(IOEventParser&);
  ~KannadaLearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicLearnLetters : public LearnLetters
{
public:
  explicit ArabicLearnLetters(IOEventParser&);
  ~ArabicLearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchLearnLetters : public LearnLetters
{
public:
  explicit FrenchLearnLetters(IOEventParser&);
  ~FrenchLearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliLearnLetters : public LearnLetters
{
public:
  explicit KiswahiliLearnLetters(IOEventParser&);
  ~KiswahiliLearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2LearnLetters : public LearnLetters
{
public:
  explicit English2LearnLetters(IOEventParser&);
  ~English2LearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2LearnLetters : public LearnLetters
{
public:
  explicit Arabic2LearnLetters(IOEventParser&);
  ~Arabic2LearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Hindi2LearnLetters : public LearnLetters
{
public:
  explicit Hindi2LearnLetters(IOEventParser&);
  ~Hindi2LearnLetters()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createGroup0Letters() const;
  const std::vector<std::string> createGroup1Letters() const;
  const std::vector<std::string> createGroup2Letters() const;
  const std::vector<std::string> createGroup3Letters() const;
  const std::vector<std::string> createGroup4Letters() const;
};
//
#endif /* LEARN_LETTERS_H_ */
