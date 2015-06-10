/*
 * player.cc
 *
 *  Created on: Nov 11, 2008
 *      Author: imran
 */

#include <iostream>
#include <sstream>
#include "BrailleTutor.h"
#include "Charset.h"
#include "Dots.h" //only for the dot_string() utility function
#include "domino_player.h"
#include "common/IBTApp.h"

DominoPlayer::DominoPlayer(int id) :
  player_id(id), assigned_letter(""), current_side(NA), current_score(0)
{
}

DominoPlayer::~DominoPlayer()
{
}

void DominoPlayer::setSide(const Side side)
{
  current_side = side;
}

void DominoPlayer::setAssignedLetter(const std::string& letter)
{
  assigned_letter.clear();
  assigned_letter.assign(letter); //btw, could've also used assigned_letter = letter
}

int DominoPlayer::updateAndGetScore(const std::string& chosen_letter, const unsigned char dot_pattern)
{
  //Incase player chooses the same letter that he was assigned, we dont change his score (ie, we treat it as an invalid input)
  if(chosen_letter == assigned_letter)
    return current_score;

  const unsigned char relevant_assigned_dots = extractRelevantDotPattern(assigned_letter, current_side);
  const unsigned char relevant_chosen_dots = extractRelevantDotPattern(chosen_letter, current_side);

  //Check if the player's chosen letter contains the same dots(on his assigned side) as his assigned letter
  if( relevant_assigned_dots == relevant_chosen_dots )
  {
    //current_score = current_score + countDots(dot_pattern);//XXX:Uncomment in REAL version!
    current_score = current_score + 1;//XXX:This is onyl for the ddemo
  }
  else
  {
    //do nothing for now? ie, dont chage his score
  }

  return current_score;
}

/*
 * Extracts those dots from the given letter that fall on the given side.
 * For example, if the given letter is "D" and the side is RIGHT, then the returned dot pattern is 000110 (eventho D's
 * real dot pattern is 100110)
 */
unsigned char DominoPlayer::extractRelevantDotPattern(const std::string& letter, const Side side)
{
  //First, we get the full dot pattern that comprises the letter
  static const BrailleTutorNS::Charset& game_charset = IBTApp::getCurrentCharset();
  const unsigned char dot_pattern = game_charset[letter];

  //Now we extract those dots that lie on the given Side.
  unsigned char relevant_dot_pattern = side == LEFT ? dot_pattern & 0x07 : dot_pattern & 0x38; //0x07 = 00 000111 and 0x38 = 00 111000

  if( 0 )
  {
    std::cout << "    (DEBUG)letter: " << letter << std::endl;
    std::cout << "    (DEBUG)format of dot pattern: " << BrailleTutorNS::dot_string(dot_pattern) << std::endl;
    std::cout << "    (DEBUG)format of relevant dot pattern: " << BrailleTutorNS::dot_string(relevant_dot_pattern) << std::endl;
  }

  return relevant_dot_pattern;
}

//Counts the number of dots in a given dot pattern
int DominoPlayer::countDots(const unsigned char dots)
{
  unsigned char mask = 0x01;
  int counter = 0;
  for(int i = 1; i <= 8; i++)
  {
    if( dots & mask )
    {
      counter++;
    }
    mask = mask << 1;
  }
  return counter;
}

/*
 * Given a dot pattern, this method calculates it's "value" based on the placement of the 1s. Note that
 * the indexing begins at 1, not 0. For example,
 * dot_pattern = 11100111
 * 1s can be found at the following index (starting right to left): 1,2,3,6,7,8
 * Therefore the value is = 1 + 2 + 3 + 6 + 7 + 8
 */
unsigned int DominoPlayer::calculateValueOfDotPattern(const unsigned char dots) const
{
  unsigned char mask = 0x01;
  unsigned int counter = 0;
  for(int i = 1; i <= 8; i++)
  {
    if( dots & mask )
    {
      counter = counter + i;
    }
    mask = mask << 1;
  }

  return counter;
}

int DominoPlayer::getID() const
{
  return player_id;
}

DominoPlayer::Side DominoPlayer::getSide() const
{
  return current_side;
}

int DominoPlayer::getCurrentScore() const
{
  return current_score;
}


const std::string& DominoPlayer::getAssignedLetter() const
{
  return assigned_letter;
}
