/*
 * player.h
 *
 *  Created on: Nov 11, 2008
 *      Author: imran
 */

#ifndef DOMINO_PLAYER_H_
#define DOMINO_PLAYER_H_

#include <string>

class DominoPlayer
{
public:
  typedef enum
  {
    LEFT='L', RIGHT='R', NA='N'
  } Side;

public:
  DominoPlayer(int);
  virtual ~DominoPlayer();

  void setSide(const Side side);
  void setAssignedLetter(const std::string& letter);
  int getID() const;
  Side getSide() const;
  int getCurrentScore() const;
  const std::string& getAssignedLetter() const;
  int updateAndGetScore(const std::string& chosen_letter, const unsigned char dot_pattern);

  friend std::ostream& operator <<(std::ostream& os, const DominoPlayer& p);

private:
  unsigned int calculateValueOfDotPattern(const unsigned char dots) const;
  unsigned char extractRelevantDotPattern(const std::string& , const Side );
  int countDots(const unsigned char);

private:
  unsigned int player_id;
  std::string assigned_letter;
  Side current_side;
  unsigned int current_score;
};

inline std::ostream& operator <<(std::ostream& os, const DominoPlayer& p) //FIXME: Make this friend?
{
  //os << "    (DEBUG)Player id is:" << p.player_id << ",score:" << p.current_score << ", side:" << (char)p.current_side << ", assigned letter:" << p.assigned_letter;
  os << "    Player" << p.player_id+1 << ". Your score is:" << p.current_score;
  return os;
}

#endif /* DOMINO_PLAYER_H_ */
