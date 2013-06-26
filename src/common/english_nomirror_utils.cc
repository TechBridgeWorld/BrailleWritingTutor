/*
 * english_nomirror_utils.cc
 *
 *  Created on: March 16 2010
 *      Author: Ameer
 *      Description: This is for the unmirrored version of English
 */

#include "language_utils.h"
#include "Dots.h" //for dot_mask()

void English2SoundsUtil::sayNumber(const Voice &v, int number, bool flip) const
{
  char sound[] = "?.wav";
  
	if(flip)
	{	
		if(number==1){
			sprintf(sound, "%d.wav", 4);
		}else if(number==2){
			sprintf(sound, "%d.wav", 5);
		}else if(number==3){
			sprintf(sound, "%d.wav", 6);
		}else if(number==4){	
			sprintf(sound, "%d.wav", 1);
		}else if(number==5){
			sprintf(sound, "%d.wav", 2);
		}else{
			sprintf(sound, "%d.wav", 3);
		}
	}
	else
		sprintf(sound, "%d.wav", number);
		
	v.say(sound);
}

void English2SoundsUtil::sayLetter(const Voice &v, const std::string& letter) const
{
  if( letter.size() > 1 )
  {
    std::cerr << "Ignoring. sayLetter() was expecting a single letter, but received more." << std::endl;
    return;
  }

  char sound[] = "?.wav";
  sprintf(sound, "%s.wav", letter.c_str());
  try
  {
    v.say(sound);
  }
  catch (std::string& s)
  {
    std::cerr << "Soundfile not found:" << s << std::endl;
  }
}

void English2SoundsUtil::sayLetterSequence(const Voice &v, const std::string& word) const
{
  for(unsigned int i = 0; i < word.size(); i++)
  {
    std::string letter(1, word.at(i));
    sayLetter(v, letter);
  }
}

void English2SoundsUtil::sayDotSequence(const Voice &v, DotSequence d) const
{
  bool sortedSequence[6] = {false,false,false,false,false,false};
  
  try
  {
     // save activated dots so we can speak them in proper order later.
             
    // this goes from bottom to top, starting on left column then right column. ie dots 321654
    for(int i = 5; i >= 0; i--)
      if( d & BrailleTutorNS::dot_mask(i) )
      {
		 //TODO: Cleanup with math
		if(i==0)
		  sortedSequence[3]=true;
		else if(i==1)
		  sortedSequence[4]=true;
		else if(i==2)
		  sortedSequence[5]=true;
		else if(i==3)
		  sortedSequence[0]=true;
		else if(i==4)
		  sortedSequence[1]=true;
		else if(i==5)
		  sortedSequence[2]=true;
	  }
        
        //std::cout << "speaking sorted sequence" << std::endl;
    for(int i = 0; i <= 5; i++)
        if(sortedSequence[i])
          sayNumber(v, i+1, false); //+1 because while speaking, the dots range from 1 to 6 
  }
  catch (std::string& e)
  {
    std::cout << "Exception occured, sound file not found in map:" << e << std::endl;
  }
}

void English2SoundsUtil::saySound(const Voice& v, const std::string& sound) const
{
  std::string sound_file(sound);
  sound_file.append(".wav"); //english sound files dont have any suffix so we just add the .wav
  v.say(sound_file);
  

}
