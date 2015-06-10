/*
 * kiswahili_utils.cc
 *
 *  Created on: Jun 22, 2009
 *      Author: imran
 *	Modified: iSTEP
 */

#include "language_utils.h"
#include "Dots.h" //for dot_mask()
void KiswahiliSoundsUtil::sayNumber(const Voice &v, int number, bool f) const
{
  char sound[] = "?_kiswahili.wav";
  sprintf(sound, "%d_kiswahili.wav", number);
  v.say(sound);
}

void KiswahiliSoundsUtil::sayLetter(const Voice &v, const std::string& letter) const
{
  if( letter.size() > 1 )
  {
    std::cerr << "Ignoring. sayLetter() was expecting a single letter, but received more." << std::endl;
    return;
  }

  char sound[] = "?_kiswahili.wav";
  sprintf(sound, "%s_kiswahili.wav", letter.c_str());
  try
  {
    v.say(sound);
  }
  catch (std::string& s)
  {
    std::cerr << "Soundfile not found:" << s << std::endl;
  }
}

void KiswahiliSoundsUtil::sayLetterSequence(const Voice &v, const std::string& word) const
{
  for(unsigned int i = 0; i < word.size(); i++)
  {
    std::string letter(1, word.at(i));
    sayLetter(v, letter);
  }
}

void KiswahiliSoundsUtil::sayDotSequence(const Voice &v, DotSequence d) const
{
  try
  {
    for(unsigned int i = 0; i <= 5; i++)
      if( d & BrailleTutorNS::dot_mask(i) )
        sayNumber(v, i + 1, false); //+1 because while speaking, the dots range from 1 to 6
  }
  catch (std::string& e)
  {
    std::cout << "Exception occured, sound file not found in map:" << e << std::endl;
  }
}

void KiswahiliSoundsUtil::saySound(const Voice& v, const std::string& sound) const
{
  std::string sound_file(sound);
  sound_file.append("_kiswahili.wav");
  v.say(sound_file);
}
