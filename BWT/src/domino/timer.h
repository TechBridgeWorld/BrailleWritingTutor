/*
 * timer.h
 *
 *  Created on: Nov 20, 2008
 *      Author: imran
 */

#ifndef TIMER_H_
#define TIMER_H_

#if defined(BT_LINUX)
	#include <time.h>
#elif defined(BT_WINDOWS)
	#include <Windows.h>
#endif

#include "domino_game.h"

class DominoGame;

class TimerTask
{
public:
  TimerTask(const DominoGame&, unsigned int);
  ~TimerTask()
  {
    //std::cout<<"		(DEBUG)Inside TimerTask destructor"<<std::endl;
  }
  void operator()();

private:
  const DominoGame& game;
  unsigned int turn; //the turn at which this thread was invoked
};

#endif /* TIMER_H_ */
