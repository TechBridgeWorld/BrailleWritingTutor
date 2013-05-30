/*
 * arabic_utils.cc
 *
 *  Created on: Dec 18, 2008
 *      Author: imran
 */

#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <map>
#include "language_utils.h"
#include "Dots.h" //for dot_mask()
const Arabic2SoundsUtil::Arabic2LettersToSoundFilesMap
    Arabic2SoundsUtil::arabic2_letter_map =
        boost::assign::map_list_of("ا", "aleph.wav")("ب", "baa.wav")("ت", "taa.wav")("ث", "thaa.wav")("ج", "jeem.wav")("ح", "hhaa.wav")("خ", "khaa.wav")("د", "dal.wav")("ذ", "tdzaal.wav")("ر", "raa.wav")("ز", "zeyn.wav")("س", "seen.wav")("ش", "sheen.wav")("ص", "saad.wav")("ض", "daad.wav")("ط", "tduh.wav")("ظ", "zhuh.wav")("ع", "eeyn.wav")("غ", "gheyn.wav")("ف", "faa.wav")("ق", "qaaf.wav")("ك", "kaaf.wav")("ل", "laam.wav")("م", "meem.wav")("ن", "noon.wav")("ه", "haaa.wav")("و", "wow.wav")("ي", "yaa.wav");

void Arabic2SoundsUtil::sayNumber(const Voice &v, int number, bool flip) const
{
	char sound[] = "?_arabic.wav";	
	
	/* This was added to remove mirroring for Al-Noor Institute since they don't use slate & stylus - thus no concept of mirrored braille cell */
	if(flip)
	{
		if(number==1){
			sprintf(sound, "%d_arabic.wav", 4);
		}else if(number==2){
			sprintf(sound, "%d_arabic.wav", 5);
		}else if(number==3){
			sprintf(sound, "%d_arabic.wav", 6);
		}else if(number==4){	
			sprintf(sound, "%d_arabic.wav", 1);
		}else if(number==5){
			sprintf(sound, "%d_arabic.wav", 2);
		}else{
			sprintf(sound, "%d_arabic.wav", 3);
		}
	}	
	else
		sprintf(sound, "%d_arabic.wav", number);
	
	v.say(sound);
}

void Arabic2SoundsUtil::sayLetter(const Voice &v, const std::string& letter) const
{
	
  if( letter.size() > 2 ) //Because a single arabic letter occupies 2 bytes in UTF8
  {
    std::cerr << "Ignoring. sayLetter() was expecting a single letter, but received more." << std::endl;
    return;
  }

  Arabic2LettersToSoundFilesMap::const_iterator iter = arabic2_letter_map.find(letter);
  if( iter == arabic2_letter_map.end() ) //this arabic letter does not have a corresponding mapping file (or maybe its not a valid arabic letter to begin with hence it doesnt have a mapping file)
  {
    std::cerr << "Ignoring. Mapping does not exist for:"<<letter<< std::endl;
    return;
  }
  else
  {
    try
    {
      v.say(iter->second);
    }
    catch (std::string& s)
    {
      std::cerr << "Soundfile not found:" << s << std::endl;
    }
  }
}

void Arabic2SoundsUtil::sayLetterSequence(const Voice &v, const std::string& word) const
{
	std::string::const_iterator iter = word.begin();
  while( iter != word.end() )
  {
    std::string arabic_letter(iter, iter + 2); //All Arabic characters encoded in UTF8 are 2 bytes
    sayLetter(v, arabic_letter);
    iter = iter + 2;
  }
}

void Arabic2SoundsUtil::sayDotSequence(const Voice &v, DotSequence d) const
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

void Arabic2SoundsUtil::saySound(const Voice& v, const std::string& sound) const
{
		
  std::string sound_file(sound);
  sound_file.append("_arabic.wav");
  v.say(sound_file);
}
