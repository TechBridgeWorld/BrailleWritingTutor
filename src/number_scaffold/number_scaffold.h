/*
 * number_scaffold.h
 *
 *  Created on: March 8, 2008
 *      Author: imran
 */

#ifndef NUMBER_SCAFFOLD_H_
#define NUMBER_SCAFFOLD_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/language_utils.h"

class NumberScaffold : public IBTApp
{
public:
  explicit NumberScaffold(IOEventParser&, const std::string&, SoundsUtil*, bool);
  virtual ~NumberScaffold();
  void processEvent(IOEvent& e);

private:
  SoundsUtil* su;
  IOEventParser& iep; //So flushGlyph() can be called
  bool firsttime;
  const Voice math_s;
  bool nomirror;
  
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishNumberScaffold : public NumberScaffold
{
public:
  explicit EnglishNumberScaffold(IOEventParser&);
  ~EnglishNumberScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicNumberScaffold : public NumberScaffold
{
public:
  explicit ArabicNumberScaffold(IOEventParser&);
  ~ArabicNumberScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchNumberScaffold : public NumberScaffold
{
public:
  explicit FrenchNumberScaffold(IOEventParser&);
  ~FrenchNumberScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliNumberScaffold : public NumberScaffold
{
public:
  explicit KiswahiliNumberScaffold(IOEventParser&);
  ~KiswahiliNumberScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2NumberScaffold : public NumberScaffold
{
public:
  explicit English2NumberScaffold(IOEventParser&);
  ~English2NumberScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2NumberScaffold : public NumberScaffold
{
public:
  explicit Arabic2NumberScaffold(IOEventParser&);
  ~Arabic2NumberScaffold()
  {
  }
};
//+++++++++++
#endif /* NUMBER_SCAFFOLD_H_ */
