/*
 * letter_practice.h
 *
 *  Created on: Dec 3, 2008
 *      Author: imran
 */

#ifndef LETTER_PRACTICE_H_
#define LETTER_PRACTICE_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"

class LetterPractice : public IBTApp
{
public:
  explicit
      LetterPractice(IOEventParser&, const std::string&, SoundsUtil*, const std::vector<std::string>, const std::vector<std::string>, const std::vector<
          std::string>, const std::vector<std::string>, bool);
  virtual ~LetterPractice();
  void processEvent(IOEvent& e);

private:
  void PL_attempt(std::string);
  void PL_new();
  bool nomirror;

private:
  enum observation
  {
    right, wrong
  };
  SoundsUtil* su;
  IOEventParser& iep; //So flushGlyph() can be called

  const std::vector<std::string> alphabet;
  const std::vector<std::string> short_words;
  const std::vector<std::string> med_words;
  const std::vector<std::string> long_words;

  std::vector<KnowledgeTracer> letter_skill;
  KnowledgeTracer LS_length_skill[5];
  std::string word;
  std::string target_letter;
  int word_pos;
  int word_length;
  bool firsttime;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishLetterPractice : public LetterPractice
{
public:
  explicit EnglishLetterPractice(IOEventParser&);
  ~EnglishLetterPractice()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicLetterPractice : public LetterPractice
{
public:
  explicit ArabicLetterPractice(IOEventParser&);
  ~ArabicLetterPractice()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchLetterPractice : public LetterPractice
{
public:
  explicit FrenchLetterPractice(IOEventParser&);
  ~FrenchLetterPractice()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliLetterPractice : public LetterPractice
{
public:
  explicit KiswahiliLetterPractice(IOEventParser&);
  ~KiswahiliLetterPractice()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2LetterPractice : public LetterPractice
{
public:
  explicit English2LetterPractice(IOEventParser&);
  ~English2LetterPractice()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2LetterPractice : public LetterPractice
{
public:
  explicit Arabic2LetterPractice(IOEventParser&);
  ~Arabic2LetterPractice()
  {
  }
private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createShortWords() const;
  const std::vector<std::string> createMedWords() const;
  const std::vector<std::string> createLongWords() const;
};

#endif /* LETTER_PRACTICE_H_ */
