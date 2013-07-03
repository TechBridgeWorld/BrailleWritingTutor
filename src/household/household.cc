/*
 * household.cc
 * adapted from animal.cc may 2013
 *      Author: Madeleine Clute
 */

#include <assert.h>
#include <boost/assign/list_of.hpp>
#include "household.h"
#include <string>

#define MAX_CATS 3 // have short, medium, and long sounds... 
                  // programmer can decide what "short," "medium," and "long" mean
#define SHORT 0
#define MEDIUM 1
#define LONG 2

static time_t last_event_time = time(0);

Household::Household(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, const std::vector<std::string> my_alph, const ForeignLanguage2EnglishMap sw, const ForeignLanguage2EnglishMap mw, const ForeignLanguage2EnglishMap lw, bool f) :
  IBTApp(my_iep, path_to_mapping_file), iep(my_iep), su(my_su), alphabet(my_alph), short_sounds(sw), med_sounds(mw), long_sounds(lw),
      letter_skill(alphabet.size()), firsttime(true), turncount(0), word(""), target_letter(""), word_pos(0), word_length(0), nomirror(f), everyday_s ("./resources/Voice/everyday_sounds/", iep)
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

void Household::processEvent(IOEvent& e)
{
  //Whenever the user hits Button0 we immediately want the LETTER event to be generated so that he doesnt have to wait for the timeout
  if( e.type == IOEvent::BUTTON && e.button == 0 )
  {
    //iep.clearQueue();
    iep.flushGlyph();
    printf("flushing glyph\n");
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
      iep.clearQueue();
      return;//skip
    }
    else
    {
      su->sayLetter(getStudentVoice(), (std::string) e.letter);
      AL_attempt((std::string) e.letter);
    }
  }
  iep.clearQueue(); // clear out the rest of the events that might be backlogged
}

/* passes in an int and then returns the average of the short medium and long
 * word skill levels. short, medium, and long are relative to the longest and 
 * shortest words in the list
 TODO: PUT SOMETHING IN THAT ASSERTS ALL WORDS ARE LESS THAN MAX_LEN
 */
int Household::getEstimate(int n)
{


}

void Household::AL_new()
{
  std::vector<int> low_letters;

  //no letter skill to be trained
  target_letter = "\0";
  //choose a new animal target:
  std::vector<std::string> choices;
  turncount = 0;
  bool need_short = (LS_length_skill[3].estimate() < .9); //animal words with 3 letters
  bool need_med = (LS_length_skill[5].estimate() < .9);//animal words with 5 letters
  bool need_long = (LS_length_skill[7].estimate() < .9);//animal words with 7 letters

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
    ForeignLanguage2EnglishMap::const_iterator it = short_sounds.begin();
    for(; it != short_sounds.end(); it++)
    {
      choices.push_back(it->first);
      word_length = SHORT;
    }
  }
  else if( need_med )
  {
    ForeignLanguage2EnglishMap::const_iterator it = med_sounds.begin();
    for(; it != med_sounds.end(); it++)
    {
      choices.push_back(it->first);
      word_length = MEDIUM;
    }
  }
  else if( need_long )
  {
    ForeignLanguage2EnglishMap::const_iterator it = long_sounds.begin();
    for(; it != long_sounds.end(); it++)
    {
      choices.push_back(it->first);
      word_length = LONG;
    }
  }

  random_shuffle(choices.begin(), choices.end());
  word = choices.front();
  word_pos = 0;
  //Asound += ".wav";
  std::cout << "		(DEBUG)Animal sound:" << word << std::endl;
  su->saySound(everyday_s, "please_write_the_object"); 
  su->saySound(everyday_s, householdNameToSound(word));
}

std::string Household::householdNameToSound(const std::string& animal)
{
  ForeignLanguage2EnglishMap::const_iterator it;
  it = short_sounds.find(animal);
  if( it != short_sounds.end() )
    return it->second;

  it = med_sounds.find(animal);
  if( it != med_sounds.end() )
    return it->second;

  it = long_sounds.find(animal);
  if( it != long_sounds.end() )
    return it->second;

  assert(false);
}

void Household::AL_attempt(std::string i)
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
      su->sayLetterSequence(getTeacherVoice(), word);
      return;
    }
    else
    {
      int index = alphabeticIndex(alphabet, word, word_pos, target_letter);
      letter_skill[index].observe(wrong);
      std::cout << target_letter << ": " << letter_skill[index].estimate() << std::endl;
      //std::cout << "    (DEBUG)Targetletter wrong" << std::endl;
      su->saySound(getTeacherVoice(), "no");
      printf("saying invalid pattern\n");

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
        //su->saySound(getTeacherVoice(), "please write the animal2");
        //su->saySound(getTeacherVoice(), householdNameToSound(word));
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

EnglishHousehold::EnglishHousehold(IOEventParser& my_iep) :
  Household(my_iep, "", new EnglishSoundsUtil, createAlphabet(), createShortHouseholdWords(), createMedHouseholdWords(), createLongHouseholdWords(), false)
{

}

const std::vector<std::string> EnglishHousehold::createAlphabet() const
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

const ForeignLanguage2EnglishMap EnglishHousehold::createShortHouseholdWords() const
{
  //objects that have <5 letters
  return boost::assign::map_list_of("RAIN", "RAIN");
}

const ForeignLanguage2EnglishMap EnglishHousehold::createMedHouseholdWords() const
{
  //5-6 letters
  return boost::assign::map_list_of("CLOCK", "CLOCK")("PHONE", "PHONE")("SIREN", "SIREN")("TRAIN", "TRAIN")("TRUCK","TRUCK");
}

const ForeignLanguage2EnglishMap EnglishHousehold::createLongHouseholdWords() const
{
  // > 7 letters
  return boost::assign::map_list_of("DOORBELL","DOORBELL")("AEROPLANE","AEROPLANE");
}
