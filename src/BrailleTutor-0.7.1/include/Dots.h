#ifndef _LIBBT_DOTS_H_
#define _LIBBT_DOTS_H_
/*
 * Braille Tutor interface library
 * Dots.h, started 16 July 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Defines and a routine for (some) bit operations on Braille cell dots.
 */

#include <string>

namespace BrailleTutorNS {

//! Bitmap mask of IOEvent::dots for braille cell dot 1
static const unsigned char DOT_1 = 0x01;
//! Bitmap mask of IOEvent::dots for braille cell dot 2
static const unsigned char DOT_2 = 0x02;
//! Bitmap mask of IOEvent::dots for braille cell dot 3
static const unsigned char DOT_3 = 0x04;
//! Bitmap mask of IOEvent::dots for braille cell dot 4
static const unsigned char DOT_4 = 0x08;
//! Bitmap mask of IOEvent::dots for braille cell dot 5
static const unsigned char DOT_5 = 0x10;
//! Bitmap mask of IOEvent::dots for braille cell dot 6
static const unsigned char DOT_6 = 0x20;
//! Bitmap mask of IOEvent::dots for braille cell dot 7 (expansion?)
static const unsigned char DOT_7 = 0x40;
//! Bitmap mask of IOEvent::dots for braille cell dot 8 (expansion?)
static const unsigned char DOT_8 = 0x80;

//! Returns cell dot bitmap mask for dot number minus one
inline unsigned char dot_mask(const unsigned int &dotno)
{
  switch(dotno) {
  case 0: return DOT_1;
  case 1: return DOT_2;
  case 2: return DOT_3;
  case 3: return DOT_4;
  case 4: return DOT_5;
  case 5: return DOT_6;
  case 6: return DOT_7;
  case 7: return DOT_8;
  default: return (unsigned char) 0x00;
  }
}

//! Returns a diagnostic string representation of a braille cell dot pattern
inline std::string dot_string(const unsigned char &dots)
{
  std::string dotstr("........");
  unsigned char mask = 0x01;
  for(unsigned int i=0; i<8; ++i) {
    if(mask & dots) dotstr[i] = 'o';
    mask <<= 1;
  }
  return dotstr;
}

} // namespace BrailleTutorNS

#endif
