/*
 * english_utils.cc
 *
 *  Created on: Dec 18, 2008
 *      Author: imran
 */

#include "language_utils.h"
#include "Dots.h" //for dot_mask()
void EnglishSoundsUtil::sayNumber(const Voice &v, int number, bool f) const
{
    std::cout << "EnglishSoundUtils sayNumber" << std::endl;
	char sound[] = "?.wav";
  	sprintf(sound, "%d.wav", number);		
	v.say(sound);
}

void EnglishSoundsUtil::sayLetter(const Voice &v, const std::string& letter) const
{

  char invalid[] = "invalid pattern.wav";
  if( letter.size() > 1 ) {
    std::cerr << "Ignoring. sayLetter() was expecting a single letter, but received more." << std::endl;
    v.say(invalid);
    return;
  }


  char sound[] = "?.wav";

  switch (letter.c_str()[0]) {
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
  case 'I':
  case 'J':
  case 'K':
  case 'L':
  case 'M':
  case 'N':
  case 'O':
  case 'P':
  case 'Q':
  case 'R':
  case 'S':
  case 'T':
  case 'U':
  case 'V':
  case 'W':
  case 'X':
  case 'Y':
  case 'Z':
    sprintf(sound, "%s.wav", letter.c_str());
    try {
      v.say(sound);
    } catch (std::string& s) {
      std::cerr << "Soundfile not found:" << s <<std::endl;
    }
    break;

  default:
    v.say(invalid);
  }
 
}

void EnglishSoundsUtil::sayLetterSequence(const Voice &v, const std::string& word) const
{
  for(unsigned int i = 0; i < word.size(); i++)
  {
    std::string letter(1, word.at(i));
    sayLetter(v, letter);
  }
}

void EnglishSoundsUtil::sayDotSequence(const Voice &v, DotSequence d) const
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

void EnglishSoundsUtil::saySound(const Voice& v, const std::string& sound) const
{
 
  std::string sound_file(sound);
  sound_file.append(".wav"); //english sound files dont have any suffix so we just add the .wav
  v.say(sound_file);
  
}
