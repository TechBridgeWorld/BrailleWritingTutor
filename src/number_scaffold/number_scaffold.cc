/*
 * number_scaffold.cc
 *
 *  Created on: March 8, 2008
 *      Author: imran
 */

#include "number_scaffold.h"

NumberScaffold::NumberScaffold(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, bool f) :
  IBTApp(my_iep, path_to_mapping_file), su(my_su), iep(my_iep), firsttime(true),math_s("./resources/Voice/math_sounds/"), nomirror(f)
{
  su->saySound(math_s, "free_number_practice");
}

NumberScaffold::~NumberScaffold()
{
  delete su;
}

void NumberScaffold::processEvent(IOEvent& e)
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
      su->sayNumber(getTeacherVoice(), atoi(((std::string) e.letter).c_str()), nomirror);//here letter actually means the number
    }
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishNumberScaffold::EnglishNumberScaffold(IOEventParser& my_iep) :
  NumberScaffold(my_iep, "", new EnglishSoundsUtil,false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ArabicNumberScaffold::ArabicNumberScaffold(IOEventParser& my_iep) :
  NumberScaffold(my_iep, "", new ArabicSoundsUtil,false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FrenchNumberScaffold::FrenchNumberScaffold(IOEventParser& my_iep) :
  NumberScaffold(my_iep, "", new FrenchSoundsUtil,false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliNumberScaffold::KiswahiliNumberScaffold(IOEventParser& my_iep) :
  NumberScaffold(my_iep, "", new KiswahiliSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
English2NumberScaffold::English2NumberScaffold(IOEventParser& my_iep) :
  NumberScaffold(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, true)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Arabic2NumberScaffold::Arabic2NumberScaffold(IOEventParser& my_iep) :
  NumberScaffold(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, true)
{

}
