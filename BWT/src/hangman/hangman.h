/*
 * hangman.h
 *
 *  Created on: Dec 9, 2008
 *      Author: Students
 *      Multi-lingual enhancements: Imran
 */

#ifndef HANGMAN_H_
#define HANGMAN_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"

class Hangman : public IBTApp
{
public:
  explicit Hangman(IOEventParser&, const std::string&, SoundsUtil*, const std::vector<std::string>, const std::vector<
      std::string>, const std::vector<std::string>, const std::vector<std::string>, const std::vector<std::string>, const std::vector<
      std::string>, bool);
  virtual ~Hangman()
  {
  }

private:
  void HM_new();
  void HM_attempt(std::string);
  void processEvent(IOEvent& e);

private:

  enum observation
  {
    right, wrong
  };

  IOEventParser& iep; //So flushGlyph() can be called
  SoundsUtil* su;

  const std::vector<std::string> alphabet;
  const std::vector<std::string> short_words;
  const std::vector<std::string> med_words;
  const std::vector<std::string> long_words;
  const std::vector<std::string> xlong_words;
  const std::vector<std::string> xxlong_words;

  std::vector<KnowledgeTracer> letter_skill;

  bool nomirror;
  bool firsttime;
  int turncount;
  int mistake; // number of mistakes in hangman
  int correctcount;
  int j;
  std::string word;
  std::string target_letter;
  int word_pos;
  int word_length;
  std::string answer; // hangman answer

  KnowledgeTracer LS_length_skill[8];
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishHangman : public Hangman
{
public:
  explicit EnglishHangman(IOEventParser&);
  ~EnglishHangman()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
  const std::vector<std::string> createxLongWords() const;
  const std::vector<std::string> createxxLongWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class ArabicHangman : public Hangman
{
public:
  explicit ArabicHangman(IOEventParser&);
  ~ArabicHangman()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
  const std::vector<std::string> createxLongWords() const;
  const std::vector<std::string> createxxLongWords() const;};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class FrenchHangman : public Hangman
{
public:
  explicit FrenchHangman(IOEventParser&);
  ~FrenchHangman()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
  const std::vector<std::string> createxLongWords() const;
  const std::vector<std::string> createxxLongWords() const;};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class KiswahiliHangman : public Hangman
{
public:
  explicit KiswahiliHangman(IOEventParser&);
  ~KiswahiliHangman()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
  const std::vector<std::string> createxLongWords() const;
  const std::vector<std::string> createxxLongWords() const;};
  
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2Hangman : public Hangman
{
public:
  explicit English2Hangman(IOEventParser&);
  ~English2Hangman()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
  const std::vector<std::string> createxLongWords() const;
  const std::vector<std::string> createxxLongWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class Arabic2Hangman : public Hangman
{
public:
  explicit Arabic2Hangman(IOEventParser&);
  ~Arabic2Hangman()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
  const std::vector<std::string> createxLongWords() const;
  const std::vector<std::string> createxxLongWords() const;};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
#endif /* HANGMAN_H_ */
