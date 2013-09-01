/*
 * dot_practice.cc
 *
 *  Created on: Dec 2, 2008
 *      Author: imran
 */
#include <boost/assign/list_of.hpp>
#include "dot_practice.h"

DotPractice::DotPractice(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, std::vector<std::string> sl, std::vector<
    std::string> ml, std::vector<std::string> ll, bool f) :
  IBTApp(my_iep, path_to_mapping_file), target_dot(-1), su(my_su), short_letters(sl), med_letters(ml), long_letters(ll), nomirror(f)
{
  su->saySound(getTeacherVoice(), "dot practice");

  for(int i = 1; i < 7; i++)
  {
    dot_skill[i] = KnowledgeTracer(.9) //think they know the dots
        .registerEvent(right, .9, .167) .registerEvent(wrong, .1, .833);
  }

  for(int i = 0; i < 3; i++)
  {
    DS_length_skill[i] = KnowledgeTracer(.01) .registerEvent(right, .7, .1) .registerEvent(wrong, .1, .7);
  }

  PD_new();
}

DotPractice::~DotPractice()
{
  delete su;
}

void DotPractice::processEvent(IOEvent& e)
{
  //ignore Button0
  if( e.type == IOEvent::BUTTON_DOWN && e.button == 0 )
    return;

  if( e.type == IOEvent::STYLUS_DOWN || e.type == IOEvent::BUTTON_DOWN )
  {
    su->sayNumber(getStudentVoice(), getDot(e), nomirror);
    PD_attempt(getDot(e));
  }
}

void DotPractice::PD_new()
{
  //std::cout << "		(DEBUG)enter PD_new" << std::endl;
  //check if we've lost any dot skills
  std::vector<int> low_dots;
  for(int i = 1; i < 7; i++)
  {
    if( dot_skill[i].estimate() < .5 )
    {
      low_dots.push_back(i);
    }
  }

  if( !low_dots.empty() )
  {
    //choose a dot skill to train
    random_shuffle(low_dots.begin(), low_dots.end());
    target_dot = low_dots.front();
    su->saySound(getTeacherVoice(), "please press");
    su->sayNumber(getStudentVoice(), target_dot, nomirror);
    return;
  }
  else
  {
    //no dot skill to be trained
    target_dot = -1;
    //choose a new sequence target:
    std::vector<DotSequence> choices;

    bool need_short = (DS_length_skill[0].estimate() < .9);
    bool need_med = (DS_length_skill[1].estimate() < .9);
    bool need_long = (DS_length_skill[2].estimate() < .9);

    if( !(need_short || need_med || need_long) )
    {
      switch( std::rand() % 3 )
      {
        case 0:
          need_short = true;
          break;
        case 1:
          need_med = true;
          break;
        case 2:
          need_long = true;
          break;
      }
    }

    const Charset &charset = IBTApp::getCurrentCharset();
    if( need_short )
    {
      for(unsigned int i = 0; i < short_letters.size(); i++)
      {
        //std::cout << "		(DEBUG)Before short letters" << std::endl;
        GlyphMapping temp(short_letters[i]);
        //std::cout << "		(DEBUG)temp is:" << (std::string) temp << std::endl;
        //std::cout << "		(DEBUG)Charset name is:" << (std::string) charset.getName() << std::endl;
        choices.push_back(charset[temp]);
      }
      target_length = 0;
    }
    else if( need_med )
    {
      for(unsigned int i = 0; i < med_letters.size(); i++)
      {
        //std::cout << "    (DEBUG)Before med letters" << std::endl;
        choices.push_back(charset[GlyphMapping(med_letters[i])]);
      }
      target_length = 1;
    }
    else if( need_long )
    {
      for(unsigned int i = 0; i < long_letters.size(); i++)
      {
        //std::cout << "    (DEBUG)Before long letters" << std::endl;
        choices.push_back(charset[GlyphMapping(long_letters[i])]);
      }
      target_length = 2;
    }

    random_shuffle(choices.begin(), choices.end());
    target_sequence = choices.front();
    current_sequence = 0;
    std::cout << "Target dots (decimal bits): " << (int) target_sequence << std::endl;
    su->saySound(getTeacherVoice(), "please press");
    //std::cout << "    (DEBUG)GOING INSIDE DOTSEQUENCE" << std::endl;
    su->sayDotSequence(getTeacherVoice(), target_sequence);
    //std::cout << "		(DEBUG)END OF ELSE PART" << std::endl;
    //now target_dot is -1,
    //target_sequence is the one we want,
    //and target_length is the length of target_sequence
  }
}

void DotPractice::PD_attempt(int i)
{
  if( target_dot != -1 )
  { //if we were re-hashing a dot skill
    if( i == target_dot )
    {
      dot_skill[target_dot].observe(right);
      std::cout << target_dot << ": " << dot_skill[target_dot].estimate() << std::endl;
      su->saySound(getTeacherVoice(), "good");
      PD_new();
      return;
    }
    else
    {
      dot_skill[target_dot].observe(wrong);
      std::cout << target_dot << ": " << dot_skill[target_dot].estimate() << std::endl;
      su->saySound(getTeacherVoice(), "no");
      su->saySound(getTeacherVoice(), "find dot");
      su->sayNumber(getStudentVoice(), target_dot, nomirror);
      return;
    }
  }
  else
  { //if we were testing a sequence
    if( my_dot_mask(i) & target_sequence )
    { //dot is in sequence
      current_sequence |= my_dot_mask(i); //add dot to our sequence so far
      dot_skill[i].observe(right);
      std::cout << i << ": " << dot_skill[i].estimate() << std::endl;
      if( current_sequence == target_sequence )
      { //are we done?
        DS_length_skill[target_length].observe(right);
        std::cout << target_length << ": " << DS_length_skill[target_length].estimate() << std::endl;
        su->saySound(getTeacherVoice(), "good");
        PD_new();
        return;
      }
    }
    else
    { //dot is not in sequence
      dot_skill[i].observe(wrong);
      DS_length_skill[target_length].observe(wrong);
      su->saySound(getTeacherVoice(), "no");
      return;
    }

  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishDotPractice::EnglishDotPractice(IOEventParser& my_iep) :
  DotPractice(my_iep, "", new EnglishSoundsUtil, createShortLettersVector(), createMediumLettersVector(), createLongLettersVector(), false)
{

}

std::vector<std::string> EnglishDotPractice::createShortLettersVector()
{
  return boost::assign::list_of("A")("B")("C")("E")("I")("K");
}

std::vector<std::string> EnglishDotPractice::createMediumLettersVector()
{
  return boost::assign::list_of("D")("F")("H")("J")("L")("M")("O")("S")("U");
}

std::vector<std::string> EnglishDotPractice::createLongLettersVector()
{
  return boost::assign::list_of("G")("N")("P")("Q")("R")("T")("V")("W")("X")("Y")("Z");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ArabicDotPractice::ArabicDotPractice(IOEventParser& my_iep) :
      DotPractice(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, createShortLettersVector(), createMediumLettersVector(), createLongLettersVector(), false)
{

}

std::vector<std::string> ArabicDotPractice::createShortLettersVector()
{
  return boost::assign::list_of("ا")("ب");
}

std::vector<std::string> ArabicDotPractice::createMediumLettersVector()
{
  return boost::assign::list_of("ج")("ح")("د")("س")("ش");
}

std::vector<std::string> ArabicDotPractice::createLongLettersVector()
{
  return boost::assign::list_of("ث")("ت")("خ")("ذ")("ز")("ص")("ض");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FrenchDotPractice::FrenchDotPractice(IOEventParser& my_iep) :
      DotPractice(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, createShortLettersVector(), createMediumLettersVector(), createLongLettersVector(), false)
{

}

std::vector<std::string> FrenchDotPractice::createShortLettersVector()
{
  return boost::assign::list_of("A")("B")("C")("E")("I")("K")("Â");
}

std::vector<std::string> FrenchDotPractice::createMediumLettersVector()
{
  return boost::assign::list_of("D")("F")("H")("J")("L")("M")("O")("S")("U")("È")("Ê")("Ë")("Î")("Ô")("Û")("Ü");
}

std::vector<std::string> FrenchDotPractice::createLongLettersVector()
{
  return boost::assign::list_of("G")("N")("P")("Q")("R")("T")("V")("W")("X")("Y")("Z")("À")("É")("Ï")("Ù")("Ç");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliDotPractice::KiswahiliDotPractice(IOEventParser& my_iep) :
      DotPractice(my_iep, "", new KiswahiliSoundsUtil, createShortLettersVector(), createMediumLettersVector(), createLongLettersVector(), false)
{

}

std::vector<std::string> KiswahiliDotPractice::createShortLettersVector()
{
  return boost::assign::list_of("A")("B")("C")("E")("I")("K");
}

std::vector<std::string> KiswahiliDotPractice::createMediumLettersVector()
{
  return boost::assign::list_of("D")("F")("H")("J")("L")("M")("O")("S")("U");
}

std::vector<std::string> KiswahiliDotPractice::createLongLettersVector()
{
  return boost::assign::list_of("G")("N")("P")("R")("T")("W")("Y")("Z");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
English2DotPractice::English2DotPractice(IOEventParser& my_iep) :
  DotPractice(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, createShortLettersVector(), createMediumLettersVector(), createLongLettersVector(), true)
{

}

std::vector<std::string> English2DotPractice::createShortLettersVector()
{
  return boost::assign::list_of("A")("B")("C")("E")("I")("K");
}

std::vector<std::string> English2DotPractice::createMediumLettersVector()
{
  return boost::assign::list_of("D")("F")("H")("J")("L")("M")("O")("S")("U");
}

std::vector<std::string> English2DotPractice::createLongLettersVector()
{
  return boost::assign::list_of("G")("N")("P")("Q")("R")("T")("V")("W")("X")("Y")("Z");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Arabic2DotPractice::Arabic2DotPractice(IOEventParser& my_iep) :
      DotPractice(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, createShortLettersVector(), createMediumLettersVector(), createLongLettersVector(), true)
{

}

std::vector<std::string> Arabic2DotPractice::createShortLettersVector()
{
  return boost::assign::list_of("ا")("ب");
}

std::vector<std::string> Arabic2DotPractice::createMediumLettersVector()
{
  return boost::assign::list_of("ج")("ح")("د")("س")("ش");
}

std::vector<std::string> Arabic2DotPractice::createLongLettersVector()
{
  return boost::assign::list_of("ث")("ت")("خ")("ذ")("ز")("ص")("ض");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

