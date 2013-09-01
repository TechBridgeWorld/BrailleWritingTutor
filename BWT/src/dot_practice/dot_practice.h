/*
 * dot_practice.h
 *
 *  Created on: Dec 2, 2008
 *      Author: imran
 */

#ifndef DOT_PRACTICE_H_
#define DOT_PRACTICE_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"

class DotPractice : public IBTApp
{
public:
  explicit DotPractice(IOEventParser&, const std::string&, SoundsUtil*, std::vector<std::string>, std::vector<std::string>, std::vector<std::string>, bool);
  virtual ~DotPractice();
  void processEvent(IOEvent& e);

private:
  void PD_attempt(int);
  void PD_new();

private:
  enum observation
  {
    right, wrong
  };
  KnowledgeTracer dot_skill[7];
  KnowledgeTracer DS_length_skill[3];
  std::string letter;
  DotSequence target_sequence;
  DotSequence current_sequence;
  int target_length;
  int target_dot;
  bool nomirror;  
  SoundsUtil* su;
  const std::vector<std::string> short_letters;
  const std::vector<std::string> med_letters;
  const std::vector<std::string> long_letters;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishDotPractice : public DotPractice
{
public:
  explicit EnglishDotPractice(IOEventParser&);
  ~EnglishDotPractice()
  {
  }

private:
  std::vector<std::string> createShortLettersVector();
  std::vector<std::string> createMediumLettersVector();
  std::vector<std::string> createLongLettersVector();

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicDotPractice : public DotPractice
{
public:
  explicit ArabicDotPractice(IOEventParser&);
  ~ArabicDotPractice()
  {
  }

private:
  std::vector<std::string> createShortLettersVector();
  std::vector<std::string> createMediumLettersVector();
  std::vector<std::string> createLongLettersVector();

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchDotPractice : public DotPractice
{
public:
  explicit FrenchDotPractice(IOEventParser&);
  ~FrenchDotPractice()
  {
  }

private:
  std::vector<std::string> createShortLettersVector();
  std::vector<std::string> createMediumLettersVector();
  std::vector<std::string> createLongLettersVector();

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliDotPractice : public DotPractice
{
public:
  explicit KiswahiliDotPractice(IOEventParser&);
  ~KiswahiliDotPractice()
  {
  }

private:
  std::vector<std::string> createShortLettersVector();
  std::vector<std::string> createMediumLettersVector();
  std::vector<std::string> createLongLettersVector();

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2DotPractice : public DotPractice
{
public:
  explicit English2DotPractice(IOEventParser&);
  ~English2DotPractice()
  {
  }

private:
  std::vector<std::string> createShortLettersVector();
  std::vector<std::string> createMediumLettersVector();
  std::vector<std::string> createLongLettersVector();

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2DotPractice : public DotPractice
{
public:
  explicit Arabic2DotPractice(IOEventParser&);
  ~Arabic2DotPractice()
  {
  }

private:
  std::vector<std::string> createShortLettersVector();
  std::vector<std::string> createMediumLettersVector();
  std::vector<std::string> createLongLettersVector();

};

#endif /* DOT_PRACTICE_H_ */
