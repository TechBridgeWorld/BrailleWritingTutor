/*
 * learn_letters.cc
 *
 *  Created on: Dec 3, 2008
 *      Author: imran
 */

#include <boost/assign/list_of.hpp>
#include "learn_letters.h"

 static time_t time_last_pressed = time(0); 
 static int last_button_pressed = 0;

//ugly constructor, sorry (I am trying to change as little of the old code as possible)
LearnLetters::LearnLetters(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, const std::vector<std::string> my_alph, const std::vector<
    std::string> g0, const std::vector<std::string> g1, const std::vector<std::string> g2, const std::vector<std::string> g3, const std::vector<
    std::string> g4, bool f) :
  IBTApp(my_iep, path_to_mapping_file), su(my_su), alphabet(my_alph), group0(g0), group1(g1), group2(g2), group3(g3), group4(g4), target_group(0),
      target_index(0), target_sequence(0), current_sequence(0), letter_skill(alphabet.size()), nomirror(f)
{

  if( group0.size() + group1.size() + group2.size() + group3.size() + group4.size() != alphabet.size() )
  {
    std::cerr << "Error in LearnLetters: Alphabet groups are not correctly divided. Exiting." << std::endl;
    exit(1);
  }

  su->saySound(getTeacherVoice(), "learn letters");

  for(unsigned int i = 0; i < alphabet.size(); i++)
  {
    letter_skill[i] = KnowledgeTracer(.01) .registerEvent(right, .8, .001) .registerEvent(wrong, .1, .9);
  }

  LL_new();
}

LearnLetters::~LearnLetters()
{
  delete su;
}

void LearnLetters::processEvent(IOEvent& e)
{
  //ignore Button0
  if( e.type == IOEvent::BUTTON_DOWN && e.button == 0 )
    return;

  if( e.type == IOEvent::STYLUS_DOWN || e.type == IOEvent::BUTTON_DOWN )
  {
    if (time(0) != time_last_pressed || last_button_pressed != e.button){
	     printf("pushed button %d\n", e.button);
       last_button_pressed = e.button;
       time_last_pressed = time(0);
       su->sayNumber(getStudentVoice(), getDot(e), nomirror);
        LL_attempt(getDot(e));
    }
    else{
      //iep.flushGlyph(); // would be good to do here?
      printf("DEBUG ignorning \n");
      return; // ignore
    }
  }
}

float LearnLetters::group_skill(int g)
{
  float gs = 2.0; //starting value larger than possible estimates
  int size = getGroupSize(g);
  for(int i = 0; i < size; i++)
  {
    float i_skill = letter_skill[g * 5 + i].estimate(); //XXX: *5 because we have 5 groups
    gs = (gs < i_skill) ? gs : i_skill; //group skill = lowest letter skill
  }
  return gs;
}

void LearnLetters::LL_new()
{
  static const Charset &charset = IBTApp::getCurrentCharset();

  //choose a new sequence target:
  std::vector<int> choices;
  current_sequence = 0;
  target_sequence = 0;

  target_group = (group_skill(0) < .9 ? 0 : group_skill(1) < .9 ? 1 : group_skill(2) < .9 ? 2 : group_skill(3) < .9 ? 3 : group_skill(4) < .9 ? 4
      : rand() % 5);
  printf("target group is %d\n", target_group);
  //look at each letter in group in turn
  bool teaching_letter = false;
  for(int i = 0; i < getGroupSize(target_group); i++)
  {
    target_index = target_group * 5 + i;//XXX: *5 because we have 5 groups
    if( letter_skill[target_index].estimate() < .1 )
    {
      target_sequence = charset[GlyphMapping((std::string) alphabet[target_index])];
      printf("target sequnce is %d\n", target_sequence);
      std::cout << alphabet[target_index] << std::endl;
      teaching_letter = true;
      break;
    }
    else
    {
      if( letter_skill[target_index].estimate() < .9 )
      {
        choices.push_back(target_index); //otherwise, put it in the choices
      }
    }
  }



  if( !teaching_letter )
  {
    if(choices.empty())
    {
      for(int i = 0; i < alphabet.size(); i++){
        letter_skill[i].reset();
      }
      LL_new();
      return;
    }
    random_shuffle(choices.begin(), choices.end());
    target_index = *choices.begin();
    target_sequence = charset[GlyphMapping(alphabet[target_index])];
  }

  if( teaching_letter )
  {
    su->saySound(getTeacherVoice(), "to write the letter");
  }
  else
  {
    su->saySound(getTeacherVoice(), "please write");
  }

  GlyphMapping g = charset[target_sequence];
  printf("size of mapping is %d\n", sizeof(g));
 // printf("type of mapping is %s\n", (char*) typeof(g));
  su->sayLetter(getTeacherVoice(), (std::string) g);

  if( teaching_letter )
  {
	  std::cout << "LN_new: teaching letter" << std::endl;
    su->saySound(getTeacherVoice(), "press");
    su->sayDotSequence(getTeacherVoice(), target_sequence);
  }

  //target_sequence is the one we want,
  //target_index is its index,
  //and target_group is the group number
}

void LearnLetters::LL_attempt(int i)
{
	std::cout << "LN_attempt" << std::endl;
	
  static const Charset &charset = IBTApp::getCurrentCharset();
  if( my_dot_mask(i) & target_sequence )
  { //dot is in sequence
  
    std::cout << "LN_attempt.  Dot in seq" << std::endl;
    current_sequence |= my_dot_mask(i); //add dot to our sequence so far
    if( current_sequence == target_sequence )
    { //are we done?
      letter_skill[target_index].observe(right);
      //std::cout << nthInAlphabet(target_index) << ": " << letter_skill[target_index].estimate() << std::endl;
      std::cout << group_skill(target_group) << std::endl;
      su->saySound(getTeacherVoice(), "good");
      LL_new();
      return;
    }
  }
  else
  { 
	 std::cout << "LN_attempt.  Dot not in seq" << std::endl; 
	 
	 //dot is not in sequence
    letter_skill[target_index].observe(wrong);
    su->saySound(getTeacherVoice(), "no");

    bool teaching_letter = (letter_skill[target_index].estimate() < .1);

    if( teaching_letter )
    {
      su->saySound(getTeacherVoice(), "to write the letter");
      GlyphMapping g = charset[target_sequence];
      su->sayLetter(getTeacherVoice(), (std::string) g);

      su->saySound(getTeacherVoice(), "press");
      su->sayDotSequence(getTeacherVoice(), target_sequence);
    }

    return;
  }
}

int LearnLetters::getGroupSize(int g)
{
  if( g < 0 || g > 4 )
  {
    std::cerr << "Logic error. groups should be between 0 and 4. Throwing excepting" << std::endl;
    throw g;
  }
  return (g == 0 ? group0.size() : g == 1 ? group1.size() : g == 2 ? group2.size() : g == 3 ? group3.size() : group4.size());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishLearnLetters::EnglishLearnLetters(IOEventParser& my_iep) :
      LearnLetters(my_iep, "", new EnglishSoundsUtil, createAlphabet(), createGroup0Letters(), createGroup1Letters(), createGroup2Letters(), createGroup3Letters(), createGroup4Letters(), false)
{

}

const std::vector<std::string> EnglishLearnLetters::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}
const std::vector<std::string> EnglishLearnLetters::createGroup0Letters() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E");
}

const std::vector<std::string> EnglishLearnLetters::createGroup1Letters() const
{
  return boost::assign::list_of("F")("G")("H")("I")("J");
}

const std::vector<std::string> EnglishLearnLetters::createGroup2Letters() const
{
  return boost::assign::list_of("K")("L")("M")("N")("O");
}

const std::vector<std::string> EnglishLearnLetters::createGroup3Letters() const
{
  return boost::assign::list_of("P")("Q")("R")("S")("T");
}

const std::vector<std::string> EnglishLearnLetters::createGroup4Letters() const
{
  return boost::assign::list_of("U")("V")("W")("X")("Y")("Z");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ArabicLearnLetters::ArabicLearnLetters(IOEventParser& my_iep) :
      LearnLetters(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, createAlphabet(), createGroup0Letters(), createGroup1Letters(), createGroup2Letters(), createGroup3Letters(), createGroup4Letters(), false)
{

}

const std::vector<std::string> ArabicLearnLetters::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

const std::vector<std::string> ArabicLearnLetters::createGroup0Letters() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح");
}

const std::vector<std::string> ArabicLearnLetters::createGroup1Letters() const
{
  return boost::assign::list_of("خ")("د")("ذ")("ر")("ز")("س");
}

const std::vector<std::string> ArabicLearnLetters::createGroup2Letters() const
{
  return boost::assign::list_of("ش")("ص")("ض")("ط")("ظ")("ع");
}

const std::vector<std::string> ArabicLearnLetters::createGroup3Letters() const
{
  return boost::assign::list_of("غ")("ف")("ق")("ك")("ل")("م");
}

const std::vector<std::string> ArabicLearnLetters::createGroup4Letters() const
{
  return boost::assign::list_of("ن")("ه")("و")("ي");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FrenchLearnLetters::FrenchLearnLetters(IOEventParser& my_iep) :
      LearnLetters(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, createAlphabet(), createGroup0Letters(), createGroup1Letters(), createGroup2Letters(), createGroup3Letters(), createGroup4Letters(), false)
{

}

const std::vector<std::string> FrenchLearnLetters::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z")("Ç")("É")("À")("È")("Ù")("Â")("Ê")("Î")("Ô")("Û")("Ë")("Ï")("Ü")("œ");
}

const std::vector<std::string> FrenchLearnLetters::createGroup0Letters() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H");
}

const std::vector<std::string> FrenchLearnLetters::createGroup1Letters() const
{
  return boost::assign::list_of("I")("J")("K")("L")("M")("N")("O")("P");
}

const std::vector<std::string> FrenchLearnLetters::createGroup2Letters() const
{
  return boost::assign::list_of("Q")("R")("S")("T")("U")("V")("W")("X")("Y");
}

const std::vector<std::string> FrenchLearnLetters::createGroup3Letters() const
{
  return boost::assign::list_of("Z")("Ç")("É")("À")("È")("Ù")("Â")("Ê");
}

const std::vector<std::string> FrenchLearnLetters::createGroup4Letters() const
{
  return boost::assign::list_of("Î")("Ô")("Û")("Ë")("Ï")("Ü")("œ");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliLearnLetters::KiswahiliLearnLetters(IOEventParser& my_iep) :
      LearnLetters(my_iep, "", new KiswahiliSoundsUtil, createAlphabet(), createGroup0Letters(), createGroup1Letters(), createGroup2Letters(), createGroup3Letters(), createGroup4Letters(), false)
{

}

const std::vector<std::string> KiswahiliLearnLetters::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("R")("S")("T")("U")("W")("Y")("Z");
}
const std::vector<std::string> KiswahiliLearnLetters::createGroup0Letters() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E");
}

const std::vector<std::string> KiswahiliLearnLetters::createGroup1Letters() const
{
  return boost::assign::list_of("F")("G")("H")("I")("J");
}

const std::vector<std::string> KiswahiliLearnLetters::createGroup2Letters() const
{
  return boost::assign::list_of("K")("L")("M")("N")("O");
}

const std::vector<std::string> KiswahiliLearnLetters::createGroup3Letters() const
{
  return boost::assign::list_of("P")("R")("S")("T");
}

const std::vector<std::string> KiswahiliLearnLetters::createGroup4Letters() const
{
  return boost::assign::list_of("U")("W")("Y")("Z");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
English2LearnLetters::English2LearnLetters(IOEventParser& my_iep) :
      LearnLetters(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, createAlphabet(), createGroup0Letters(), createGroup1Letters(), createGroup2Letters(), createGroup3Letters(), createGroup4Letters(), true)
{

}

const std::vector<std::string> English2LearnLetters::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}
const std::vector<std::string> English2LearnLetters::createGroup0Letters() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E");
}

const std::vector<std::string> English2LearnLetters::createGroup1Letters() const
{
  return boost::assign::list_of("F")("G")("H")("I")("J");
}

const std::vector<std::string> English2LearnLetters::createGroup2Letters() const
{
  return boost::assign::list_of("K")("L")("M")("N")("O");
}

const std::vector<std::string> English2LearnLetters::createGroup3Letters() const
{
  return boost::assign::list_of("P")("Q")("R")("S")("T");
}

const std::vector<std::string> English2LearnLetters::createGroup4Letters() const
{
  return boost::assign::list_of("U")("V")("W")("X")("Y")("Z");
}

//+++++++++++++++++++++++++++++++mir+++++++++++++++++++++++++++++++++++++
Arabic2LearnLetters::Arabic2LearnLetters(IOEventParser& my_iep) :
      LearnLetters(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, createAlphabet(), createGroup0Letters(), createGroup1Letters(), createGroup2Letters(), createGroup3Letters(), createGroup4Letters(), true)
{

}

const std::vector<std::string> Arabic2LearnLetters::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

const std::vector<std::string> Arabic2LearnLetters::createGroup0Letters() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح");
}

const std::vector<std::string> Arabic2LearnLetters::createGroup1Letters() const
{
  return boost::assign::list_of("خ")("د")("ذ")("ر")("ز")("س");
}

const std::vector<std::string> Arabic2LearnLetters::createGroup2Letters() const
{
  return boost::assign::list_of("ش")("ص")("ض")("ط")("ظ")("ع");
}

const std::vector<std::string> Arabic2LearnLetters::createGroup3Letters() const
{
  return boost::assign::list_of("غ")("ف")("ق")("ك")("ل")("م");
}

const std::vector<std::string> Arabic2LearnLetters::createGroup4Letters() const
{
  return boost::assign::list_of("ن")("ه")("و")("ي");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Hindi2LearnLetters::Hindi2LearnLetters(IOEventParser& my_iep) :
      LearnLetters(my_iep, "./language_mapping_files/devanagari_mapping.txt", new Hindi2SoundsUtil, createAlphabet(), createGroup0Letters(), createGroup1Letters(), createGroup2Letters(), createGroup3Letters(), createGroup4Letters(), true)
{

}

const std::vector<std::string> Hindi2LearnLetters::createAlphabet() const
{
  return boost::assign::list_of ("अ")
                                  ("आ")
                                  ("इ")
                                  ("ई")
                                  ("उ")
                                  ("ऊ")
                                  ("ए")
                                  ("ऐ")
                                  ("ओ")
                                  ("औ")
                                  ("क")
                                  ("ख")
                                  ("ग")
                                  ("घ")
                                  ("च")
                                  ("छ")
                                  ("ज")
                                  ("झ")
                                  ("ट")
                                  ("ठ")
                                  ("ड")
                                  ("ढ")
                                  ("ण")
                                  ("त")
                                  ("थ")
                                  ("द")
                                  ("ध")
                                  ("न")
                                  ("प")
                                  ("फ")
                                  ("ब")
                                  ("भ")
                                  ("म")
                                  ("य")
                                  ("र")
                                  ("ल")
                                  ("व")
                                  ("श")
                                  ("ष")
                                  ("स")
                                  ("ह");
}

const std::vector<std::string> Hindi2LearnLetters::createGroup0Letters() const
{
  return boost::assign::list_of ("अ")("आ")("इ")("ई")("उ")("ऊ")("ए")("ऐ");
  
}

const std::vector<std::string> Hindi2LearnLetters::createGroup1Letters() const
{
  return boost::assign::list_of ("ओ")("औ")("क")("ख")("ग")("घ")("च")("छ");
}

const std::vector<std::string> Hindi2LearnLetters::createGroup2Letters() const
{
  return boost::assign::list_of ("ज")("झ")("ट")("ठ")("ड")("ढ")("ण")("त")("थ");
} //31

const std::vector<std::string> Hindi2LearnLetters::createGroup3Letters() const
{
  return boost::assign::list_of ("द")("ध")("न")("प")("फ")("ब")("भ")("म")("य")("र")   ;
}
//
const std::vector<std::string> Hindi2LearnLetters::createGroup4Letters() const
{
  return boost::assign::list_of  ("ल")("व")("श")("ष")("स")("ह");
}
//43