/*
 * hangman.cc
 *
 *  Created on: Dec 9, 2008
 *      Author: Students
 *      Multi-lingual enhancements: Imran
 */

#include <boost/assign/list_of.hpp>
#include "hangman.h"

Hangman::Hangman(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, const std::vector<std::string> my_alph, const std::vector<
    std::string> sw, const std::vector<std::string> mw, const std::vector<std::string> lw, const std::vector<std::string> xlw, const std::vector<
    std::string> xxlw, bool f) :
  IBTApp(my_iep, path_to_mapping_file), iep(my_iep), su(my_su), alphabet(my_alph), short_words(sw), med_words(mw), long_words(lw), xlong_words(xlw),
      xxlong_words(xxlw), letter_skill(alphabet.size()), firsttime(true), turncount(0), mistake(0), correctcount(0), j(0), word(""),
      target_letter(""), word_pos(0), word_length(0), answer(""), nomirror(f)
{
  for(size_t i = 0; i < alphabet.size(); i++)
  {
    letter_skill[i] = KnowledgeTracer(.9) //think they know the letters
        .registerEvent(right, .8, .01) .registerEvent(wrong, .2, .833);
  }

  for(int i = 2; i < 8; i++)
  {
    LS_length_skill[i] = KnowledgeTracer(.01) .registerEvent(right, .7, .1) .registerEvent(wrong, .1, .7);
  }

  HM_new();
}

void Hangman::processEvent(IOEvent& e)
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
      su->sayLetter(getStudentVoice(), (std::string) e.letter);
      HM_attempt((std::string) e.letter);
    }
  }
}

void Hangman::HM_new()
{
  static const Charset &charset = IBTApp::getCurrentCharset();

  //check if we've lost any letter skills
  std::vector<int> low_letters;
  turncount = 0;
  mistake = 0;
  correctcount = 0;
  for(size_t i = 0; i < alphabet.size(); i++)
  {
    if( letter_skill[i].estimate() < .5 )
    {
      low_letters.push_back(i);
    }
  }

  if( !low_letters.empty() )
  {
    //choose a letter skill to train
    random_shuffle(low_letters.begin(), low_letters.end());
    target_letter = alphabet.at(low_letters.front());

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
    int s; //number of dashes

    bool need_med = (LS_length_skill[4].estimate() < .9);
    bool need_long = (LS_length_skill[5].estimate() < .9);
    bool need_xlong = (LS_length_skill[6].estimate() < .9);
    bool need_xxlong = (LS_length_skill[7].estimate() < .9);

    if( !(need_med || need_long || need_xlong || need_xxlong) )
    {
      switch( std::rand() % 4 )
      {
        case 0:
          need_med = true;
          break;
        case 1:
          need_long = true;
          break;
        case 2:
          need_xlong = true;
          break;
        case 3:
          need_xxlong = true;
          break;
      }
    }

    if( need_med )
    {
      for(size_t i = 0; i < med_words.size(); i++)
      {
        choices.push_back(med_words[i]);
      }
      word_length = 4;
    }
    else if( need_long )
    {
      for(size_t i = 0; i < long_words.size(); i++)
      {
        choices.push_back(long_words[i]);
      }
      word_length = 5;
    }
    else if( need_xlong )
    {
      for(size_t i = 0; i < xlong_words.size(); i++)
      {
        choices.push_back(xlong_words[i]);
      }
      word_length = 6;
    }
    else if( need_xxlong )
    {
      for(size_t i = 0; i < xxlong_words.size(); i++)
      {
        choices.push_back(xxlong_words[i]);
      }
      word_length = 7;
    }

    random_shuffle(choices.begin(), choices.end());
    word = choices.front();
    word_pos = 0;

    if( word_length == 4 )
    {
      su->saySound(getTeacherVoice(), "word 4 letters");
    }
    else if( word_length == 5 )
    {
      su->saySound(getTeacherVoice(), "word 5 letters");
    }
    else if( word_length == 6 )
    {
      su->saySound(getTeacherVoice(), "word 6 letters");
    }
    else
    {
      su->saySound(getTeacherVoice(), "word 7 letters");
    }

    for(s = 0; s < word_length; s++)
    {
      su->saySound(getTeacherVoice(), "DASH");
    }
    int num_bytes_in_letter = numBytesInUTF8Letter(word);
    answer.clear();

    //We want answer's size to be same as word's.
    answer.resize(word.size());// filled with null characters.

    //Fill answer with *s, but add these *s every num_bytes_in_letter intervals. To keep consistent with word's
    for(size_t i = 0; i<word.size();)
    {
      answer.replace(i,1,"*");
      i = i + numBytesInUTF8Letter(word.at(i));
    }
    /*for(size_t i = 0; i<word_length*num_bytes_in_letter;i=i+num_bytes_in_letter)//FIXME:we assuming that every character in word will have the same number_of_bytes in UTF8.
    {
      answer.replace(i,1,"*");
    }*/



    //answer = std::string(word_length, '*');//answer is originally all  *'s students need to guess what the *'s are
    su->saySound(getTeacherVoice(), "guess a letter"); //will change to please guess
    std::cout << "Current answer is " << word << std::endl;
    std::cout << "Current guessed word is " << answer <<std::endl;
    std::cout<<"		(DEBUG)Answer size is:"<<answer.size()<<std::endl;
  }
}

void Hangman::HM_attempt(std::string i)
{
  //if we were testing a sequence
  //int index;
  //char temp[2] = "?";
  turncount = 0;
  if( mistake == 7 )
  { //after 7 mistakes students give one chance to guess the word
    //int index = alphabeticIndex(alphabet, word, word_pos, word.at(word_pos));
    int num_bytes_in_letter = numBytesInUTF8Letter(word.at(word_pos));
    std::string correct_letter(word, word_pos, num_bytes_in_letter);
    std::cout << "		(DEBUG)Corr letter:" << correct_letter << " word.at(word_pos)" << correct_letter << std::endl;
    if( i.compare(correct_letter) == 0 )
    { //letter is next in sequence
      word_pos = word_pos + numBytesInUTF8Letter(word.at(word_pos)); //move to next character
      if( word_pos == word.size() )
      { //are we done?
        su->saySound(getTeacherVoice(), "good");
        LS_length_skill[word_length].observe(right);
        std::cout << "WOW you guessed it correctly" << std::endl;
        HM_new();
        return;
      }
    }
    else
    { //guess is incorrect
      su->saySound(getTeacherVoice(), "no");
      su->saySound(getTeacherVoice(), "solution");
      su->sayLetterSequence(getTeacherVoice(), word);
      std::cout << "SORRY you have run out of guesses the correct word was " << word << std::endl;
      HM_new();
      return;

    }
    // mistake guess method ends here
  }
  else
  {
    for(; word_pos < word.size(); word_pos = word_pos + numBytesInUTF8Letter(word.at(word_pos)))
    {
      int index = alphabeticIndex(alphabet, word, word_pos, word.at(word_pos));
      int num_bytes_in_letter = numBytesInUTF8Letter(word.at(word_pos));
      std::string correct_letter(word, word_pos, num_bytes_in_letter);
      if( i.compare(correct_letter) == 0 )
      { //letter is next in sequence
        //std::cout<<"		(DEBUG)here1"<<std::endl;
        if( answer.at(word_pos) != '*' )
        {
          su->saySound(getTeacherVoice(), "same letter");
          std::cout << "You have already guessed that letter please guess again" << std::endl;
          mistake--;
        }
        else
        {
          answer.replace(word_pos, num_bytes_in_letter, correct_letter);
          //std::cout<<"		(DEBUG)num bytes:"<<num_bytes_in_UTF8letter<<" letter is:"<<letter<<" answer is:"<<answer<<" word.at(word_pos) is:"<<word.at(word_pos)<<std::endl;
          turncount++;
          correctcount++;
          //word_pos++; //move to next character
          letter_skill[index].observe(right);
          std::cout << i << ": " << letter_skill[index].estimate() << std::endl;
          std::cout << "    (DEBUG)Picked a good letter!" << std::endl;
          std::cout << "    (DEBUG)word_pos:" << word_pos << " word.at(word_pos):" << correct_letter << " Answer is:"<<answer<<std::endl;
        }
      }
    }
    word_pos = 0;
    if( turncount == 0 )
    {
      mistake++;
      if( mistake == 7 )
      {
        std::cout << "You have run out of guesses but you have one last chance to guess the word." << std::endl;
        std::cout << "Please write your guess" << std::endl;
      }
      su->saySound(getTeacherVoice(), "no");
      std::cout << "Letter guessed is not present please guess another letter" << std::endl;
    }
    if( correctcount == word_length )
    {
      std::cout << "GOODJOB YOU SPELLED " << word << " correctly" << std::endl;
      LS_length_skill[word_length].observe(right);
      std::cout << word_length << ": " << LS_length_skill[word_length].estimate() << std::endl;
      su->saySound(getTeacherVoice(), "good");
      HM_new();
      return;
    }
    su->saySound(getTeacherVoice(), "current");
    for(int s = 0; s < answer.size();)
    {
      int num_bytes_in_letter = numBytesInUTF8Letter(answer.at(s));
      std::string letter(answer, s, num_bytes_in_letter);
      std::cout<<"		(DEBUG)Extracted letter from answer is:"<<letter<<std::endl;
      if( !strcmp(letter.c_str(), "*") )
      {
        su->saySound(getTeacherVoice(), "DASH");
      }
      else
      {
        su->sayLetter(getTeacherVoice(), letter);
      }
      s=s+num_bytes_in_letter;
    }
    if( mistake == 7 )
    {
      su->saySound(getTeacherVoice(), "7 mistakes");
    }
    else
    {
      switch( mistake )
      {
        case 0:
          su->saySound(getTeacherVoice(), "0 mistakes");
          break;
        case 1:
          su->saySound(getTeacherVoice(), "1 mistake");
          break;
        case 2:
          su->saySound(getTeacherVoice(), "2 mistakes");
          break;
        case 3:
          su->saySound(getTeacherVoice(), "3 mistakes");
          break;
        case 4:
          su->saySound(getTeacherVoice(), "4 mistakes");
          break;
        case 5:
          su->saySound(getTeacherVoice(), "5 mistakes");
          break;
        case 6:
          su->saySound(getTeacherVoice(), "6 mistakes");
          break;
      }

      su->saySound(getTeacherVoice(), "guess a letter");
    }
    std::cout << "Current guessed word is " << answer << std::endl;
    std::cout << "    (DEBUG)word_pos:" << word_pos <<" Answer is:"<<answer<<std::endl;
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EnglishHangman::EnglishHangman(IOEventParser& my_iep) :
      Hangman(my_iep, "", new EnglishSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), createxLongWords(), createxxLongWords(), false)
{

}

const std::vector<std::string> EnglishHangman::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

//3 letter words
const std::vector<std::string> EnglishHangman::createShortWords() const
{
  //XXX:Not used in the application code
  return boost::assign::list_of("CAT")("DOG")("HOW")("LET")("MOM")("NOT")("SET")("GET")("YES")("ZOO");
}

//4 letter words
const std::vector<std::string> EnglishHangman::createMedWords() const
{
  return boost::assign::list_of("NOTE")("LION")("QUIT")("REAL")("TIME")("VIEW")("EXAM")("YAWN")("FOUR")("BLUE")("TREE")("FROG")("NOSE")("FAST")("SLOW")("PLAY")("REST")("HOME")("PARK")("SICK")("SING")("BOOK")("JUMP")("STOP")("WALK");
}

//5 letter words
const std::vector<std::string> EnglishHangman::createLongWords() const
{
  return boost::assign::list_of("PIZZA")("APPLE")("BEACH")("INDIA")("SLATE")("MOUSE")("MONEY")("WATER")("GREAT")("GRADE")("GRAND")("STATE")("NIGHT")("DRESS")("CLOCK")("THING")("WATCH")("CLEAN")("CHECK")("SEVEN")("HOUSE")("STORY")("ABOUT")("SOUND")("ROUND");
}

//6 letter words
const std::vector<std::string> EnglishHangman::createxLongWords() const
{
  return boost::assign::list_of("PLEASE")("GARDEN")("YELLOW")("ORANGE")("PURPLE")("FLOWER")("FRIEND")("MATHRU")("GROUND")("PLEASE")("FAMILY")("BORROW")("PUNISH")("ALWAYS")("ACROSS")("SQUARE")("PERSON")("INSIDE")("NUMBER")("INSECT");
}

//7 letter words
const std::vector<std::string> EnglishHangman::createxxLongWords() const
{
  return boost::assign::list_of("STUDENT")("TEACHER")("PRETEND")("JEALOUS")("HUNDRED")("WEATHER")("WHISPER")("MILLION")("CHICKEN")("OCTOPUS");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ArabicHangman::ArabicHangman(IOEventParser& my_iep) :
      Hangman(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), createxLongWords(), createxxLongWords(), false)
{

}

const std::vector<std::string> ArabicHangman::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

//3 letter words
const std::vector<std::string> ArabicHangman::createShortWords() const
{
  //XXX:Not used in the application code
  return boost::assign::list_of("");
}

//4 letter words
const std::vector<std::string> ArabicHangman::createMedWords() const
{
  //return boost::assign::list_of("ضصثق")("شسيب")("دجحخ");
  return boost::assign::list_of("ضفدع")("شجرة")("ضجيج")("سريع")("كتاب")("مريض")("عظيم")("سبعة");
}

//5 letter words
const std::vector<std::string> ArabicHangman::createLongWords() const
{
  //return boost::assign::list_of("طكمنت")("جحخهع");
  return boost::assign::list_of("فوشيا")("طعام")("داخل")("متكبر");
}

//6 letter words
const std::vector<std::string> ArabicHangman::createxLongWords() const
{
  //return boost::assign::list_of("ةىركبت")("ضصهثشسي");
  return boost::assign::list_of("توفاحة")("موسيقى")("معلمتي")("الكحول");
}

//7 letter words
const std::vector<std::string> ArabicHangman::createxxLongWords() const
{
  //return boost::assign::list_of("منتالبي");
  return boost::assign::list_of("باذنجان");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FrenchHangman::FrenchHangman(IOEventParser& my_iep) :
      Hangman(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), createxLongWords(), createxxLongWords(), false)
{

}

const std::vector<std::string> FrenchHangman::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z")("Ç")("É")("À")("È")("Ù")("Â")("Ê")("Î")("Ô")("Û")("Ë")("Ï")("Ü")("œ");
}

//3 letter words
const std::vector<std::string> FrenchHangman::createShortWords() const
{
  //XXX:Not used in the application code
  return boost::assign::list_of("");
}

//4 letter words
const std::vector<std::string> FrenchHangman::createMedWords() const
{
  //return boost::assign::list_of("AœÜB");
  return boost::assign::list_of("FAUX")("BLEU")("CAFÉ");
}

//5 letter words
const std::vector<std::string> FrenchHangman::createLongWords() const
{
  return boost::assign::list_of("MERCI")("ROUGE")("HÔTEL");
}

//6 letter words
const std::vector<std::string> FrenchHangman::createxLongWords() const
{
  return boost::assign::list_of("CHARGÉ")("PETITE")("SUCCÈS");
}

//7 letter words
const std::vector<std::string> FrenchHangman::createxxLongWords() const
{
  return boost::assign::list_of("ATTACHÉ")("BONJOUR")("HUÎTRES");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//TODO: Currently just using the French words. Need to implement with Swahili'ish/English words?

KiswahiliHangman::KiswahiliHangman(IOEventParser& my_iep) :
      Hangman(my_iep, "", new KiswahiliSoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), createxLongWords(), createxxLongWords(), false)
{

}

const std::vector<std::string> KiswahiliHangman::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("R")("S")("T")("U")("W")("Y")("Z");
}

//3 letter words
const std::vector<std::string> KiswahiliHangman::createShortWords() const
{
  //XXX:Not used in the application code
  return boost::assign::list_of("");
}

//4 letter words
const std::vector<std::string> KiswahiliHangman::createMedWords() const
{
  //return boost::assign::list_of("AœÜB");
  return boost::assign::list_of("FAUX")("BLEU")("CAFÉ");
}

//5 letter words
const std::vector<std::string> KiswahiliHangman::createLongWords() const
{
  return boost::assign::list_of("MERCI")("ROUGE")("HÔTEL");
}

//6 letter words
const std::vector<std::string> KiswahiliHangman::createxLongWords() const
{
  return boost::assign::list_of("CHARGÉ")("PETITE")("SUCCÈS");
}

//7 letter words
const std::vector<std::string> KiswahiliHangman::createxxLongWords() const
{
  return boost::assign::list_of("ATTACHÉ")("BONJOUR")("HUÎTRES");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

English2Hangman::English2Hangman(IOEventParser& my_iep) :
      Hangman(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), createxLongWords(), createxxLongWords(), true)
{

}

const std::vector<std::string> English2Hangman::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

//3 letter words
const std::vector<std::string> English2Hangman::createShortWords() const
{
  //XXX:Not used in the application code
  return boost::assign::list_of("CAT")("DOG")("HOW")("LET")("MOM")("NOT")("SET")("GET")("YES")("ZOO");
}

//4 letter words
const std::vector<std::string> English2Hangman::createMedWords() const
{
  return boost::assign::list_of("NOTE")("LION")("QUIT")("REAL")("TIME")("VIEW")("EXAM")("YAWN")("FOUR")("BLUE")("TREE")("FROG")("NOSE")("FAST")("SLOW")("PLAY")("REST")("HOME")("PARK")("SICK")("SING")("BOOK")("JUMP")("STOP")("WALK");
}

//5 letter words
const std::vector<std::string> English2Hangman::createLongWords() const
{
  return boost::assign::list_of("PIZZA")("APPLE")("BEACH")("INDIA")("SLATE")("MOUSE")("MONEY")("WATER")("GREAT")("GRADE")("GRAND")("STATE")("NIGHT")("DRESS")("CLOCK")("THING")("WATCH")("CLEAN")("CHECK")("SEVEN")("HOUSE")("STORY")("ABOUT")("SOUND")("ROUND");
}

//6 letter words
const std::vector<std::string> English2Hangman::createxLongWords() const
{
  return boost::assign::list_of("PLEASE")("GARDEN")("YELLOW")("ORANGE")("PURPLE")("FLOWER")("FRIEND")("MATHRU")("GROUND")("PLEASE")("FAMILY")("BORROW")("PUNISH")("ALWAYS")("ACROSS")("SQUARE")("PERSON")("INSIDE")("NUMBER")("INSECT");
}

//7 letter words
const std::vector<std::string> English2Hangman::createxxLongWords() const
{
  return boost::assign::list_of("STUDENT")("TEACHER")("PRETEND")("JEALOUS")("HUNDRED")("WEATHER")("WHISPER")("MILLION")("CHICKEN")("OCTOPUS");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Arabic2Hangman::Arabic2Hangman(IOEventParser& my_iep) :
      Hangman(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, createAlphabet(), createShortWords(), createMedWords(), createLongWords(), createxLongWords(), createxxLongWords(), true)
{

}

const std::vector<std::string> Arabic2Hangman::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

//3 letter words
const std::vector<std::string> Arabic2Hangman::createShortWords() const
{
  //XXX:Not used in the application code
  return boost::assign::list_of("");
}

//4 letter words
const std::vector<std::string> Arabic2Hangman::createMedWords() const
{
  //return boost::assign::list_of("ضصثق")("شسيب")("دجحخ");
  return boost::assign::list_of("ضفدع")("شجرة")("ضجيج")("سريع")("كتاب")("مريض")("عظيم")("سبعة");
}

//5 letter words
const std::vector<std::string> Arabic2Hangman::createLongWords() const
{
  //return boost::assign::list_of("طكمنت")("جحخهع");
  return boost::assign::list_of("فوشيا")("طعام")("داخل")("متكبر");
}

//6 letter words
const std::vector<std::string> Arabic2Hangman::createxLongWords() const
{
  //return boost::assign::list_of("ةىركبت")("ضصهثشسي");
  return boost::assign::list_of("توفاحة")("موسيقى")("معلمتي")("الكحول");
}

//7 letter words
const std::vector<std::string> Arabic2Hangman::createxxLongWords() const
{
  //return boost::assign::list_of("منتالبي");
  return boost::assign::list_of("باذنجان");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
