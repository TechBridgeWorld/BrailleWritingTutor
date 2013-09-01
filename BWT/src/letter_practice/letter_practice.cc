/*
 * letter_practice.cc
 *
 *  Created on: Dec 3, 2008
 *      Author: imran
 */
#include <boost/assign/list_of.hpp>
#include "letter_practice.h"

LetterPractice::LetterPractice(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, const std::vector<std::string> my_alph, const std::vector<
    std::string> sw, const std::vector<std::string> mw, const std::vector<std::string> lw, bool f) :
  IBTApp(my_iep, path_to_mapping_file), su(my_su), iep(my_iep), alphabet(my_alph), short_words(sw), med_words(mw), long_words(lw),
      letter_skill(alphabet.size()), word(""), target_letter("\0"), word_pos(0), word_length(0), firsttime(true), nomirror(f)
{
  su->saySound(getTeacherVoice(), "letter practice");

  for(unsigned int i = 0; i < alphabet.size(); i++)
  {
    letter_skill[i] = KnowledgeTracer(.9) //think they know the letters
        .registerEvent(right, .8, .01) .registerEvent(wrong, .2, .833);
  }

  for(int i = 2; i < 5; i++)
  {
    LS_length_skill[i] = KnowledgeTracer(.01) .registerEvent(right, .7, .1) .registerEvent(wrong, .1, .7);
  }

  PL_new();
}

LetterPractice::~LetterPractice()
{
  delete su;
}

void LetterPractice::processEvent(IOEvent& e)
{
  //Whenever the user hits Button0 we immediately want the LETTER event to be generated so that he doesnt have to wait for the timeout
  if( e.type == IOEvent::BUTTON && e.button == 0 )
  {
    iep.flushGlyph();
    return; //required? hmm..
  }

  if( e.type == IOEvent::CELL_LETTER || e.type == IOEvent::BUTTON_LETTER )
  {
    printEvent(e);
    //Upon entering this mode, we dont want any pending LETTER events to interfere. So we skip the first LETTER event.
    if( firsttime )//Check if this is the first letter event, if so, we skip it
    {
      std::cout << "    (DEBUG)Skipping first letter event" << std::endl;
      firsttime = false;
      return;//skip
    }
    else
    {
      su->sayLetter(getTeacherVoice(), (std::string) e.letter);
      PL_attempt((std::string) e.letter);
    }
  }
}

void LetterPractice::PL_attempt(std::string i)
{
  static const Charset &charset = IBTApp::getCurrentCharset();

  if( target_letter.compare("\0") != 0 )
  { //if we were re-hashing a letter skill
    if( target_letter.compare((std::string) i) == 0 )
    { //match!
      int index = std::distance(alphabet.begin(), std::find(alphabet.begin(), alphabet.end(), target_letter));//finding the index of the target_letter
      //std::cout << "		(DEBUG)1alphabet at index is:" << alphabet.at(index) << std::endl;
      letter_skill[index].observe(right);
      std::cout << target_letter << ": " << letter_skill[index].estimate() << std::endl;
      su->saySound(getTeacherVoice(), "good");
      PL_new();
      return;
    }
    else
    {
      int index = std::distance(alphabet.begin(), std::find(alphabet.begin(), alphabet.end(), target_letter));//finding the index of the target_letter
      //std::cout << "    (DEBUG)2alphabet at index is:" << alphabet.at(index) << std::endl;
      letter_skill[index].observe(wrong);
      std::cout << target_letter << ": " << letter_skill[index].estimate() << std::endl;
      su->saySound(getTeacherVoice(), "no");

      bool teaching_letter = (letter_skill[index].estimate() < .1);

      if( teaching_letter )
      {
        //std::cout << "		(DEBUG)Inside teaching_letter" << std::endl;
        su->saySound(getTeacherVoice(), "to write the letter");
        su->sayLetter(getTeacherVoice(), target_letter);
        su->saySound(getTeacherVoice(), "press");
        su->sayDotSequence(getTeacherVoice(), charset[GlyphMapping(target_letter)]);
      }

      su->saySound(getTeacherVoice(), "please write");
      su->sayLetter(getTeacherVoice(), target_letter);
      return;
    }
  }
  else
  { //if we were testing a sequence
    int num_bytes_in_letter = numBytesInUTF8Letter(word.at(word_pos));
    std::string correct_letter(word, word_pos, num_bytes_in_letter);
    int index = std::distance(alphabet.begin(), std::find(alphabet.begin(), alphabet.end(), correct_letter));
    if( 0 )
    {
      std::cout << "    (DEBUG)utf8 encoded size is:" << num_bytes_in_letter << "  word.size is:" << word.size() << "  word length is:"
          << word_length << std::endl;
      std::cout << "    (DEBUG)Looking for letter:" << correct_letter << std::endl;
      std::cout << "		(DEBUG)index of letter in alphabet array is:" << index << std::endl;
      std::cout << "    (DEBUG)Alphabet at index is:" << alphabet.at(index) << std::endl;
    }
    if( i.compare(correct_letter) == 0 )
    { //letter is next in sequence
      word_pos = word_pos + num_bytes_in_letter; //move to next letter/character
      letter_skill[index].observe(right);
      std::cout << i << ": " << letter_skill[index].estimate() << std::endl;
      if( word_pos == word.size() )
      { //are we done?
        LS_length_skill[word_length].observe(right);
        std::cout << word_length << ": " << LS_length_skill[word_length].estimate() << std::endl;
        su->saySound(getTeacherVoice(), "good");
        PL_new();
        return;
      }
    }
    else
    { //letter is incorrect
      su->saySound(getTeacherVoice(), "no");
      letter_skill[index].observe(wrong);
      LS_length_skill[word_length].observe(wrong);
      std::cout << correct_letter << ": " << letter_skill[index].estimate() << std::endl;
      if( letter_skill[index].estimate() < .1 )
      {
        su->saySound(getTeacherVoice(), "to write the letter");
        su->sayLetter(getTeacherVoice(), correct_letter);
        su->saySound(getTeacherVoice(), "press");
        su->sayDotSequence(getTeacherVoice(), charset[GlyphMapping(correct_letter)]);
        target_letter = correct_letter;
      }
      return;
    }
  }
}

void LetterPractice::PL_new()
{
  static const Charset &charset = IBTApp::getCurrentCharset();

  //check if we've lost any letter skills
  std::vector<int> low_letters;
  for(unsigned int i = 0; i < alphabet.size(); i++)
  {
    if( letter_skill[i].estimate() < .5 )
    {
      low_letters.push_back(i);
    }
  }

  //Check if they need re-training on any letter
  if( !low_letters.empty() )
  {
    //choose a letter skill to train
    random_shuffle(low_letters.begin(), low_letters.end());
    target_letter = alphabet.at(low_letters.front());
    //std::cout << "		(DEBUG)Bad at letter:" << target_letter << std::endl;
    su->saySound(getTeacherVoice(), "to write the letter");
    su->sayLetter(getTeacherVoice(), target_letter);
    su->saySound(getTeacherVoice(), "press");
    su->sayDotSequence(getTeacherVoice(), charset[GlyphMapping(target_letter)]);

    su->saySound(getTeacherVoice(), "please write");
    su->sayLetter(getTeacherVoice(), target_letter);
    return;
  }
  else
  {
    //no letter skill to be trained
    target_letter = "\0";
    //choose a new letter target:
    std::vector<std::string> choices;

    bool need_short = (LS_length_skill[2].estimate() < .9);
    bool need_med = (LS_length_skill[3].estimate() < .9);
    bool need_long = (LS_length_skill[4].estimate() < .9);

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

    if( need_short )
    {
      for(unsigned int i = 0; i < short_words.size(); i++)
      {
        choices.push_back(short_words[i]);
      }
      word_length = 2;
    }
    else if( need_med )
    {
      for(unsigned int i = 0; i < med_words.size(); i++)
      {
        choices.push_back(med_words[i]);
      }
      word_length = 3;
    }
    else if( need_long )
    {
      for(unsigned int i = 0; i < long_words.size(); i++)
      {
        choices.push_back(long_words[i]);
      }
      word_length = 4;
    }

    random_shuffle(choices.begin(), choices.end());
    word = choices.front();
    word_pos = 0;
    su->saySound(getTeacherVoice(), "please write");
    su->sayLetterSequence(getTeacherVoice(), word);
    //now target_dot is -1,
    //target_sequence is the one we want,
    //and target_length is the length of target_sequence
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EnglishLetterPractice::EnglishLetterPractice(IOEventParser& my_iep) :
  LetterPractice(my_iep, "", new EnglishSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), false)
{

}

const std::vector<std::string> EnglishLetterPractice::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

const std::vector<std::string> EnglishLetterPractice::createShortWords() const
{
  return boost::assign::list_of("AT")("BE")("IF")("AN")("TO")("ON");
}

const std::vector<std::string> EnglishLetterPractice::createMedWords() const
{
  return boost::assign::list_of("CAT")("DOG")("HOW")("LET")("MOM")("COD")("SET")("SUD")("YES")("ZOO");
}

const std::vector<std::string> EnglishLetterPractice::createLongWords() const
{
  return boost::assign::list_of("NOTE")("PIES")("QUIT")("REAL")("TIME")("VIEW");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ArabicLetterPractice::ArabicLetterPractice(IOEventParser& my_iep) :
  LetterPractice(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), false)
{

}

const std::vector<std::string> ArabicLetterPractice::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

//2 letter words
const std::vector<std::string> ArabicLetterPractice::createShortWords() const
{
  //return boost::assign::list_of("اح")("بس")("تج")("شص");
  return boost::assign::list_of("قف")("دب")("قط")("جد");
}

//3 letter words
const std::vector<std::string> ArabicLetterPractice::createMedWords() const
{
  //return boost::assign::list_of("تخص")("سشح")("اضذ");
  return boost::assign::list_of("بيت")("وسخ")("صعب");
}

//4 letter words
const std::vector<std::string> ArabicLetterPractice::createLongWords() const
{
  //return boost::assign::list_of("ابتث")("جحخد");
  return boost::assign::list_of("عظيم")("مياه")("خارج")("زجاج");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FrenchLetterPractice::FrenchLetterPractice(IOEventParser& my_iep) :
  LetterPractice(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), false)
{

}

const std::vector<std::string> FrenchLetterPractice::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z")("Ç")("É")("À")("È")("Ù")("Â")("Ê")("Î")("Ô")("Û")("Ë")("Ï")("Ü")("œ");
}

const std::vector<std::string> FrenchLetterPractice::createShortWords() const
{
  return boost::assign::list_of("OÙ")("OU")("SI")("UN")("EN")("ÇA");
}

const std::vector<std::string> FrenchLetterPractice::createMedWords() const
{
  return boost::assign::list_of("NON")("UNE")("PAS")("PEU")("OUI")("AMI")("ÉTÉ")("QUI")("SUR")("VIE");
}

const std::vector<std::string> FrenchLetterPractice::createLongWords() const
{
  return boost::assign::list_of("CHAT")("SOUS")("MÈRE")("RIEN")("ÊTRE");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KiswahiliLetterPractice::KiswahiliLetterPractice(IOEventParser& my_iep) :
  LetterPractice(my_iep, "", new KiswahiliSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), false)
{

}

const std::vector<std::string> KiswahiliLetterPractice::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("R")("S")("T")("U")("W")("Y")("Z");
}

const std::vector<std::string> KiswahiliLetterPractice::createShortWords() const
{
  return boost::assign::list_of("AT")("BE")("IF")("AN")("TO")("ON");
}

const std::vector<std::string> KiswahiliLetterPractice::createMedWords() const
{
  return boost::assign::list_of("CAT")("DOG")("HOW")("LET")("MOM")("COD")("SET")("SUD")("YES")("ZOO");
}

const std::vector<std::string> KiswahiliLetterPractice::createLongWords() const
{
  return boost::assign::list_of("NOTE")("PIES")("QUIT")("REAL")("TIME")("VIEW");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

English2LetterPractice::English2LetterPractice(IOEventParser& my_iep) :
  LetterPractice(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), true)
{

}

const std::vector<std::string> English2LetterPractice::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

const std::vector<std::string> English2LetterPractice::createShortWords() const
{
  return boost::assign::list_of("AT")("BE")("IF")("AN")("TO")("ON");
}

const std::vector<std::string> English2LetterPractice::createMedWords() const
{
  return boost::assign::list_of("CAT")("DOG")("HOW")("LET")("MOM")("COD")("SET")("SUD")("YES")("ZOO");
}

const std::vector<std::string> English2LetterPractice::createLongWords() const
{
  return boost::assign::list_of("NOTE")("PIES")("QUIT")("REAL")("TIME")("VIEW");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Arabic2LetterPractice::Arabic2LetterPractice(IOEventParser& my_iep) :
  LetterPractice(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), true)
{

}

const std::vector<std::string> Arabic2LetterPractice::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

//2 letter words
const std::vector<std::string> Arabic2LetterPractice::createShortWords() const
{
  //return boost::assign::list_of("اح")("بس")("تج")("شص");
  return boost::assign::list_of("قف")("دب")("قط")("جد");
}

//3 letter words
const std::vector<std::string> Arabic2LetterPractice::createMedWords() const
{
  //return boost::assign::list_of("تخص")("سشح")("اضذ");
  return boost::assign::list_of("بيت")("وسخ")("صعب");
}

//4 letter words
const std::vector<std::string> Arabic2LetterPractice::createLongWords() const
{
  //return boost::assign::list_of("ابتث")("جحخد");
  return boost::assign::list_of("عظيم")("مياه")("خارج")("زجاج");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
