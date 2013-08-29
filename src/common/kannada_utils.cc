/*
 * kannada_utils.cc
 *
 *  Created July 2013
 *      Author: madeleine clute
 */

#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <map>
#include "language_utils.h"
#include "Dots.h" //for dot_mask()

const KannadaSoundsUtil::KannadaLettersToSoundFilesMap
    KannadaSoundsUtil::Kannada_letter_map =
        boost::assign::map_list_of("ಅ", "KAN_A.wav")
                                  ("ಆ", "KAN_AA.wav")
                                  ("ಆ", "KAN_I.wav")
                                  ("ಈ", "KAN_II.wav") 
                                  ("ಉ", "KAN_U.wav")
                                  ("ಊ", "KAN_UU.wav")
                                  ("ಋ", "KAN_RU.wav")
                                  ("ಎ", "KAN_E.wav")
                                  ("ಏ", "KAN_EE.wav")
                                  ("ಐ", "KAN_AI.wav")
                                  ("ಒ", "KAN_O.wav")
                                  ("ಓ", "KAN_OO.wav")
                                  ("ಔ", "KAN_AU.wav")
                                  ("A", "KAN_AM.wav")
                                  ("B", "KAN_AHA.wav")
                                  ("ಕ", "KAN_KA.wav")
                                  ("ಖ", "KAN_KHA.wav")
                                  ("ಗ", "KAN_GA.wav")
                                  ("ಘ", "KAN_GHA.wav")
                                  ("ಙ", "KAN_NYA.wav")
                                  ("ಚ", "KAN_CHA.wav")
                                  ("ಛ", "KAN_CHHA.wav")
                                  ("ಜ", "KAN_JA.wav")
                                  ("ಝ", "KAN_JHA.wav")
                                  ("ಞ", "KAN_NYAA.wav")
                                  ("ಟ", "KAN_TTA.wav")
                                  ("ಠ", "KAN_TTHA.wav")
                                  ("ಡ", "KAN_DDA.wav")
                                  ("ಢ", "KAN_DDHA.wav")
                                  ("ಣ", "KAN_GNA.wav")
                                  ("ತ", "KAN_TA.wav")
                                  ("ಥ", "KAN_THA.wav")
                                  ("ದ", "KAN_DA.wav")
                                  ("ಧ", "KAN_DHA.wav")
                                  ("ನ", "KAN_NA.wav")
                                  ("ಪ", "KAN_PA.wav")
                                  ("ಫ", "KAN_PHA.wav")
                                  ("ಬ", "KAN_BA.wav")
                                  ("ಭ", "KAN_BHA.wav")
                                  ("ಮ", "KAN_MA.wav")
                                  ("ಯ", "KAN_YA.wav")
                                  ("ರ", "KAN_RA.wav")
                                  ("ಲ", "KAN_LA.wav")
                                  ("ವ", "KAN_VA.wav")
                                  ("ಶ", "KAN_SHA.wav")
                                  ("ಷ", "KAN_SHHA.wav")
                                  ("ಸ", "KAN_SA.wav")
                                  ("ಹ", "KAN_HA.wav")
                                  ("ಳ", "KAN_DLA.wav")
                                  ("C", "KAN_KSHA.wav")
                                  ;

void KannadaSoundsUtil::sayNumber(const Voice &v, int number, bool flip) const
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



void KannadaSoundsUtil::sayLetter(const Voice &v, const std::string& letter) const
{
	/*MIGHT HAVE TO CHANGE THIS */
  if( letter.size() > 3 ) //Because a single devanagari letter is 3 bytes in UTF8
  {
    std::cerr << "Ignoring. sayLetter() was expecting a single letter, but received more." << std::endl;
    //std::cout << "actual size was " << (letter.size()) << std::endl;
    return;
  }
  printf("size alphabet is %d is %d\n", Kannada_letter_map.size(), letter.size());
  
  KannadaLettersToSoundFilesMap::const_iterator iter = Kannada_letter_map.find(letter);
  
  if( iter == Kannada_letter_map.end() ) //this arabic letter does not have a corresponding mapping file (or maybe its not a valid arabic letter to begin with hence it doesnt have a mapping file)
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

void KannadaSoundsUtil::sayLetterSequence(const Voice &v, const std::string& word) const
{
	std::string::const_iterator iter = word.begin();
  while( iter != word.end() )
  {
    std::string hindi_letter(iter, iter + 3); //All hindi utf chars are 3 bytes
    sayLetter(v, hindi_letter);
    iter = iter + 3;
  }
}

void KannadaSoundsUtil::sayDotSequence(const Voice &v, DotSequence d) const
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

void KannadaSoundsUtil::saySound(const Voice& v, const std::string& sound) const
{
	
  std::string sound_file(sound);
  sound_file.append(".wav");
  v.say(sound_file);
}
