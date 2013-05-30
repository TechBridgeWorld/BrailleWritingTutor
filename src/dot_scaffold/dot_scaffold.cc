/*
 * dot_scaffold.cc
 *
 *  Created on: Nov 30, 2008
 *      Author: imran
 */

#include "dot_scaffold.h"

using namespace BrailleTutorNS;

DotScaffold::DotScaffold(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, bool f) :
  IBTApp(my_iep, path_to_mapping_file), su(my_su), nomirror(f)
{

}

DotScaffold::~DotScaffold()
{
  delete su;
}

void DotScaffold::processEvent(IOEvent& e)
{
  if( isDown(e) )
  {
    int num = getDot(e);
    if( num > 0 )
    {
      su->sayNumber(getTeacherVoice(), num, nomirror);
      printEvent(e);
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EnglishDotScaffold::EnglishDotScaffold(IOEventParser& my_iep) :
  DotScaffold(my_iep, "", new EnglishSoundsUtil, false)
{
  getTeacherVoice().say("free play.wav");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ArabicDotScaffold::ArabicDotScaffold(IOEventParser& my_iep) :
  DotScaffold(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, false)
{
  getTeacherVoice().say("free play_arabic.wav");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FrenchDotScaffold::FrenchDotScaffold(IOEventParser& my_iep) :
  DotScaffold(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, false)
{
  getTeacherVoice().say("free play_french.wav");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KiswahiliDotScaffold::KiswahiliDotScaffold(IOEventParser& my_iep) :
  DotScaffold(my_iep, "", new KiswahiliSoundsUtil, false)
{
  getTeacherVoice().say("free play_kiswahili.wav");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

English2DotScaffold::English2DotScaffold(IOEventParser& my_iep) :
  DotScaffold(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, true)
{
  getTeacherVoice().say("free play.wav");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Arabic2DotScaffold::Arabic2DotScaffold(IOEventParser& my_iep) :
  DotScaffold(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, true)
{
  getTeacherVoice().say("free play_arabic.wav");
}
