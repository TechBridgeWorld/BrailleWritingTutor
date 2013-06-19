/*
 * app_dispatcher.h
 *
 *  Created on: Dec 1, 2008
 *      Author: imran
 *  Updated in 2010 & 2011 by Ameer
 *  Updated to allow flexible mode configurations: August 2012: M. Freddie Dias <mfdias@ri.cmu.edu>
 */

#ifndef APP_DISPATCHER_H_
#define APP_DISPATCHER_H_

#include <cstdlib>
#include <map>
#include <string>
#include "common/utilities.h"
#include "common/IBTApp.h"


struct ApplicationDispatcher : public IOEventHandler
{
  enum modes {
    FREE_PLAY_ENGLISH,
    FREE_SPELLING_ENGLISH,
    FREE_NUMBERS_ENGLISH,
    LEARN_DOTS_ENGLISH,
    DOT_PRACTICE_ENGLISH,
    LEARN_LETTERS_ENGLISH,
    LETTER_PRACTICE_ENGLISH,
    LEARN_NUMBERS_ENGLISH,
    DOMINOS_ENGLISH,
    HANGMAN_ENGLISH,
    ANIMAL_GAME_ENGLISH,
	  HOUSEHOLD_GAME_ENGLISH,
    ARITHMETIC_PRACTICE_ENGLISH,
    LEARN_LETTERS_HINDI,
    FREE_PLAY_ARABIC,
    FREE_SPELLING_ARABIC,
    FREE_NUMBERS_ARABIC,
    LEARN_DOTS_ARABIC,
    DOT_PRACTICE_ARABIC,
    LEARN_LETTERS_ARABIC,
    LETTER_PRACTICE_ARABIC,
    LEARN_NUMBERS_ARABIC,
    DOMINOS_ARABIC,
    HANGMAN_ARABIC,
    ANIMAL_GAME_ARABIC,
    FREE_PLAY_FRENCH,
    FREE_SPELLING_FRENCH,
    FREE_NUMBERS_FRENCH,
    LEARN_DOTS_FRENCH,
    DOT_PRACTICE_FRENCH,
    LEARN_LETTERS_FRENCH,
    LETTER_PRACTICE_FRENCH,
    LEARN_NUMBERS_FRENCH,
    DOMINOS_FRENCH,
    HANGMAN_FRENCH,
    ANIMAL_GAME_FRENCH,
    FREE_PLAY_SWAHILI,
    FREE_SPELLING_SWAHILI,
    FREE_NUMBERS_SWAHILI,
    LEARN_DOTS_SWAHILI,
    DOT_PRACTICE_SWAHILI,
    LEARN_LETTERS_SWAHILI,
    LETTER_PRACTICE_SWAHILI,
    LEARN_NUMBERS_SWAHILI,
    DOMINOS_SWAHILI,
    HANGMAN_SWAHILI,
    ANIMAL_GAME_SWAHILI
  };
  
  static const int SCROLL_OFF = 0;//Events are interpreted as normal events that are passed to the current application
  static const int BZERO_DOWN = 1;//Is Button0 held down?
  static const int SCROLL_ON = 2; //Events are interpreted as scrolling events - this enables us to scroll

  BrailleTutor& bt;
  IOEventParser& iep;
  IBTApp* current_app;
  Voice teach;
  Voice numbers;
  //modes current_mode;
  unsigned int current_mode_index;
  int switching_modes;
  //int language_mode; //value determined by config file - decides language combination
  bool cfg_file_processed; // so we don't read config file multiple times

  std::vector<modes> modes_list; // stores the indexes to the current subset of modes being used (determined by reading config file)
  
  
  virtual void operator()(std::deque<IOEvent> &events);
  explicit inline ApplicationDispatcher(BrailleTutor &my_bt, IOEventParser &my_iep) : //struct constructor
      bt(my_bt), iep(my_iep),current_app(NULL),teach("./resources/Voice/teacher/"),numbers("./resources/Voice/math_sounds/"),current_mode_index(0),switching_modes(SCROLL_ON),cfg_file_processed(false)
  { 
    // Process the config file to make sure we have a modes list to work with
    // The process config file adds default modes to the list if reading the config fails
    processConfigFile(); 
  }
  bool isSwitchAppEvent(const IOEvent& e);
  bool isScrollEvent(const IOEvent& e);
  void scroll(const IOEvent& e);
  IBTApp* switchApp(const IOEvent& e);

  IBTApp* switchToSelectedMode() const;
  void playSelectedMode() const;
  
  void processConfigFile();
};

#endif /* APP_DISPATCHER_H_ */
