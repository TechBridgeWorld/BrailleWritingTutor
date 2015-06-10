/*
 * french_utils.cc
 *
 *  Created on: Dec 30, 2008
 *      Author: imran
 */

#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <map>
#include "language_utils.h"
#include "utilities.h"
#include "Dots.h" //for dot_mask()
const FrenchSoundsUtil::FrenchLettersToSoundFilesMap
    FrenchSoundsUtil::french_letter_map =
        boost::assign::map_list_of("A", "A_french.wav")("B", "B_french.wav")("C", "C_french.wav")("D", "D_french.wav")("E", "E_french.wav")("F", "F_french.wav")("G", "G_french.wav")("H", "H_french.wav")("I", "I_french.wav")("J", "J_french.wav")("K", "K_french.wav")("L", "L_french.wav")("M", "M_french.wav")("N", "N_french.wav")("O", "O_french.wav")("P", "P_french.wav")("Q", "Q_french.wav")("R", "R_french.wav")("S", "S_french.wav")("T", "T_french.wav")("U", "U_french.wav")("V", "V_french.wav")("W", "W_french.wav")("X", "X_french.wav")("Y", "Y_french.wav")("Z", "Z_french.wav")("Ç", "C_tail_french.wav")("É", "E_slant_french.wav")("À", "A_slant_french.wav")("È", "E_slant_right_french.wav")("Ù", "U_slant_french.wav")("Â", "A_hat_french.wav")("Ê", "E_hat_french.wav")("Î", "I_hat_french.wav")("Ô", "O_hat_french.wav")("Û", "U_hat_french.wav")("Ë", "E_dots_french.wav")("Ï", "I_dots_french.wav")("Ü", "U_dots_french.wav")("œ", "OE_french.wav");

void FrenchSoundsUtil::sayNumber(const Voice &v, int number, bool f) const
{
  char sound[] = "?_french.wav";
  sprintf(sound, "%d_french.wav", number);
  v.say(sound);
}

void FrenchSoundsUtil::sayLetter(const Voice &v, const std::string& letter) const
{
  if( letter.size() > 2 ) //because UTF* french letters are 1 or 2 bytes FIXME:Should use more robust checking
  {
    std::cerr << "Ignoring. sayLetter() was expecting a single letter, but received more." << std::endl;
    return;
  }

  FrenchLettersToSoundFilesMap::const_iterator iter = french_letter_map.find(letter);
  if( iter == french_letter_map.end() ) //this french letter does not have a corresponding mapping file (or maybe its not a valid french letter to begin with hence it doesnt have a mapping file)
  {
    std::cerr << "Ignoring. Mapping does not exist for:" << letter << std::endl;
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

void FrenchSoundsUtil::sayLetterSequence(const Voice &v, const std::string& word) const
{
  std::string::const_iterator iter = word.begin();
  while( iter != word.end() )
  {
    size_t num_bytes_in_letter = numBytesInUTF8Letter(*iter);//Check how many bytes the letter occupies (incase its multibyte letter)
    std::string french_letter(iter, iter + num_bytes_in_letter);
    sayLetter(v, french_letter);
    iter = iter + num_bytes_in_letter;
  }
}

void FrenchSoundsUtil::sayDotSequence(const Voice &v, DotSequence d) const
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

void FrenchSoundsUtil::saySound(const Voice& v, const std::string& sound) const
{
  std::string sound_file(sound);
  sound_file.append("_french.wav");
  v.say(sound_file);
}
