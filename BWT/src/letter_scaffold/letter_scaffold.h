/*
 * letter_scaffold.h
 *
 *  Created on: Dec 3, 2008
 *      Author: imran
 */

#ifndef LETTER_SCAFFOLD_H_
#define LETTER_SCAFFOLD_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/KnowledgeTracer.h"
#include "common/language_utils.h"

class LetterScaffold : public IBTApp
{
public:
  explicit LetterScaffold(IOEventParser&, const std::string&, SoundsUtil*, bool);
  virtual ~LetterScaffold();
  void processEvent(IOEvent& e);

private:
  SoundsUtil* su;
  IOEventParser& iep; //So flushGlyph() can be called
  bool firsttime;
  bool nomirror;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishLetterScaffold : public LetterScaffold
{
public:
  explicit EnglishLetterScaffold(IOEventParser&);
  ~EnglishLetterScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicLetterScaffold : public LetterScaffold
{
public:
  explicit ArabicLetterScaffold(IOEventParser&);
  ~ArabicLetterScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchLetterScaffold : public LetterScaffold
{
public:
  explicit FrenchLetterScaffold(IOEventParser&);
  ~FrenchLetterScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliLetterScaffold : public LetterScaffold
{
public:
  explicit KiswahiliLetterScaffold(IOEventParser&);
  ~KiswahiliLetterScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2LetterScaffold : public LetterScaffold
{
public:
  explicit English2LetterScaffold(IOEventParser&);
  ~English2LetterScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2LetterScaffold : public LetterScaffold
{
public:
  explicit Arabic2LetterScaffold(IOEventParser&);
  ~Arabic2LetterScaffold()
  {
  }
};
#endif /* LETTER_SCAFFOLD_H_ */
