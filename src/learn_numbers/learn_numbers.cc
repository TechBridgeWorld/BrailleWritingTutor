/*
 * learn_numbers.cc
 *
 *  Created on: Mar 8, 2009
 *      Author: imran
 */
#include "learn_numbers.h"
#include "Dots.h"

LearnNumbers::LearnNumbers(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, bool f) :
  IBTApp(my_iep, path_to_mapping_file), su(my_su), math_s("./resources/Voice/math_sounds/"), target_sequence('\0'), current_sequence('\0'), nomirror(f)
{
  su->saySound(math_s, "learn_numbers");
  LN_new();
}

LearnNumbers::~LearnNumbers()
{
  delete su;
}

void LearnNumbers::processEvent(IOEvent& e)
{
	//std::cout << "processEvent" << std::endl;
  if( e.type == IOEvent::BUTTON_DOWN && e.button == 0 )
    return;

  if( e.type == IOEvent::STYLUS_DOWN || e.type == IOEvent::BUTTON_DOWN )
  {
    LN_attempt(getDot(e));
  }
}
void LearnNumbers::LN_new()
{
  //generate a random number between 0 to 9
  int n = (int) (10.0 * rand() / (RAND_MAX+1.0)); //better alternative than rand()%10
  current_sequence = '\0';
  target_sequence = convertToDotSequence(IBTApp::getCurrentCharset(), n);
  std::cout<<"Chosen number is:"<<n<<std::endl;

  //
  playWriteTheNumberSounds(target_sequence);
}

void LearnNumbers::LN_attempt(unsigned char dot)
{
	su->sayNumber(getStudentVoice(), dot, nomirror);

  //Check if user hit the right dot (ie, the dot exists in the target sequence)
  if( my_dot_mask(dot) & target_sequence )
  {
    current_sequence = current_sequence | my_dot_mask(dot); //add the dot to the current on-going sequence
    //are we done?
    if( current_sequence == target_sequence )
    {
      su->saySound(getTeacherVoice(), "good");
      LN_new();
      return;
    }
  }
  else
  {
    su->saySound(getTeacherVoice(), "no");
    playWriteTheNumberSounds(target_sequence);
  }
}

void LearnNumbers::playWriteTheNumberSounds(const DotSequence& d) const
{
  std::cout << "playWriteTheNumberSounds" << std::endl;
  
  static const Charset& charset = IBTApp::getCurrentCharset();
  int current_number = atoi(((std::string) charset[d]).c_str()); //mapping the target_sequence to it's original number

  su->saySound(math_s, "to_write_number");
  su->sayNumber(getTeacherVoice(), current_number,false); //flip is false regardless of mirrored or unmirrored
  su->saySound(getTeacherVoice(), "press");
  su->sayDotSequence(getTeacherVoice(), d);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishLearnNumbers::EnglishLearnNumbers(IOEventParser& my_iep) :
  LearnNumbers(my_iep, "./language_mapping_files/number_mapping.txt", new EnglishSoundsUtil, false)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ArabicLearnNumbers::ArabicLearnNumbers(IOEventParser& my_iep) :
  LearnNumbers(my_iep, "./language_mapping_files/number_mapping.txt", new ArabicSoundsUtil, false)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FrenchLearnNumbers::FrenchLearnNumbers(IOEventParser& my_iep) :
  LearnNumbers(my_iep, "./language_mapping_files/number_mapping.txt", new FrenchSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliLearnNumbers::KiswahiliLearnNumbers(IOEventParser& my_iep) :
  LearnNumbers(my_iep, "./language_mapping_files/number_mapping.txt", new KiswahiliSoundsUtil, false)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Arabic2LearnNumbers::Arabic2LearnNumbers(IOEventParser& my_iep) :
  LearnNumbers(my_iep, "./language_mapping_files/number_mapping_unmirrored.txt", new Arabic2SoundsUtil, true)
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
English2LearnNumbers::English2LearnNumbers(IOEventParser& my_iep) :
  LearnNumbers(my_iep, "./language_mapping_files/number_mapping_unmirrored.txt", new English2SoundsUtil, true)
{

}


