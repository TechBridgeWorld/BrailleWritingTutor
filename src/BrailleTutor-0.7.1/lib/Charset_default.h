#ifndef _LIBBT_CHARSET_DEFAULT_H_
#define _LIBBT_CHARSET_DEFAULT_H_
/*
 * Braille Tutor interface library
 * Charset_default.h, started 25 May 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Contains data for several default mappings between letters/strings and
 * cell dot patterns. The library will use these mappings unless they are
 * overridden in user code. The particular character set is a combination of
 * uppercase unaccented roman letters in standard braille and some Grade 2
 * contractions. It should probably be replaced with something more rigorous.
 */

#include <string>

#include "Dots.h"

namespace BrailleTutorNS {

//! Name of the default character set
static const std::string
  default_charset_name("(BUILTIN) BT Roman Letters v0.1");

//! The dot patters in the default character set
static const unsigned char default_charset_dots[] = {
  DOT_1,						// A
  DOT_1 | DOT_2,					// B
  DOT_1 |                 DOT_4,			// C
  DOT_1 |                 DOT_4 | DOT_5,		// D
  DOT_1 |                         DOT_5,		// E
  DOT_1 | DOT_2 |         DOT_4,			// F
  DOT_1 | DOT_2 |         DOT_4 | DOT_5,		// G
  DOT_1 | DOT_2 |                 DOT_5,		// H
          DOT_2 |         DOT_4,			// I
          DOT_2 |         DOT_4 | DOT_5,		// J
  DOT_1 |         DOT_3,				// K
  DOT_1 | DOT_2 | DOT_3,				// L
  DOT_1 |         DOT_3 | DOT_4,			// M
  DOT_1 |         DOT_3 | DOT_4 | DOT_5,		// N
  DOT_1 |         DOT_3 |         DOT_5,		// O
  DOT_1 | DOT_2 | DOT_3 | DOT_4,			// P
  DOT_1 | DOT_2 | DOT_3 | DOT_4 | DOT_5,		// Q
  DOT_1 | DOT_2 | DOT_3 |         DOT_5,		// R
          DOT_2 | DOT_3 | DOT_4,			// S
          DOT_2 | DOT_3 | DOT_4 | DOT_5,		// T
  DOT_1 |         DOT_3 |                 DOT_6,	// U
  DOT_1 | DOT_2 | DOT_3 |                 DOT_6,	// V
          DOT_2 |         DOT_4 | DOT_5 | DOT_6,	// W
  DOT_1 |         DOT_3 | DOT_4 |         DOT_6,	// X
  DOT_1 |         DOT_3 | DOT_4 | DOT_5 | DOT_6,	// Y
  DOT_1 |         DOT_3 |         DOT_5 | DOT_6,	// Z

                  DOT_3,				// '
          DOT_2 |                 DOT_5 | DOT_6,	// .
          DOT_2,					// ,
          DOT_2 | DOT_3,				// ;
          DOT_2 | DOT_3 |         DOT_5,		// !
          DOT_2 | DOT_3 |                 DOT_6,	// `` (open quote)
                  DOT_3 |         DOT_5 | DOT_6,	// '' (close quote)
          DOT_2 | DOT_3 |         DOT_5 | DOT_6,	// <bracket/>
                                  DOT_5 | DOT_6,	// -

  DOT_1 | DOT_2 | DOT_3 | DOT_4 |         DOT_6,	// AND
  DOT_1 |                                 DOT_6,	// CH
                  DOT_3 | DOT_4 |         DOT_6,	// ING
  DOT_1 |                 DOT_4 |         DOT_6,	// SH
                  DOT_3 | DOT_4,			// ST
  DOT_1 |                 DOT_4 | DOT_5 | DOT_6,	// TH
};

//! The letters in the default character set
static const char *default_charset_letters[] = {
  "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L",
  "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X",
  "Y", "Z",

  "'", ".", ",", ";", "!", "``", "''", "<bracket/>", "-",

  "AND", "CH", "ING", "SH", "ST", "TH"
};

} // namespace BrailleTutorNS

#endif
