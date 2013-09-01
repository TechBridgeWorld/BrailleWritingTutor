/*
 * hangman_int2011.h
 *
 */

#ifndef HANGMAN_INT2011_H_
#define HANGMAN_INT2011_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/language_utils.h"

class Int2011 : public IBTApp, public boost::noncopyable
{
public:
  explicit Int2011(IOEventParser&, const std::string&, SoundsUtil*, const std::vector<std::string>, const std::vector<std::string>);
  virtual ~Int2011();
  void processEvent(IOEvent& e);

private:	
  SoundsUtil* su;
  Voice int2011sounds;
  IOEventParser& iep; //So flushGlyph() can be called
  bool button_zero_down;


  enum observation
  {
    right, wrong
  };

  const std::vector<std::string> alphabet;
  const std::vector<std::string> int2011_words;

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

  std::vector<std::string> choices;
  void HM_new();
  void HM_attempt(std::string);

};

class EnglishInt2011 : public Int2011
{
public:
  explicit EnglishInt2011(IOEventParser&);
  ~EnglishInt2011()
  {
  }

private:
  const std::vector<std::string> createAlphabet() const;
  const std::vector<std::string> createWords() const;
};

#endif
