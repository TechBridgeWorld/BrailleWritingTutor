/*
 * hindi_utils.cc
 *
 *  Created June 2013
 *      Author: madeleine clute
 */

#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <map>
#include "language_utils.h"
#include "Dots.h" //for dot_mask()
const Hindi2SoundsUtil::Hindi2LettersToSoundFilesMap
    Hindi2SoundsUtil::Hindi2_letter_map =
        boost::assign::map_list_of("अ", "HIN_A.wav")
                                  ("आ", "HIN_AA.wav")
                                  ("इ", "HIN_I.wav")
                                  ("ई", "HIN_II.wav") 
                                  ("उ", "HIN_U.wav")
                                  ("ऊ", "HIN_UU.wav")
                                  ("ऋ", "HIN_RU.wav")

                                  ("ए", "HIN_E.wav")
                                  ("ऐ", "HIN_AI.wav")
                                  ("ओ", "HIN_O.wav")
                                  ("औ", "HIN_AU.wav")
                                  ("ं", "HIN_AM.wav")
                                  ("ः", "HIN_AHA.wav")

                                  ("क", "HIN_KA.wav")
                                  ("ख", "HIN_KHA.wav")
                                  ("ग", "HIN_GA.wav")
                                  ("घ", "HIN_GHA.wav")
                                  ("ङ", "HIN_NYA.wav")

                                  ("च", "HIN_CHA.wav")
                                  ("छ", "HIN_CHHA.wav")
                                  ("ज", "HIN_JA.wav")
                                  ("झ", "HIN_JHA.wav")
                                  ("ञ", "HIN_NYAA.wav")

                                  ("ट", "HIN_TTA.wav")
                                  ("ठ", "HIN_TTHA.wav")
                                  ("ड", "HIN_DDA.wav")
                                  ("ढ", "HIN_DDHA.wav")
                                  ("ण", "HIN_GNA.wav")

                                  ("त", "HIN_TA.wav")
                                  ("थ", "HIN_THA.wav")
                                  ("द", "HIN_DA.wav")
                                  ("ध", "HIN_DHA.wav")
                                  ("न", "HIN_NA.wav")

                                  ("प", "HIN_PA.wav")
                                  ("फ", "HIN_PHA.wav")
                                  ("ब", "HIN_BA.wav")
                                  ("भ", "HIN_BHA.wav")
                                  ("म", "HIN_MA.wav")

                                  ("य", "HIN_YA.wav")
                                  ("र", "HIN_RA.wav")
                                  ("ल", "HIN_LA.wav")
                                  ("व", "HIN_VA.wav")

                                  ("श", "HIN_SHA.wav")
                                  ("ष", "HIN_SHHA.wav")
                                  ("स", "HIN_SA.wav")
                                  ("ह", "HIN_HA.wav")
                                  
                                  ("ळ", "HIN_DLA.wav")
                                  ("V", "HIN_KSHA.wav"); // done as roman b/c of unicode problems...

void Hindi2SoundsUtil::sayNumber(const Voice &v, int number, bool flip) const
{ 
  /* ENGLISH NUMBERS FOR NOW */
  char sound[] = "?.wav";
  flip = false; // temp
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



void Hindi2SoundsUtil::sayLetter(const Voice &v, const std::string& letter) const
{
	
  if( letter.size() > 3 ) //Because a single devanagari letter is 3 bytes in UTF8
  {
    std::cerr << "Ignoring. sayLetter() was expecting a single letter, but received more." << std::endl;
    return;
  }
  printf("size alphabet is %d is %d\n", Hindi2_letter_map.size(), letter.size());
  
  Hindi2LettersToSoundFilesMap::const_iterator iter = Hindi2_letter_map.find(letter);
  
  if( iter == Hindi2_letter_map.end() ) //this arabic letter does not have a corresponding mapping file (or maybe its not a valid arabic letter to begin with hence it doesnt have a mapping file)
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

void Hindi2SoundsUtil::sayLetterSequence(const Voice &v, const std::string& word) const
{
	std::string::const_iterator iter = word.begin();
  while( iter != word.end() )
  {
    std::string hindi_letter(iter, iter + 3); //All hindi utf chars are 3 bytes
    sayLetter(v, hindi_letter);
    iter = iter + 3;
  }
}

void Hindi2SoundsUtil::sayDotSequence(const Voice &v, DotSequence d) const
{
  
   bool sortedSequence[6] = {false,false,false,false,false,false};
  
  try
  {
     // save activated dots so we can speak them in proper order later.
             
    // this goes from bottom to top, starting on left column then right column. ie dots 321654
    for(int i = 5; i >= 0; i--) {
      if( d & BrailleTutorNS::dot_mask(i) )
      {
        /*
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
        */
        sortedSequence[i] = true;
       } 
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

void Hindi2SoundsUtil::saySound(const Voice& v, const std::string& sound) const
{
	
  std::string sound_file(sound);
  sound_file.append(".wav");
  v.say(sound_file);
}
