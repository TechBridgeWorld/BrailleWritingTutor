/*
 * timer.cc
 *
 *  Created on: Nov 20, 2008
 *      Author: imran
 */
#include "timer.h"

TimerTask::TimerTask(const DominoGame& my_game, unsigned int t) :
  game(my_game),turn(t)
{
}

void TimerTask::operator()()
{
  BT_sleep(25);
  //sleep(7); //Good for testing the concurrency
  game.notifyTimeoutOccured(turn);
}
