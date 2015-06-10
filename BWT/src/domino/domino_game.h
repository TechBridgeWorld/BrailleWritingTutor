/*
 * domino_game.h
 *
 *  Created on: Nov 11, 2008
 *      Author: imran
 */

#ifndef DOMINO_GAME_H_
#define DOMINO_GAME_H_

#include <iostream>
#include <vector>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/language_utils.h"
#include "domino_player.h"
#include "timer.h"
#include "Voice/Voice.h"


class TimerTask;

class DominoGame : public IBTApp
{

public:
  explicit DominoGame(IOEventParser&, const std::string&, const std::vector<std::string>, SoundsUtil*, std::string, std::string, bool);//This constructor is called explicitly by the subclass.
  virtual ~DominoGame();
  virtual void processEvent(IOEvent&);
  void start(size_t) const;
  void stop() const;
  bool hasStarted() const;
  void processInput(std::string, const unsigned char) const;
  void notifyTimeoutOccured(unsigned int) const;

private:
  void proceed() const;
  int random(int to) const;
  DominoPlayer::Side getRandomSide() const;
  void getRandomLetter(std::string&) const;
  DominoPlayer& getNextPlayer() const;
  void beginTimer() const;
  bool isValidLetter(std::string&) const;
  void playNextTurnSounds(const DominoPlayer&) const;
  void playPlayerScoreSounds(const DominoPlayer&) const;
  void playGameOverSounds() const;
  void checkIfGameOver() const;
  

private:
  const std::vector<std::string> game_alphabet; //this vector will be populated using the vector provided by the subclass
  static unsigned int current_players_index;
  static std::vector<DominoPlayer> players;
  static unsigned int current_turn;
  static boost::try_mutex mx;
  static const int WINNING_SCORE = 3;
  bool nomirror;
  SoundsUtil* su;
  Voice dominosounds;
  Voice othersounds;
  IOEventParser& iep; 

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class EnglishDominoGame : public DominoGame
{
public:
  explicit EnglishDominoGame(IOEventParser&);
  ~EnglishDominoGame()
  {
  }

private:
  static std::vector<std::string> createAlphabetVector();
  /*
   * Using a static member function to create the vector
   * BTW if I was using Boost 1.37, I would not need such a function because I could directly use list_of in the vector's
   * constructor, ie, my_vector(assign::list_of(..))
   */

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class ArabicDominoGame : public DominoGame
{
public:
  explicit ArabicDominoGame(IOEventParser&);
  ~ArabicDominoGame()
  {
  }
private:
  static std::vector<std::string> createAlphabetVector();
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class FrenchDominoGame : public DominoGame
{
public:
  explicit FrenchDominoGame(IOEventParser&);
  ~FrenchDominoGame()
  {
  }
private:
  static std::vector<std::string> createAlphabetVector();
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class KiswahiliDominoGame : public DominoGame
{
public:
  explicit KiswahiliDominoGame(IOEventParser&);
  ~KiswahiliDominoGame()
  {
  }
private:
  static std::vector<std::string> createAlphabetVector();
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class English2DominoGame : public DominoGame
{
public:
  explicit English2DominoGame(IOEventParser&);
  ~English2DominoGame()
  {
  }

private:
  static std::vector<std::string> createAlphabetVector();
  /*
   * Using a static member function to create the vector
   * BTW if I was using Boost 1.37, I would not need such a function because I could directly use list_of in the vector's
   * constructor, ie, my_vector(assign::list_of(..))
   */

};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Arabic2DominoGame : public DominoGame
{
public:
  explicit Arabic2DominoGame(IOEventParser&);
  ~Arabic2DominoGame()
  {
  }
private:
  static std::vector<std::string> createAlphabetVector();
};

#endif /* DOMINO_GAME_H_ */
