/*
 * app_dispatcher.cc
 *
 *  Created on: Nov 30, 2008
 *      Author: imran
 */

#include "app_dispatcher.h"

#include "domino/domino_game.h"
#include "dot_scaffold/dot_scaffold.h"
#include "learn_dots/learn_dots.h"
#include "dot_practice/dot_practice.h"
#include "learn_letters/learn_letters.h"
#include "letter_scaffold/letter_scaffold.h"
#include "letter_practice/letter_practice.h"
#include "animal/animal.h"
#include "hangman/hangman.h"
#include "learn_numbers/learn_numbers.h"
#include "number_scaffold/number_scaffold.h"
#include "musicmaker/musicmaker.h"

#define SWITCH_OFF 0
#define BZERO_DOWN 1
#define SWITCH_ON 2

using namespace BrailleTutorNS;
int switching_modes = SWITCH_ON;

enum modes { FREE_SPELLING, LEARN_DOTS, DOT_PRACTICE, LEARN_LETTERS, LETTER_PRACTICE, MUSICMAKER, ANIMAL_GAME, ANIMAL_GAMEKIS };
Voice teach("./resources/Voice/teacher/");
modes current_mode = FREE_SPELLING;

void ApplicationDispatcher::operator()(std::deque<IOEvent> &events)
{
  while( !events.empty() )
  {
    IOEvent e = events.front();
    events.pop_front();

    //If Switching is Off, detect when button 0 is down
    //If BZero comes back up and we're not in switching mode yet, return to the previous state
    if(e.type == IOEvent::BUTTON_DOWN && e.button == 0 && switching_modes == SWITCH_OFF){
      switching_modes = BZERO_DOWN;
    }
    else if(e.type == IOEvent::BUTTON_UP && e.button == 0 && switching_modes == BZERO_DOWN){
      switching_modes = SWITCH_OFF;
    }

    //Check if the event is a trigger to switch to another app
    if( isSwitchAppEvent(e) )
    {
      delete current_app;//stop the running app (calls the destructor)
      current_app = switchApp(e);//make the switch

      return;
    }

    if(switching_modes != SWITCH_ON) //it is a normal event, so just pass it to whatever app is currently running
    {
      if( current_app != NULL )
        current_app->processEvent(e);
    }
  }
}

/*
 *  If Button 0 is down and the user presses a button, we're mode switching
 */
bool ApplicationDispatcher::isSwitchAppEvent(const IOEvent& e) const
{
    if(e.type != IOEvent::BUTTON_DOWN)
      return false;
    if(switching_modes == SWITCH_ON)
      return true;
    if(switching_modes == BZERO_DOWN){
      if(e.button != 0){
        switching_modes = SWITCH_ON; 
        return true;
      }
    }
    return false;
}

//TODO: The implementation of this method will change when we finalize how apps are switched
/*
 * This function determines which app to switch to, and then makes the switch by instantiating a class of the
 * determined app.
 */
IBTApp* ApplicationDispatcher::switchApp(const IOEvent& e) const
{
	

  if(e.button >= 1 && e.button <= 3){
    if(current_mode == FREE_SPELLING)
      current_mode = ANIMAL_GAMEKIS;
    else
      current_mode = (modes) (current_mode - 1);
  }
  else if(e.button >= 4 && e.button <= 6){
    if(current_mode == ANIMAL_GAMEKIS)
      current_mode = FREE_SPELLING;
    else
      current_mode = (modes) (current_mode + 1);
  }

  if(e.button == 0 && e.type == IOEvent::BUTTON_DOWN) {
    switching_modes = SWITCH_OFF;
    switch( current_mode )
    {
      case LEARN_DOTS:
        return new EnglishLearnDots(iep);
      case DOT_PRACTICE:
        return new EnglishDotPractice(iep);
      case LEARN_LETTERS:
        return new EnglishLearnLetters(iep);
      case LETTER_PRACTICE:
        return new EnglishLetterPractice(iep);
      case FREE_SPELLING:
        return new EnglishLetterScaffold(iep);
      case ANIMAL_GAME:
        return new EnglishAnimal(iep);
      case ANIMAL_GAMEKIS:
        return new KiswahiliAnimal(iep);
      case MUSICMAKER:
	    return new EnglishMusicMaker(iep);
    }
  }

  //Currently we determine the target app based on the cell
  switch( current_mode )
  {
    case LEARN_DOTS:
	  teach.say("learn dots.wav");
      printf("Switching to English Learn Dots\n");
      break;
	case DOT_PRACTICE:
	  teach.say("dot practice.wav");
      printf("Switching to English Dot Practice\n");
      break;
    case LEARN_LETTERS:
	  teach.say("learn letters.wav");
      printf("Switching to English Learn Letters\n");
      break;
    case LETTER_PRACTICE:
	  teach.say("letter practice.wav");
      printf("Switching to English Letter Practice\n");
      break;
    case FREE_SPELLING:
	  teach.say("free spelling.wav");
      printf("Switching to English Free Spelling\n");
      break;
    case ANIMAL_GAME:
	  teach.say("animal game.wav");
      printf("Switching to English Animal Game\n");
      break;
    case ANIMAL_GAMEKIS:
      teach.say("animal game_kiswahili.wav");
      printf("Switching to Kiswahili Animal Game\n");
      break;
    case MUSICMAKER:
	  teach.say("good.wav");
	  printf("Switching to Music Maker\n");
	  break;
  }
  return NULL;
}
