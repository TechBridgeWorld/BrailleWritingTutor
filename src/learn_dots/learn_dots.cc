/*
 * learn_dots.cc
 *
 *  Created on: Dec 1, 2008
 *      Author: imran
 */

#include "learn_dots.h"

const int LearnDots::sequence[][6] = { { 1, 2, 3, 4, 5, 6 }, { 1, 3, 4, 6, 2, 5 }, { 1, 4, 2, 5, 3, 6 }, { 1, 5, 4, 2, 6, 3 }, { 1, 6, 4, 3, 5, 2 } };

LearnDots::~LearnDots()
{
  delete su;
}

LearnDots::LearnDots(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, bool f) :
  IBTApp(my_iep, path_to_mapping_file), su(my_su), target_dot(-1), seq(0), pos(0), nomirror(f)
{
  su->saySound(getTeacherVoice(), "learn dots");

  for(int i = 1; i < 7; i++)
  {
    dot_skill[i] = KnowledgeTracer(.01).registerEvent(right, .9, .167).registerEvent(wrong, .1, .833);
  }
  LD_new();
}

void LearnDots::processEvent(IOEvent& e)
{
  //ignore Button0
  if( e.type == IOEvent::BUTTON_DOWN && e.button == 0 )
    return;

  //In Learn Dots we only care about DOT events... so ignore everything else
  if( e.type == IOEvent::STYLUS_DOWN || e.type == IOEvent::BUTTON_DOWN )
  {
    su->sayNumber(getTeacherVoice(), getDot(e), nomirror);
    LD_attempt(getDot(e));
  }
}

void LearnDots::LD_attempt(int i)
{
  if( i == target_dot )
  {
    dot_skill[target_dot].observe(right);
    std::cout << target_dot << ": " << dot_skill[target_dot].estimate() << std::endl;
    su->saySound(getTeacherVoice(), "good");
    LD_new();
  }
  else
  {
    dot_skill[target_dot].observe(wrong);
    std::cout << target_dot << ": " << dot_skill[target_dot].estimate() << std::endl;
    su->saySound(getTeacherVoice(), "no");
    su->saySound(getTeacherVoice(), "find dot");
    su->sayNumber(getTeacherVoice(), target_dot,nomirror);
  }
}

void LearnDots::LD_new()
{

  //target cells under threshold
  std::vector<int> low_skills;
  for(int i = 1; i < 7; i++)
  {
    if( dot_skill[i].estimate() < .9 && target_dot != i )
    {
      low_skills.push_back(i);
    }
  }

  if( low_skills.empty() )
  {
    //sequences...
    target_dot = sequence[seq][pos];
    pos = (pos + 1) % 6;
    if( pos == 0 )
      seq = (seq + 1) % 5;
  }
  else
  {
    //choose a low skill to train
    random_shuffle(low_skills.begin(), low_skills.end());
    target_dot = low_skills.front();
  }
  su->saySound(getTeacherVoice(), "find dot");
  su->sayNumber(getTeacherVoice(), target_dot, nomirror);
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishLearnDots::EnglishLearnDots(IOEventParser& my_iep) :
  LearnDots(my_iep, "", new EnglishSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ArabicLearnDots::ArabicLearnDots(IOEventParser& my_iep) :
  LearnDots(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FrenchLearnDots::FrenchLearnDots(IOEventParser& my_iep) :
  LearnDots(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliLearnDots::KiswahiliLearnDots(IOEventParser& my_iep) :
  LearnDots(my_iep, "", new KiswahiliSoundsUtil, false)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
English2LearnDots::English2LearnDots(IOEventParser& my_iep) :
  LearnDots(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, true)
{

}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Arabic2LearnDots::Arabic2LearnDots(IOEventParser& my_iep) :
  LearnDots(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, true)
{

}
