/*
 * letter_scaffold.cc
 *
 *  Created on: Dec 3, 2008
 *      Author: imran
 */

#include "letter_scaffold.h"

LetterScaffold::LetterScaffold(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, bool f) :
  IBTApp(my_iep, path_to_mapping_file), su(my_su), iep(my_iep), firsttime(true), nomirror(f)
{
  su->saySound(getTeacherVoice(), "free spelling");
}

LetterScaffold::~LetterScaffold()
{
  delete su;
}

void LetterScaffold::processEvent(IOEvent& e)
{
  //echo back the dots the user is typing
  int dot = getDot(e);
  if( isDown(e) && dot > 0 )
    su->sayNumber(getStudentVoice(), dot, nomirror);

  //Whenever the user hits Button0 we immediately want the LETTER event to be generated so that he doesnt have to wait for the timeout
  if( e.type == IOEvent::BUTTON && e.button == 0 )
  {
    iep.flushGlyph();
    return; //required? hmm..
  }

  if( e.type == IOEvent::BUTTON_LETTER || e.type == IOEvent::CELL_LETTER )
  {
    printEvent(e);
    //We dont want any pending LETTER events to interfere. So we skip the first LETTER event.
    if( firsttime )//Check if this is the first letter event, if so, we skip it
    {
      std::cout << "    (DEBUG)Skipping first letter event" << std::endl;
      firsttime = false;
      return;//skip
    }
    else
    {
      su->sayLetter(getTeacherVoice(), (std::string) e.letter);
    }
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishLetterScaffold::EnglishLetterScaffold(IOEventParser& my_iep) :
  LetterScaffold(my_iep, "", new EnglishSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ArabicLetterScaffold::ArabicLetterScaffold(IOEventParser& my_iep) :
  LetterScaffold(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FrenchLetterScaffold::FrenchLetterScaffold(IOEventParser& my_iep) :
  LetterScaffold(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliLetterScaffold::KiswahiliLetterScaffold(IOEventParser& my_iep) :
  LetterScaffold(my_iep, "", new KiswahiliSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
English2LetterScaffold::English2LetterScaffold(IOEventParser& my_iep) :
  LetterScaffold(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, true)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Arabic2LetterScaffold::Arabic2LetterScaffold(IOEventParser& my_iep) :
  LetterScaffold(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, true)
{

}
