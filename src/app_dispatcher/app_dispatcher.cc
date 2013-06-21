/*
 * app_dispatcher.cc
 *
 *  Created on: Nov 30, 2008
 *      Author: imran
 *  Updated in 2010 & 2011 by Ameer
 *  Updated to allow flexible mode configurations: August 2012: M. Freddie Dias <mfdias@ri.cmu.edu>
 */

#include <cassert>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include "app_dispatcher.h"

#include "dot_scaffold/dot_scaffold.h"
#include "learn_dots/learn_dots.h"
#include "dot_practice/dot_practice.h"
#include "learn_letters/learn_letters.h"
#include "learn_numbers/learn_numbers.h"
#include "number_scaffold/number_scaffold.h"
#include "letter_scaffold/letter_scaffold.h"
#include "letter_practice/letter_practice.h"
#include "domino/domino_game.h"
#include "hangman/hangman.h"
#include "animal/animal.h"
#include "household/household.h"
#include "arithmetic_practice/arithmetic.h"


using namespace BrailleTutorNS;

void ApplicationDispatcher::operator()(std::deque<IOEvent> &events)
{
  while( !events.empty() )
  {
    IOEvent e = events.front();
    events.pop_front();

    //Check if the event is for scrolling thru the list of applications
    if (isScrollEvent(e))
    {
      scroll(e);
      return;
    }

    //Check if the user has finished scrolling by selecting his app, and thus wants to start it
    if( isSwitchAppEvent(e) )
    {
      delete current_app;//stop the running app (calls the destructor)
      current_app = switchApp(e);//make the switch
      return;
    }

    if(switching_modes != SCROLL_ON) //it is a normal event, so just pass it to whatever app is currently running
    {
      if( current_app != NULL )
        current_app->processEvent(e);
    }
  }
}

/*
 *  If Button 0 is down and the user presses a button, we're mode switching
 */
bool ApplicationDispatcher::isScrollEvent(const IOEvent& e)
{
  //Check if someone is trying to retun back to scrolling mode.
  if (switching_modes == SCROLL_OFF && e.type == IOEvent::BUTTON_DOWN && e.button == 0)
  {
    switching_modes = BZERO_DOWN;
  }

  //Check if someone released Button0
  if(switching_modes == BZERO_DOWN && e.type == IOEvent::BUTTON_UP && e.button == 0)
  {
    switching_modes = SCROLL_OFF;
  }

  //Check if we are in scrolling mode
  if(switching_modes == SCROLL_ON && e.type == IOEvent::BUTTON_DOWN)
  {
    //We are already in scrolling mode, so hitting Button0 again does not indicate a scroll event
    if (e.button == 0)
      return false;
    return true;
  }

  //A user tries to return back to scrolling mode by holding down Button0, and then pressing another button
  if(switching_modes == BZERO_DOWN && e.type == IOEvent::BUTTON_DOWN && e.button != 0)
  {
    switching_modes = SCROLL_ON;
    return true;
  }

  return false;
}

bool ApplicationDispatcher::isSwitchAppEvent(const IOEvent& e)
{
  return switching_modes == SCROLL_ON && e.type == IOEvent::BUTTON_DOWN && e.button == 0;
}


/*
 * This function handles scrolling through the modes 
 */
void ApplicationDispatcher::scroll(const IOEvent& e)
{
	assert(SCROLL_ON && "scroll() called while not in scrolling state");
  assert(!modes_list.empty()); // must have at least one mode
	
	//Scrolling Left/Up (reducing current mode index)
	if(e.button >= 1 && e.button <= 3) {
	  //current_mode = current_mode == FREE_PLAY ? ANIMAL_GAME_OTHER : (modes) (current_mode - 1);
    current_mode_index = (current_mode_index == 0) ? (modes_list.size() - 1) : (current_mode_index - 1);
	}
	
	//Scrolling Right/Down (increasing current mode index)
	else if(e.button >= 4 && e.button <= 6) {
    //current_mode = current_mode == ANIMAL_GAME_OTHER ? FREE_PLAY : (modes) (current_mode + 1);
    current_mode_index = (current_mode_index == (modes_list.size() - 1)) ? 0 : (current_mode_index + 1);
	}

  playSelectedMode(); // plays the sound file associated with the mode at the current_mode_index
	
	/* TODO: Move config file reading code to higher level in the program (ie closer to top-level main).  
	*  We currently use a bool so as only to read once, simple and works fine
	*/
	// read config file to determine language mode
	//if(!cfg_file_processed)
	//	processConfigFile();
	
	//Inform the user which mode he has currently selected
	//if(language_mode==EN_AR)
	//	playSelectedModeInEnglishArabic();
	//else if(language_mode==EN_FR)
	//	playSelectedModeInEnglishFrench();
	//else if(language_mode==EN_SWH)
	//	playSelectedModeInEnglishKiswahili();
	//else if(language_mode==EN2_AR2)
	//	playSelectedModeInEnglish2Arabic2();

}

/*
 * This function determines which app to switch to, and then makes the switch by instantiating a class of the
 * determined app.
 */
IBTApp* ApplicationDispatcher::switchApp(const IOEvent& e)
{
	assert(switching_modes==SCROLL_ON && "switchApp() while not in scrolling state");
	
	//The fact that we are in this function means that the user has made his
	//choice by selecting an application. So we disable scrolling
	switching_modes = SCROLL_OFF;

  return switchToSelectedMode(); // switches to the mode indicated by the current_mode_index
  
	/* TODO: Move config file reading code to higher level in the program (ie closer to top-level main).  
	*  We currently use a bool so as only to read once, simple and works fine
	*/
	// read config file to determine language mode
	//if(!cfg_file_processed)
	//	processConfigFile();

	//if(language_mode==EN_AR)
	//	return switchAppEnglishArabic();
	//else if(language_mode==EN_FR)
	//	return switchAppEnglishFrench();
	//else if(language_mode==EN_SWH)
	//	return switchAppEnglishKiswahili();
	//else if(language_mode==EN2_AR2)
	//	return switchAppEnglish2Arabic2();
}

// reads config file and sets up vector of modes to be used
void ApplicationDispatcher::processConfigFile()
{
	bool cfg_file_ok=false; //assume config file non-existant or corrupted, update status after reading
	std::string str;
	std::vector<std::string> strs;
	std::ifstream cfgfile;
	
	std::cout << "reading config file..." << std::endl;
	cfgfile.open("./config/modes.cfg");
  
  bool reading_mode_lines = false; // helps us to read multi-line modes config values
  
	while(getline(cfgfile, str)) {	
    if(str.substr(0,1) != "#") { // ignore comment lines
      if (reading_mode_lines) {
        if (boost::starts_with(str, "Modes_End")) {
          reading_mode_lines = false;
          if (!modes_list.empty()) {
            cfg_file_ok = true; // we've got more at least one mode, set file status to OK
          }
        } else {
          // line between "Modes_Begin" and "Modes_End" - try to map to known modes
          std::string mode_str = str;
          boost::algorithm::trim(mode_str); // remove any whitespace (allows indentation)

          // now we check each mode -- this is a long list!
          if (mode_str == "FREE_PLAY_ENGLISH") {
            modes_list.push_back(FREE_PLAY_ENGLISH);
          } else if (mode_str == "FREE_SPELLING_ENGLISH") {
            modes_list.push_back(FREE_SPELLING_ENGLISH);
          } else if (mode_str == "FREE_NUMBERS_ENGLISH") {
            modes_list.push_back(FREE_NUMBERS_ENGLISH);
          } else if (mode_str == "LEARN_DOTS_ENGLISH") {
            modes_list.push_back(LEARN_DOTS_ENGLISH);
          } else if (mode_str == "DOT_PRACTICE_ENGLISH") {
            modes_list.push_back(DOT_PRACTICE_ENGLISH);
          } else if (mode_str == "LEARN_LETTERS_ENGLISH") {
            modes_list.push_back(LEARN_LETTERS_ENGLISH);
          } else if (mode_str == "LETTER_PRACTICE_ENGLISH") {
            modes_list.push_back(LETTER_PRACTICE_ENGLISH);
          } else if (mode_str == "LEARN_NUMBERS_ENGLISH") {
            modes_list.push_back(LEARN_NUMBERS_ENGLISH);
          } else if (mode_str == "DOMINOS_ENGLISH") {
            modes_list.push_back(DOMINOS_ENGLISH);
          } else if (mode_str == "HANGMAN_ENGLISH") {
            modes_list.push_back(HANGMAN_ENGLISH);
          } else if (mode_str == "ANIMAL_GAME_ENGLISH") {
            modes_list.push_back(ANIMAL_GAME_ENGLISH);
		      } else if (mode_str == "HOUSEHOLD_GAME_ENGLISH") {
			       modes_list.push_back(HOUSEHOLD_GAME_ENGLISH);
          } else if (mode_str == "ARITHMETIC_PRACTICE_ENGLISH") {
            modes_list.push_back(ARITHMETIC_PRACTICE_ENGLISH);
          } else if (mode_str == "LEARN_LETTERS_HINDI") {
            modes_list.push_back  (LEARN_LETTERS_HINDI);
          } else if (mode_str == "FREE_PLAY_ARABIC") {
            modes_list.push_back(FREE_PLAY_ARABIC);
          } else if (mode_str == "FREE_SPELLING_ARABIC") {
            modes_list.push_back(FREE_SPELLING_ARABIC);
          } else if (mode_str == "FREE_NUMBERS_ARABIC") {
            modes_list.push_back(FREE_NUMBERS_ARABIC);
          } else if (mode_str == "LEARN_DOTS_ARABIC") {
            modes_list.push_back(LEARN_DOTS_ARABIC);
          } else if (mode_str == "DOT_PRACTICE_ARABIC") {
            modes_list.push_back(DOT_PRACTICE_ARABIC);
          } else if (mode_str == "LEARN_LETTERS_ARABIC") {
            modes_list.push_back(LEARN_LETTERS_ARABIC);
          } else if (mode_str == "LETTER_PRACTICE_ARABIC") {
            modes_list.push_back(LETTER_PRACTICE_ARABIC);
          } else if (mode_str == "LEARN_NUMBERS_ARABIC") {
            modes_list.push_back(LEARN_NUMBERS_ARABIC);
          } else if (mode_str == "DOMINOS_ARABIC") {
            modes_list.push_back(DOMINOS_ARABIC);
          } else if (mode_str == "HANGMAN_ARABIC") {
            modes_list.push_back(HANGMAN_ARABIC);
          } else if (mode_str == "ANIMAL_GAME_ARABIC") {
            modes_list.push_back(ANIMAL_GAME_ARABIC);
          } else if (mode_str == "FREE_PLAY_FRENCH") {
            modes_list.push_back(FREE_PLAY_FRENCH);
          } else if (mode_str == "FREE_SPELLING_FRENCH") {
            modes_list.push_back(FREE_SPELLING_FRENCH);
          } else if (mode_str == "FREE_NUMBERS_FRENCH") {
            modes_list.push_back(FREE_NUMBERS_FRENCH);
          } else if (mode_str == "LEARN_DOTS_FRENCH") {
            modes_list.push_back(LEARN_DOTS_FRENCH);
          } else if (mode_str == "DOT_PRACTICE_FRENCH") {
            modes_list.push_back(DOT_PRACTICE_FRENCH);
          } else if (mode_str == "LEARN_LETTERS_FRENCH") {
            modes_list.push_back(LEARN_LETTERS_FRENCH);
          } else if (mode_str == "LETTER_PRACTICE_FRENCH") {
            modes_list.push_back(LETTER_PRACTICE_FRENCH);
          } else if (mode_str == "LEARN_NUMBERS_FRENCH") {
            modes_list.push_back(LEARN_NUMBERS_FRENCH);
          } else if (mode_str == "DOMINOS_FRENCH") {
            modes_list.push_back(DOMINOS_FRENCH);
          } else if (mode_str == "HANGMAN_FRENCH") {
            modes_list.push_back(HANGMAN_FRENCH);
          } else if (mode_str == "ANIMAL_GAME_FRENCH") {
            modes_list.push_back(ANIMAL_GAME_FRENCH);
          } else if (mode_str == "FREE_PLAY_SWAHILI") {
            modes_list.push_back(FREE_PLAY_SWAHILI);
          } else if (mode_str == "FREE_SPELLING_SWAHILI") {
            modes_list.push_back(FREE_SPELLING_SWAHILI);
          } else if (mode_str == "FREE_NUMBERS_SWAHILI") {
            modes_list.push_back(FREE_NUMBERS_SWAHILI);
          } else if (mode_str == "LEARN_DOTS_SWAHILI") {
            modes_list.push_back(LEARN_DOTS_SWAHILI);
          } else if (mode_str == "DOT_PRACTICE_SWAHILI") {
            modes_list.push_back(DOT_PRACTICE_SWAHILI);
          } else if (mode_str == "LEARN_LETTERS_SWAHILI") {
            modes_list.push_back(LEARN_LETTERS_SWAHILI);
          } else if (mode_str == "LETTER_PRACTICE_SWAHILI") {
            modes_list.push_back(LETTER_PRACTICE_SWAHILI);
          } else if (mode_str == "LEARN_NUMBERS_SWAHILI") {
            modes_list.push_back(LEARN_NUMBERS_SWAHILI);
          } else if (mode_str == "DOMINOS_SWAHILI") {
            modes_list.push_back(DOMINOS_SWAHILI);
          } else if (mode_str == "HANGMAN_SWAHILI") {
            modes_list.push_back(HANGMAN_SWAHILI);
          } else if (mode_str == "ANIMAL_GAME_SWAHILI") {
            modes_list.push_back(ANIMAL_GAME_SWAHILI);
          } else {
            std::cout << "WARNING! Unrecognized mode \"" << mode_str << "\"! Ignoring..." << std::endl;
          }
        }
      } else {
        if (boost::starts_with(str, "Modes_Begin")) {
          reading_mode_lines = true;
          continue;
        } else {
          // not reading modes so this must be some other config value 
          // Right now we just echo the config value to the screen
          // In the future we could use this to read in things like "use mirrorring" etc.
          
          boost::split(strs, str, boost::is_any_of("="));
			
          // check that we got two strings separated by an equals sign
          if(strs.size()==2) {
            std::cout << "Read config value: " << strs[0] << " = " << strs[1] << std::endl;
          }
        }
      }
    }
  }
	
	cfgfile.close();
	cfg_file_processed=true;
	
	if(cfg_file_ok==false) {
    std::cout << "Warning:  Config file does not exist or does not specify set of modes to be used.  Defaulting to standard set of modes" << std::endl;
    
    // Since the config file was invalid, we default to some basic English modes
    modes_list.clear(); // clear any modes that might have been added while reading invalid config
    modes_list.push_back(FREE_PLAY_ENGLISH);
    modes_list.push_back(FREE_SPELLING_ENGLISH);
    modes_list.push_back(LEARN_DOTS_ENGLISH);
    modes_list.push_back(DOT_PRACTICE_ENGLISH);
    modes_list.push_back(LEARN_LETTERS_ENGLISH);
    modes_list.push_back(LETTER_PRACTICE_ENGLISH);
    modes_list.push_back(HANGMAN_ENGLISH);
    modes_list.push_back(ANIMAL_GAME_ENGLISH);
    modes_list.push_back(ARITHMETIC_PRACTICE_ENGLISH);
	  modes_list.push_back(HOUSEHOLD_GAME_ENGLISH);
  }
}	


IBTApp* ApplicationDispatcher::switchToSelectedMode() const {
	std::cout << "Dispatcher switchToSelectedMode" << std::endl;
	
  switch(modes_list[current_mode_index]) {
  case FREE_PLAY_ENGLISH:
    return new EnglishDotScaffold(iep);
  case FREE_NUMBERS_ENGLISH:
    return new EnglishNumberScaffold(iep);
  case LEARN_DOTS_ENGLISH:
    return new EnglishLearnDots(iep);
  case DOT_PRACTICE_ENGLISH:
    return new EnglishDotPractice(iep);
  case LEARN_LETTERS_ENGLISH:
    return new EnglishLearnLetters(iep);
  case LEARN_NUMBERS_ENGLISH:
    return new EnglishLearnNumbers(iep);
  case LETTER_PRACTICE_ENGLISH:
    return new EnglishLetterPractice(iep);
  case FREE_SPELLING_ENGLISH:
    return new EnglishLetterScaffold(iep);
  case DOMINOS_ENGLISH:
    return new EnglishDominoGame(iep);
  case HANGMAN_ENGLISH:
    return new EnglishHangman(iep);
  case ANIMAL_GAME_ENGLISH:
    return new EnglishAnimal(iep);
  case HOUSEHOLD_GAME_ENGLISH:
	 return new EnglishHousehold(iep); 
  case ARITHMETIC_PRACTICE_ENGLISH:
    return new EnglishArithmeticPractice(iep);
  case LEARN_LETTERS_HINDI:
    return new Hindi2LearnLetters(iep);
  case FREE_PLAY_ARABIC:
    return new ArabicDotScaffold(iep);
  case FREE_NUMBERS_ARABIC:
    return new ArabicNumberScaffold(iep);
  case LEARN_DOTS_ARABIC:
    return new ArabicLearnDots(iep);
  case DOT_PRACTICE_ARABIC:
    return new ArabicDotPractice(iep);
  case LEARN_LETTERS_ARABIC:
    return new ArabicLearnLetters(iep);
  case LEARN_NUMBERS_ARABIC:
    return new ArabicLearnNumbers(iep);
  case LETTER_PRACTICE_ARABIC:
    return new ArabicLetterPractice(iep);
  case FREE_SPELLING_ARABIC:
    return new ArabicLetterScaffold(iep);
  case DOMINOS_ARABIC:
    return new ArabicDominoGame(iep);
  case HANGMAN_ARABIC:
    return new ArabicHangman(iep);
  case ANIMAL_GAME_ARABIC:
    return new ArabicAnimal(iep);
  case FREE_PLAY_FRENCH:
    return new FrenchDotScaffold(iep);
  case FREE_NUMBERS_FRENCH:
    return new FrenchNumberScaffold(iep);
  case LEARN_DOTS_FRENCH:
    return new FrenchLearnDots(iep);
  case DOT_PRACTICE_FRENCH:
    return new FrenchDotPractice(iep);
  case LEARN_LETTERS_FRENCH:
    return new FrenchLearnLetters(iep);
  case LEARN_NUMBERS_FRENCH:
    return new FrenchLearnNumbers(iep);
  case LETTER_PRACTICE_FRENCH:
    return new FrenchLetterPractice(iep);
  case FREE_SPELLING_FRENCH:
    return new FrenchLetterScaffold(iep);
  case DOMINOS_FRENCH:
    return new FrenchDominoGame(iep);
  case HANGMAN_FRENCH:
    return new FrenchHangman(iep);
  case ANIMAL_GAME_FRENCH:
    return new FrenchAnimal(iep);
  case FREE_PLAY_SWAHILI:
    return new KiswahiliDotScaffold(iep);
  case FREE_NUMBERS_SWAHILI:
    return new KiswahiliNumberScaffold(iep);
  case LEARN_DOTS_SWAHILI:
    return new KiswahiliLearnDots(iep);
  case DOT_PRACTICE_SWAHILI:
    return new KiswahiliDotPractice(iep);
  case LEARN_LETTERS_SWAHILI:
    return new KiswahiliLearnLetters(iep);
  case LEARN_NUMBERS_SWAHILI:
    return new KiswahiliLearnNumbers(iep);
  case LETTER_PRACTICE_SWAHILI:
    return new KiswahiliLetterPractice(iep);
  case FREE_SPELLING_SWAHILI:
    return new KiswahiliLetterScaffold(iep);
  case DOMINOS_SWAHILI:
    return new KiswahiliDominoGame(iep);
  case HANGMAN_SWAHILI:
    return new KiswahiliHangman(iep);
  case ANIMAL_GAME_SWAHILI:
    return new KiswahiliAnimal(iep);
  default:
    std::cerr << "ApplicationDispatcher::switchToSelectedMode(): ERROR! Current mode undefined!" << std::endl;
    return NULL;
  }
}

void ApplicationDispatcher::playSelectedMode() const
{
  switch (modes_list[current_mode_index]) {
  case FREE_PLAY_ENGLISH:
    teach.say("free play.wav");
    printf("Selecting English Dot Scaffold\n");
    break;
  case FREE_NUMBERS_ENGLISH:
    numbers.say("free_number_practice.wav");
    printf("Selecting English Number Scaffold\n");
    break;
  case LEARN_DOTS_ENGLISH:
    teach.say("learn dots.wav");
    printf("Selecting English Learn Dots\n");
    break;
  case DOT_PRACTICE_ENGLISH:
    teach.say("dot practice.wav");
    printf("Selecting English Dot Practice\n");
    break;
  case LEARN_LETTERS_ENGLISH:
    teach.say("learn letters.wav");
    printf("Selecting English Learn Letters\n");
    break;
  case LEARN_NUMBERS_ENGLISH:
    numbers.say("learn_numbers.wav");
    printf("Selecting English Learn Numbers\n");
    break;
  case LETTER_PRACTICE_ENGLISH:
    teach.say("letter practice.wav");
    printf("Selecting English Letter Practice\n");
    break;
  case FREE_SPELLING_ENGLISH:
    teach.say("free spelling.wav");
    printf("Selecting English Free Spelling\n");
    break;
  case DOMINOS_ENGLISH:
    teach.say("dominos.wav");
    printf("Selecting English Dominos\n");
    break;
  case HANGMAN_ENGLISH:
    teach.say("hangman.wav");
    printf("Selecting English Hangman\n");
    break;
  case ANIMAL_GAME_ENGLISH:
    teach.say("animal game.wav");
    printf("Selecting English Animal Game\n");
    break;
  case HOUSEHOLD_GAME_ENGLISH:
	  teach.say("everyday_noises.wav"); 
	  printf("Selecting English Household Game\n");
	  break;
  case ARITHMETIC_PRACTICE_ENGLISH:
    teach.say("maths_practice.wav"); 
    printf("Selecting English Maths pracice\n");
    break;
  case LEARN_LETTERS_HINDI:
      teach.say("learn_letters_hindi.wav"); //TODO change this
      printf("Selecting Hindi Learn Letters\n");
      break;
  case FREE_PLAY_ARABIC:
    teach.say("free play_arabic.wav");
    printf("Selecting Arabic Dot Scaffold\n");
    break;
  case FREE_NUMBERS_ARABIC:
    numbers.say("free_number_practice_arabic.wav");
    printf("Selecting Arabic Number Scaffold\n");
    break;
  case LEARN_DOTS_ARABIC:
    teach.say("learn dots_arabic.wav");
    printf("Selecting Arabic Learn Dots\n");
    break;
  case DOT_PRACTICE_ARABIC:
    teach.say("dot practice_arabic.wav");
    printf("Selecting Arabic Dot Practice\n");
    break;
  case LEARN_LETTERS_ARABIC:
    teach.say("learn letters_arabic.wav");
    printf("Selecting Arabic Learn Letters\n");
    break;
  case LEARN_NUMBERS_ARABIC:
    numbers.say("learn_numbers_arabic.wav");
    printf("Selecting Arabic Learn Numbers\n");
    break;
  case LETTER_PRACTICE_ARABIC:
    teach.say("letter practice_arabic.wav");
    printf("Selecting Arabic Letter Practice\n");
    break;
  case FREE_SPELLING_ARABIC:
    teach.say("free spelling_arabic.wav");
    printf("Selecting Arabic Free Spelling\n");
    break;
  case DOMINOS_ARABIC:
    teach.say("dominos_arabic.wav");
    printf("Selecting Arabic Dominos\n");
    break;
  case HANGMAN_ARABIC:
    teach.say("hangman_arabic.wav");
    printf("Selecting Arabic Hangman\n");
    break;
  case ANIMAL_GAME_ARABIC:
    teach.say("animal game_arabic.wav");
    printf("Selecting Arabic Animal Game\n");
    break;
  case FREE_PLAY_FRENCH:
    teach.say("free play_french.wav");
    printf("Selecting French Dot Scaffold\n");
    break;
  case FREE_NUMBERS_FRENCH:
    numbers.say("free_number_practice_french.wav");
    printf("Selecting French Number Scaffold\n");
    break;
  case LEARN_DOTS_FRENCH:
    teach.say("learn dots_french.wav");
    printf("Selecting French Learn Dots\n");
    break;
  case DOT_PRACTICE_FRENCH:
    teach.say("dot practice_french.wav");
    printf("Selecting French Dot Practice\n");
    break;
  case LEARN_LETTERS_FRENCH:
    teach.say("learn letters_french.wav");
    printf("Selecting French Learn Letters\n");
    break;
  case LEARN_NUMBERS_FRENCH:
    numbers.say("learn_numbers_french.wav");
    printf("Selecting French Learn Numbers\n");
    break;
  case LETTER_PRACTICE_FRENCH:
    teach.say("letter practice_french.wav");
    printf("Selecting French Letter Practice\n");
    break;
  case FREE_SPELLING_FRENCH:
    teach.say("free spelling_french.wav");
    printf("Selecting French Free Spelling\n");
    break;
  case DOMINOS_FRENCH:
    teach.say("dominos_french.wav");
    printf("Selecting French Dominos\n");
    break;
  case HANGMAN_FRENCH:
    teach.say("hangman_french.wav");
    printf("Selecting French Hangman\n");
    break;
  case ANIMAL_GAME_FRENCH:
    teach.say("animal game_french.wav");
    printf("Selecting French Animal Game\n");
    break;
  case FREE_PLAY_SWAHILI:
    teach.say("free play_kiswahili.wav");
    printf("Selecting Kiswahili Dot Scaffold\n");
    break;
  case FREE_NUMBERS_SWAHILI:
    numbers.say("free_number_practice_kiswahili.wav");
    printf("Selecting Kiswahili Number Scaffold\n");
    break;
  case LEARN_DOTS_SWAHILI:
    teach.say("learn dots_kiswahili.wav");
    printf("Selecting Kiswahili Learn Dots\n");
    break;
  case DOT_PRACTICE_SWAHILI:
    teach.say("dot practice_kiswahili.wav");
    printf("Selecting Kiswahili Dot Practice\n");
    break;
  case LEARN_LETTERS_SWAHILI:
    teach.say("learn letters_kiswahili.wav");
    printf("Selecting Kiswahili Learn Letters\n");
    break;
  case LEARN_NUMBERS_SWAHILI:
    numbers.say("learn_numbers_kiswahili.wav");
    printf("Selecting Kiswahili Learn Numbers\n");
    break;
  case LETTER_PRACTICE_SWAHILI:
    teach.say("letter practice_kiswahili.wav");
    printf("Selecting Kiswahili Letter Practice\n");
    break;
  case FREE_SPELLING_SWAHILI:
    teach.say("free spelling_kiswahili.wav");
    printf("Selecting Kiswahili Free Spelling\n");
    break;
  case DOMINOS_SWAHILI:
    teach.say("dominos_kiswahili.wav");
    printf("Selecting Kiswahili Dominos\n");
    break;
  case HANGMAN_SWAHILI:
    teach.say("hangman_kiswahili.wav");
    printf("Selecting Kiswahili Hangman\n");
    break;
  case ANIMAL_GAME_SWAHILI:
    teach.say("animal game_kiswahili.wav");
    printf("Selecting Kiswahili Animal Game\n");
    break;
  }
}
