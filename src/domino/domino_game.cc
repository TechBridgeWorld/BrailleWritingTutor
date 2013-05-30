/*
 * domino_game.cc
 *
 *  Created on: Nov 11, 2008
 *      Author: imran
 */
#include <stdlib.h>
#include <time.h>
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_inserter.hpp>
#include <unistd.h>
#include <sstream>
#include "domino_game.h"

unsigned int DominoGame::current_players_index = 0; //Holds the index of the player whose input we are waiting for.
std::vector<DominoPlayer> DominoGame::players;
unsigned int DominoGame::current_turn = 0;

boost::try_mutex DominoGame::mx;

/*
 * This constructor is called by the subclass. The subclass creates the alphabet vector and passes it to DominoGame
 * One BAD alternative would be call a virtual function (which is implemented in the subclass), in this constructor's initalization
 * list. But this is a BAD idea (Item 9, Effective C++)
 */
DominoGame::DominoGame(IOEventParser& my_iep, const std::string& path_to_mapping_file, const std::vector<std::string> my_alphabet, SoundsUtil* my_su, std::string path_to_dominosounds, std::string path_to_othersounds, bool f) :
  IBTApp(my_iep, path_to_mapping_file), game_alphabet(my_alphabet), su(my_su), dominosounds(path_to_dominosounds), othersounds(path_to_othersounds), nomirror(f), iep(my_iep)
{
  start(2);
}

DominoGame::~DominoGame()
{
  stop();
  delete su;
}

void DominoGame::processEvent(IOEvent& e)
{
  //Just print the relevant events for Dominos in order to filter out the rest
  if( e.type == IOEvent::BUTTON || e.type == IOEvent::BUTTON_LETTER )
    printEvent(e);

  //Whenever the user hits Button0 we immediately want the LETTER event to be generated so that he doesnt have to wait for the timeout
  if( e.type == IOEvent::BUTTON && e.button == 0 )
  {
    iep.flushGlyph();
    return; //required? hmm..
  }

  //Check if we received a letter, if so we do the processing
  if( e.type == IOEvent::BUTTON_LETTER && hasStarted() )
  {
    su->sayLetter(othersounds, (std::string) e.letter);//Because we want to give feedback to the user what he entered
    processInput((std::string) e.letter, e.dots); //BTW, the casting works because GlyphMapping has overloaded the std::string() operator
    return;
  }
}

//returns a random number between 0 to n-1
int DominoGame::random(int n) const
{
  srand(time(NULL));
  return rand() % n;
}

DominoPlayer::Side DominoGame::getRandomSide() const
{
  return this->random(2) ? DominoPlayer::LEFT : DominoPlayer::RIGHT; //We are asking for a random number between 0 to 1..
}

void DominoGame::getRandomLetter(std::string& s) const
{
  int r = this->random(game_alphabet.size());
  s.assign(game_alphabet[r]);
}

void DominoGame::start(size_t num_players) const
{
  //std::cout << "    (DEBUG)Starting game" << std::endl;

  //Init the players
  for(size_t i = 0; i < num_players; i++)
  {
    DominoPlayer p(i);
    players.push_back(p);
  }

  //Randomly choose the player who will go first
  current_players_index = this->random(num_players);

  su->saySound(dominosounds, "opening_music_final");
  BT_sleep(1);
  su->saySound(dominosounds, "new_game");

  proceed();
}

void DominoGame::proceed() const
{
  //This check if required if the game already has a winner
  if( !hasStarted() )
    return;

  //Before starting a new turn we pause to give the next player a chance to get ready
  BT_sleep(3.5);

  DominoPlayer& p = getNextPlayer();

  //Randomly assign a side to the next player
  DominoPlayer::Side assigned_side = getRandomSide();
  p.setSide(assigned_side);

  //Randomly assign a letter to the next player
  std::string assigned_letter;
  getRandomLetter(assigned_letter);
  p.setAssignedLetter(assigned_letter);

  //
  char s = (char) assigned_side;
  std::string side;
  if( s == 'L' )
    side.assign("LEFT");
  else
    side.assign("RIGHT");

  std::cout << "\n\nsay:Player" << p.getID() + 1 << " your side is " << side << ". Your new assigned letter is:" << assigned_letter << ". Go!"
      << std::endl;
  playNextTurnSounds(p);

  //
  current_turn++;

  //
  //beginTimer();
}

DominoPlayer& DominoGame::getNextPlayer() const
{
  current_players_index++;
  current_players_index = current_players_index % players.size(); //Ensures our index is always "circular"
  //std::cout << "		(DEBUG)Next player is at index:" << current_players_index << std::endl;
  return players.at(current_players_index);
}

void DominoGame::processInput(std::string input, const unsigned char dot_pattern) const
{
  //Before we process the input, we check if the input arrived too late.
  boost::try_mutex::scoped_try_lock lock(mx);//try to acquire the lock
  if( !lock ) //Unable to acquire the lock, this means that a Timer thread has acquired it (ie, timeout has occured)
  {
    //std::cout << "		(DEBUG)Input arrived too late for Player" << (current_players_index - 1) % players.size() << std::endl;
    return;//We dont further process the Player's input
  }

  if( isValidLetter(input) )
  {
    DominoPlayer& currentPlayer = players.at(current_players_index);
    int old_score = currentPlayer.getCurrentScore();
    int new_score = currentPlayer.updateAndGetScore(input, dot_pattern);
    std::cout << currentPlayer << std::endl;
    if( new_score > old_score )
    {
      su->saySound(dominosounds, "beep_after_input");
    }
    else
    {
      su->saySound(dominosounds, "sad");
    }
    playPlayerScoreSounds(currentPlayer);

    //Now check if the game is over, ie, we have reached the max score
    checkIfGameOver();
  }
  else
  {
    std::cout << "say:Invalid letter" << std::endl;
    su->saySound(dominosounds, "invalid");
  }

  proceed();
}

bool DominoGame::isValidLetter(std::string& letter) const
{
  for(unsigned int i = 0; i < game_alphabet.size(); i++)
    if( letter == game_alphabet[i] )
      return true;

  return false;
}

void DominoGame::stop() const
{
  players.clear();
  current_players_index = 0;
  current_turn = 0;
  //std::cout << "    (DEBUG)Stopping game" << std::endl;
}

bool DominoGame::hasStarted() const
{
  return players.size() > 0;
}

void DominoGame::beginTimer() const
{
  TimerTask t(*this, current_turn);
  boost::thread start(t);
}

void DominoGame::checkIfGameOver() const
{
  DominoPlayer& currentPlayer = players.at(current_players_index);
  if( currentPlayer.getCurrentScore() >= WINNING_SCORE )
  {
    playGameOverSounds();
    stop();
  }
}

void DominoGame::notifyTimeoutOccured(unsigned int turn_at_which_timeout_occured) const
{
  boost::try_mutex::scoped_try_lock lock(mx); //try to acquire a lock

  /*
   * You might wonder why we need the second condition.. well, recall that all Timer
   * threads call this method. We dont want an "irrelevant" thread causing a timeout.
   * For example, a thread that was doing the timing for turn #13 should arrive later and cause
   * a timeout if the game has already progressed to turn #14
   *
   * Additionally note that we are using scoped_locks with RAII, this means that the mutex is automatically
   * unlocked at the end of every turn.. so just successfuly acquiring a lock is not sufficient
   */
  if( lock && turn_at_which_timeout_occured == current_turn )
  {
    std::cout << "say:TIMEOUT OCCURED for Player" << current_players_index << std::endl;
    proceed(); //go to the next turn
  }
}
void DominoGame::playNextTurnSounds(const DominoPlayer& player) const
{

  //say the player's id. Recall that player ids start from 0; but we refer to players starting from 1. For example, if the id is 0 we speak "Player 1 your score is.."
  su->saySound(dominosounds, "player");
  su->sayNumber(othersounds, player.getID() + 1, nomirror);

  //say the player's side
  player.getSide() == DominoPlayer::LEFT ? su->saySound(dominosounds, "left_side") : su->saySound(dominosounds, "right_side");

  //say the player's assigned letter
  su->saySound(dominosounds, "random_letter");
  su->sayLetter(othersounds, player.getAssignedLetter());
}

void DominoGame::playPlayerScoreSounds(const DominoPlayer& player) const
{
  DominoPlayer& currentPlayer = players.at(current_players_index);
  su->saySound(dominosounds, "your_score_is");

  //Converting int to string
  int score = currentPlayer.getCurrentScore();
  std::string score_;
  std::stringstream out;
  out << score;
  score_ = out.str();

  su->saySound(othersounds, score_);
}

void DominoGame::playGameOverSounds() const
{
  su->saySound(dominosounds, "cheer");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishDominoGame::EnglishDominoGame(IOEventParser& my_iep) :
  DominoGame(my_iep, "", createAlphabetVector(), new EnglishSoundsUtil, "./resources/Voice/domino_sounds/", "./resources/Voice/student/", false)
{

}

std::vector<std::string> EnglishDominoGame::createAlphabetVector()
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ArabicDominoGame::ArabicDominoGame(IOEventParser& my_iep) :
      DominoGame(my_iep, "./language_mapping_files/arabic_mapping.txt", createAlphabetVector(), new ArabicSoundsUtil, "./resources/Voice/domino_sounds/", "./resources/Voice/student/", false)
{

}

std::vector<std::string> ArabicDominoGame::createAlphabetVector()
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FrenchDominoGame::FrenchDominoGame(IOEventParser& my_iep) :
      DominoGame(my_iep, "./language_mapping_files/french_mapping.txt", createAlphabetVector(), new FrenchSoundsUtil, "./resources/Voice/domino_sounds/", "./resources/Voice/teacher/", false)
{

}

std::vector<std::string> FrenchDominoGame::createAlphabetVector()
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z")("Ç")("É")("À")("È")("Ù")("Â")("Ê")("Î")("Ô")("Û")("Ë")("Ï")("Ü")("œ");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliDominoGame::KiswahiliDominoGame(IOEventParser& my_iep) :
      DominoGame(my_iep, "", createAlphabetVector(), new KiswahiliSoundsUtil, "./resources/Voice/domino_sounds/", "./resources/Voice/teacher/", false)
{

}

std::vector<std::string> KiswahiliDominoGame::createAlphabetVector()
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("R")("S")("T")("U")("W")("Y")("Z");
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
English2DominoGame::English2DominoGame(IOEventParser& my_iep) :
  DominoGame(my_iep, "./language_mapping_files/english_mapping_nomirror.txt", createAlphabetVector(), new English2SoundsUtil, "./resources/Voice/domino_sounds/", "./resources/Voice/student/", true)
{

}

std::vector<std::string> English2DominoGame::createAlphabetVector()
{
  return boost::assign::list_of("A")("B")("C")("D")("E")("F")("G")("H")("I")("J")("K")("L")("M")("N")("O")("P")("Q")("R")("S")("T")("U")("V")("W")("X")("Y")("Z");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Arabic2DominoGame::Arabic2DominoGame(IOEventParser& my_iep) :
      DominoGame(my_iep, "./language_mapping_files/arabic_mapping_nomirror.txt", createAlphabetVector(), new Arabic2SoundsUtil, "./resources/Voice/domino_sounds/", "./resources/Voice/student/", true)
{

}

std::vector<std::string> Arabic2DominoGame::createAlphabetVector()
{
  return boost::assign::list_of("ا")("ب")("ت")("ث")("ج")("ح")("خ")("د")("ذ")("ر")("ز")("س")("ش")("ص")("ض")("ط")("ظ")("ع")("غ")("ف")("ق")("ك")("ل")("م")("ن")("ه")("و")("ي");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
