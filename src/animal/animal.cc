/*
 * animal.cc
 *
 *  Created on: Dec 8, 2008
 *      Author: Students
 *      Multi-lingual enhancements: Imran
 */

#include <assert.h>
#include <boost/assign/list_of.hpp>
#include "animal.h"
#define MAX_CATS 3 
#define SHORT 0
#define MEDIUM 1
#define LONG 2


 time_t last_time = time(0);

Animal::Animal(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, const std::vector<std::string> my_alph, const ForeignLanguage2EnglishMap sw, const ForeignLanguage2EnglishMap mw, const ForeignLanguage2EnglishMap lw, bool f) :
  IBTApp(my_iep, path_to_mapping_file), iep(my_iep), su(my_su), alphabet(my_alph), short_animals(sw), med_animals(mw), long_animals(lw),
      letter_skill(alphabet.size()), firsttime(true), turncount(0), word(""), target_letter(""), word_pos(0), word_length(0), nomirror(f), animal_s("./resources/Voice/animal_sounds/",iep)
{
  for(int i = 0; i < alphabet.size(); i++)
  {
    letter_skill[i] = KnowledgeTracer(.9) //think they know the letters
        .registerEvent(right, .8, .01) .registerEvent(wrong, .2, .833);
  }

  for(int i = 0; i < MAX_CATS; i++)
  {
    LS_length_skill[i] = KnowledgeTracer(.01) .registerEvent(right, .7, .1) .registerEvent(wrong, .1, .7);
  }

  AL_new();
}

void Animal::processEvent(IOEvent& e)
{
  printf("animal process event called\n");
  //Whenever the user hits Button0 we immediately want the LETTER event to be generated so that he doesnt have to wait for the timeout
  if( e.type == IOEvent::BUTTON && e.button == 0 )
  {
    printf("DEBUG flushing glyph\n");
    iep.flushGlyph();
    return; //required? hmm..
  }

  if( e.type == IOEvent::CELL_LETTER || e.type == IOEvent::BUTTON_LETTER )
  {
    //printEvent(e);
    //Upon entering this mode, we dont want any pending LETTER events to interfere. So we skip the first LETTER event.
    if( firsttime )//Check if this is the first letter event, if so, we skip it
    {
      std::cout << "    (DEBUG)Skipping first letter event" << std::endl;
      firsttime = false;
      return;//skip
    }
    else if (last_time == time(0)) {
      last_time = time(0);
      printf("caught\n");
      return;
    }
    else 
    {
      last_time = time(0);
      std::cout << " thinks "<< ((std::string) e.letter) << "was entered" << std::endl;
      su->sayLetter(getStudentVoice(), (std::string) e.letter);
      AL_attempt((std::string) e.letter);
    }
  }
  //iep.clearQueue();
}

void Animal::AL_new()
{
  srand(time(0));
  std::vector<int> low_letters;
  float min_knowledge = .7;
  //no letter skill to be trained
  target_letter = "\0";
  //choose a new animal target:
  std::vector<std::string> choices;
  turncount = 0;
  bool need_short = (LS_length_skill[SHORT].estimate() < min_knowledge); //animal words with 3 letters
  bool need_med = (LS_length_skill[MEDIUM].estimate() < min_knowledge);//animal words with 5 letters
  bool need_long = (LS_length_skill[LONG].estimate() < min_knowledge);//animal words with 7 letters

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
    ForeignLanguage2EnglishMap::const_iterator it = short_animals.begin();
    for(; it != short_animals.end(); it++)
    {
      choices.push_back(it->first);
    }
    word_length = SHORT;
  }
  else if( need_med )
  {
    ForeignLanguage2EnglishMap::const_iterator it = med_animals.begin();
    for(; it != med_animals.end(); it++)
    {
      choices.push_back(it->first);
    }
    word_length = MEDIUM;
  }
  else if( need_long )
  {
    ForeignLanguage2EnglishMap::const_iterator it = long_animals.begin();
    for(; it != long_animals.end(); it++)
    {
      choices.push_back(it->first);
    }
    word_length = LONG;
  }

  random_shuffle(choices.begin(), choices.end());
  word = choices.front();
  word_pos = 0;
  //Asound += ".wav";
  std::cout << "		(DEBUG)Animal sound:" << word << std::endl;
  printf("got here animal game\n");
  su->saySound(animal_s, "please write the animal2"); 
  su->saySound(animal_s, animalNameToSound(word));
  //iep.clearQueue();
}

std::string Animal::animalNameToSound(const std::string& animal)
{
  ForeignLanguage2EnglishMap::const_iterator it;
  it = short_animals.find(animal);
  if( it != short_animals.end() )
    return it->second;

  it = med_animals.find(animal);
  if( it != med_animals.end() )
    return it->second;

  it = long_animals.find(animal);
  if( it != long_animals.end() )
    return it->second;

  assert(false);
}

void Animal::AL_attempt(std::string i)
{

  static const Charset &charset = IBTApp::getCurrentCharset();

  ////
  if( target_letter.compare("\0") != 0 )
  { //if we were re-hashing a letter skill then one letter at a time
    if( target_letter.compare((std::string) i) == 0 )
    { //match!
      int index = alphabeticIndex(alphabet, word, word_pos, target_letter);

      letter_skill[index].observe(right);
      std::cout << target_letter << ": " << letter_skill[index].estimate() << std::endl;
      su->saySound(getTeacherVoice(), "good");
      target_letter = "\0";
      word_pos = 0;
      su->saySound(getTeacherVoice(), "please write");
      su->sayLetterSequence(animal_s, word);
      return;
    }
    else
    {
      int index = alphabeticIndex(alphabet, word, word_pos, target_letter);
      letter_skill[index].observe(wrong);
      std::cout << target_letter << ": " << letter_skill[index].estimate() << std::endl;
      //std::cout << "    (DEBUG)Targetletter wrong" << std::endl;
      su->saySound(getTeacherVoice(), "no");

      bool teaching_letter = (letter_skill[index].estimate() < .1);

      if( teaching_letter )
      {
        su->saySound(getTeacherVoice(), "to write the letter");
        su->sayLetter(getTeacherVoice(), target_letter);
        su->saySound(getTeacherVoice(), "press");
        su->sayDotSequence(getTeacherVoice(), charset[GlyphMapping(target_letter)]);//get the dots of the corresponding letter
        //std::cout << "    (DEBUG)Targetletter wrong, learning this letter" << std::endl;
      }

      su->saySound(getTeacherVoice(), "please write");
      su->saySound(getTeacherVoice(), target_letter);
      //std::cout << "    (DEBUG)Targetletter was asked to be written" << std::endl;

      word_pos = 0;

      return;
    }
  }

  else
  { // need to spell entire name of animal allowed three chances to guess before it tells you asnwer
    int num_bytes_in_letter = numBytesInUTF8Letter(word.at(word_pos));
    std::string correct_letter(word, word_pos, num_bytes_in_letter);
    //std::cout << "    (DEBUG)utf8 encoded size is:" << num_bytes_in_letter << "  word.size is:" << word.size() << "  word length is:" << word_length << " Correct letter is:"<<correct_letter<<" Target letter is:"<<target_letter<<" word.at(word_pos) is:"<<word.at(word_pos)<<std::endl;
    int index = alphabeticIndex(alphabet, word, word_pos, word.at(word_pos));
    //std::cout << "    (DEBUG)First time,correct letter is" << correct_letter << " at index " << index << std::endl;
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
        AL_new();
        return;
      }
    }
    else
    { //letter is incorrect
      su->saySound(getTeacherVoice(), "no"); // that is the incorrect animal
      word_pos = 0;
      turncount++;
      //std::cout << "    (DEBUG)Now at turn:" << turncount << std::endl;
      if( turncount < 3 )
      {
        su->saySound(animal_s, "please write the animal2");
        su->saySound(animal_s, animalNameToSound(word));
        //std::cout << "    (DEBUG)Got the letter wrong at turn" << turncount << std::endl;
      }
      else
      {
        if( turncount == 3 )
        {
          su->saySound(getTeacherVoice(), "solution");
          su->sayLetterSequence(getTeacherVoice(), word);
          //std::cout << "    (DEBUG)All 3 turns used" << std::endl;
        }
        su->saySound(getTeacherVoice(), "please write");
        su->sayLetterSequence(getTeacherVoice(), word);
        letter_skill[index].observe(wrong);
        //std::cout << "    (DEBUG)The estimate of the letter is:" << letter_skill[index].estimate() << std::endl;
        //LS_length_skill[word_length].observe(wrong);
        if( letter_skill[index].estimate() < .1 )
        {
          su->saySound(getTeacherVoice(), "to write the letter");
          su->sayLetter(getTeacherVoice(), correct_letter);
          su->saySound(getTeacherVoice(), "press");
          su->sayDotSequence(getTeacherVoice(), charset[GlyphMapping(correct_letter)]);
          su->saySound(getTeacherVoice(), "please write");
          su->sayLetter(getTeacherVoice(), correct_letter);
          //iep.clearQueue();
          target_letter = correct_letter;
          word_pos = 0;
          //std::cout << "    (DEBUG)Bad at this letter" << std::endl;
        }
      }
      return;
    }
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EnglishAnimal::EnglishAnimal(IOEventParser& my_iep) :
  Animal(my_iep, "", new EnglishSoundsUtil, createAlphabet(), createShortAnimalWords(), createMedAnimalWords(), createLongAnimalWords(), false)
{

}

const std::vector<std::string> EnglishAnimal::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

const ForeignLanguage2EnglishMap EnglishAnimal::createShortAnimalWords() const
{
  //animals whos name has  3-4 ish letters
  return boost::assign::map_list_of("CAT", "CAT")("DOG", "DOG")("BEE", "BEE")("COW", "COW")
                                   ("PIG", "PIG")("CROW", "CROW")("OWL","OWL")("LION","LION");
}

const ForeignLanguage2EnglishMap EnglishAnimal::createMedAnimalWords() const
{
  //animals whos name has 5-6 ish letters
  return boost::assign::map_list_of("SHEEP", "SHEEP")("HORSE", "HORSE")("ZEBRA", "ZEBRA")("CAMEL", "CAMEL")
                                   ("MONKEY","MONKEY")("SNAKE","SNAKE");
}

const ForeignLanguage2EnglishMap EnglishAnimal::createLongAnimalWords() const
{
  //animals whos name has > 5 letters
  return boost::assign::map_list_of("ROOSTER","ROOSTER")("ELEPHANT","ELEPHANT")("PEACOCK", "PEACOCK");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

ArabicAnimal::ArabicAnimal(IOEventParser& my_iep) :
      Animal(my_iep, "./language_mapping_files/arabic_mapping.txt", new ArabicSoundsUtil, createAlphabet(), createShortAnimalWords(), createMedAnimalWords(), createLongAnimalWords(), false)
{

}

const std::vector<std::string> ArabicAnimal::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

const ForeignLanguage2EnglishMap ArabicAnimal::createShortAnimalWords() const
{
  //animals whos name has exactly 3 letters
  return boost::assign::map_list_of("كلب", "DOG")("جمل", "CAMEL");
}

const ForeignLanguage2EnglishMap ArabicAnimal::createMedAnimalWords() const
{
  //animals whos name has exactly 5 letters
  return boost::assign::map_list_of("خنزير", "PIG");
}

const ForeignLanguage2EnglishMap ArabicAnimal::createLongAnimalWords() const
{
  //animals whos name has exactly 7 letters
  return boost::assign::map_list_of("","");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FrenchAnimal::FrenchAnimal(IOEventParser& my_iep) :
      Animal(my_iep, "./language_mapping_files/french_mapping.txt", new FrenchSoundsUtil, createAlphabet(), createShortAnimalWords(), createMedAnimalWords(), createLongAnimalWords(), false)
{

}

const std::vector<std::string> FrenchAnimal::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z")("Ç")("É")("À")("È")("Ù")("Â")("Ê")("Î")("Ô")("Û")("Ë")("Ï")("Ü")("œ");
}

const ForeignLanguage2EnglishMap FrenchAnimal::createShortAnimalWords() const
{
  //animals whos name has exactly 3 letters
  return boost::assign::map_list_of("COQ","ROOSTER")("LION","LION")("PAON","PEACOCK")("CHAT","CAT"); 
}

const ForeignLanguage2EnglishMap FrenchAnimal::createMedAnimalWords() const
{
  //animals whos name has exactly 5 letters
  return boost::assign::map_list_of("CHIEN","DOG")("VACHE","COW")("ZÈBRE","ZEBRA")("SINGE","MONKEY");
}

const ForeignLanguage2EnglishMap FrenchAnimal::createLongAnimalWords() const
{
  //animals whos name has exactly 7 letters
  return boost::assign::map_list_of("CHAMEAU","CAMEL")("ABEILLE","BEE")("COCHON","PIG")("CORBEAU","CROW")
                                    ("CHOUETTE","OWL")("MOUTON","SHEEP")("CHEVAL","HORSE")("SERPENT","SNAKE")
                                    ("ÉLÉPHANT","ELEPHANT");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

KiswahiliAnimal::KiswahiliAnimal(IOEventParser& my_iep) :
      Animal(my_iep, "", new KiswahiliSoundsUtil, createAlphabet(), createShortAnimalWords(), createMedAnimalWords(), createLongAnimalWords(), false)
{

}

const std::vector<std::string> KiswahiliAnimal::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("R")("S")("T")("U")("W")("Y")("Z");
}

const ForeignLanguage2EnglishMap KiswahiliAnimal::createShortAnimalWords() const
{
  //animals whos name has exactly 3 letters
  return boost::assign::map_list_of("OIE","ROOSTER"); //"oie" means rooster
}

const ForeignLanguage2EnglishMap KiswahiliAnimal::createMedAnimalWords() const
{
  //animals whos name has exactly 5 letters
  return boost::assign::map_list_of("JIMBI","ROOSTER")("VACHE","COW");
}

const ForeignLanguage2EnglishMap KiswahiliAnimal::createLongAnimalWords() const
{
  //animals whos name has exactly 7 letters
  return boost::assign::map_list_of("CHAMEAU","CAMEL");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

English2Animal::English2Animal(IOEventParser& my_iep) :
  Animal(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", new English2SoundsUtil, createAlphabet(), createShortAnimalWords(), createMedAnimalWords(), createLongAnimalWords(), true)
{

}

const std::vector<std::string> English2Animal::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

const ForeignLanguage2EnglishMap English2Animal::createShortAnimalWords() const
{
  //animals whos name has exactly 3 letters
  return boost::assign::map_list_of("CAT", "CAT")("DOG", "DOG")("BEE", "BEE")("COW", "COW")("PIG", "PIG");
}

const ForeignLanguage2EnglishMap English2Animal::createMedAnimalWords() const
{
  //animals whos name has exactly 5 letters
  return boost::assign::map_list_of("SHEEP", "SHEEP")("HORSE", "HORSE")("ZEBRA", "ZEBRA")("CAMEL", "CAMEL")("HYENA", "HYENA");
}

const ForeignLanguage2EnglishMap English2Animal::createLongAnimalWords() const
{
  //animals whos name has exactly 7 letters
  return boost::assign::map_list_of("ROOSTER","ROOSTER");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Arabic2Animal::Arabic2Animal(IOEventParser& my_iep) :
      Animal(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", new Arabic2SoundsUtil, createAlphabet(), createShortAnimalWords(), createMedAnimalWords(), createLongAnimalWords(), true)
{

}

const std::vector<std::string> Arabic2Animal::createAlphabet() const
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}

const ForeignLanguage2EnglishMap Arabic2Animal::createShortAnimalWords() const
{
  //animals whos name has exactly 3 letters
  return boost::assign::map_list_of("كلب", "DOG")("جمل", "CAMEL");
}

const ForeignLanguage2EnglishMap Arabic2Animal::createMedAnimalWords() const
{
  //animals whos name has exactly 5 letters
  return boost::assign::map_list_of("خنزير", "PIG");
}

const ForeignLanguage2EnglishMap Arabic2Animal::createLongAnimalWords() const
{
  //animals whos name has exactly 7 letters
  return boost::assign::map_list_of("","");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
