/*
 * dot_scaffold.h
 *
 *  Created on: Nov 30, 2008
 *      Author: imran
 */

#ifndef DOT_SCAFFOLD_H_
#define DOT_SCAFFOLD_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/language_utils.h"

//+++++++++++++++++++++++++++++++++++++++
class DotScaffold : public IBTApp
{
public:
  explicit DotScaffold(IOEventParser&, const std::string&, SoundsUtil*, bool);
  virtual ~DotScaffold();
  void processEvent(IOEvent&);

private:
  SoundsUtil* su;
  bool nomirror;

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishDotScaffold : public DotScaffold
{
public:
  explicit EnglishDotScaffold(IOEventParser&);
  ~EnglishDotScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicDotScaffold : public DotScaffold
{
public:
  explicit ArabicDotScaffold(IOEventParser&);
  ~ArabicDotScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchDotScaffold : public DotScaffold
{
public:
  explicit FrenchDotScaffold(IOEventParser&);
  ~FrenchDotScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliDotScaffold : public DotScaffold
{
public:
  explicit KiswahiliDotScaffold(IOEventParser&);
  ~KiswahiliDotScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2DotScaffold : public DotScaffold
{
public:
  explicit English2DotScaffold(IOEventParser&);
  ~English2DotScaffold()
  {
  }
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2DotScaffold : public DotScaffold
{
public:
  explicit Arabic2DotScaffold(IOEventParser&);
  ~Arabic2DotScaffold()
  {
  }
};

#endif /* DOT_SCAFFOLD_H_ */
