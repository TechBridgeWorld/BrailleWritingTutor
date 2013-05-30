/*
 * Braille Tutor inferface library
 * Charset.cc, started 25 May 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Implements Charset.h, particularly I/O code and the initialization
 * of Charset objects with the default character set.
 */

#include "Types.h"
#include "Charset.h"
#include "StateMachine.h"

#include "Charset_default.h"

#include <map>
#include <cctype>
#include <cassert>
#include <fstream>
#include <stdint.h>
#include <iostream>

namespace BrailleTutorNS {

//////////////////////////////////////////////////////
//// UTF-8 <-> "wide character string" conversion ////
//////////////////////////////////////////////////////

//! Homerolled templated strlen; real strlen is only for chars.
template <typename T>
inline static unsigned int local_strlen(const T *str_T)
{ unsigned int len = 0;
  while(*str_T++) ++len;
  return len; }

//! Homerolled templated strcpy; real strcpy is only for chars.
template <typename T>
inline static void local_strcpy(T *dest, const T *src)
{ while((*dest++ = *src++)); }

//! Decodes the next few characters as one UTF-8 character

//! This routine examines the next few bytes in the furnished null-terminated
//! unsigned char string and determines how many of them make up the next
//! UTF-8 character. It then decodes the/those next byte/s into a 32-bit
//! unsigned integer value. Returns the number of characters it took from
//! the input character string.
static unsigned int local_utf8_decode_char(const uint8_t *str_utf8,
					   uint32_t &ch_32)
{
  // Test first for a four-byte coding. Have to test bytes separately in case
  // we run up against the null terminator.
  if(((str_utf8[0] & (uint8_t) 0xf8) == (uint8_t) 0xf0) &&
     ((str_utf8[1] & (uint8_t) 0xc0) == (uint8_t) 0x80) &&
     ((str_utf8[2] & (uint8_t) 0xc0) == (uint8_t) 0x80) &&
     ((str_utf8[3] & (uint8_t) 0xc0) == (uint8_t) 0x80)) {
    ch_32 = (((uint32_t) str_utf8[0] & 0x00000007u) << 18) |
	    (((uint32_t) str_utf8[1] & 0x0000003fu) << 12) |
	    (((uint32_t) str_utf8[2] & 0x0000003fu) <<  6) |
	     ((uint32_t) str_utf8[3] & 0x0000003fu);
    return 4;
  }

  // Test next for a three byte coding.
  if(((str_utf8[0] & (uint8_t) 0xf0) == (uint8_t) 0xe0) &&
     ((str_utf8[1] & (uint8_t) 0xc0) == (uint8_t) 0x80) &&
     ((str_utf8[2] & (uint8_t) 0xc0) == (uint8_t) 0x80)) {
    ch_32 = (((uint32_t) str_utf8[0] & 0x0000000fu) << 12) |
	    (((uint32_t) str_utf8[1] & 0x0000003fu) <<  6) |
	     ((uint32_t) str_utf8[2] & 0x0000003fu);
    return 3;
  }

  // Two bytes?
  if(((str_utf8[0] & (uint8_t) 0xe0) == (uint8_t) 0xc0) &&
     ((str_utf8[1] & (uint8_t) 0xc0) == (uint8_t) 0x80)) {
    ch_32 = (((uint32_t) str_utf8[0] & 0x0000001fu) << 6) |
	     ((uint32_t) str_utf8[1] & 0x0000003fu);
    return 2;
  }

  // One byte
  ch_32 = (uint32_t) str_utf8[0] & 0x0000007f;
  return 1;
}

//! Injects the furnished 32-bit "character" into a UTF-8 string. Returns
//! the number of 8-bit characters inserted. In the current implementation,
//! it can't be any more than 4.
static unsigned int local_utf8_encode_ch32(const uint32_t &ch_32,
					   uint8_t *str_utf8)
{
  // Test first for a four-byte coding
  if((ch_32 >= 0x10000) && (ch_32 <= 0x10ffff)) {
    *str_utf8++ = (uint8_t) 0xf0 | (uint8_t) ((ch_32 & 0x001c0000u) >> 18);
    *str_utf8++ = (uint8_t) 0x80 | (uint8_t) ((ch_32 & 0x0003f000u) >> 12);
    *str_utf8++ = (uint8_t) 0x80 | (uint8_t) ((ch_32 & 0x00000fc0u) >>  6);
    *str_utf8++ = (uint8_t) 0x80 | (uint8_t)  (ch_32 & 0x0000003fu);
    return 4;
  }

  // Test next for a three-byte coding
  if(((ch_32 >=  0x800) && (ch_32 <= 0xd7ff)) ||
     ((ch_32 >= 0xe000) && (ch_32 <= 0xffff))) {
    *str_utf8++ = (uint8_t) 0xe0 | (uint8_t) ((ch_32 & 0x0000f000u) >> 12);
    *str_utf8++ = (uint8_t) 0x80 | (uint8_t) ((ch_32 & 0x00000fc0u) >>  6);
    *str_utf8++ = (uint8_t) 0x80 | (uint8_t)  (ch_32 & 0x0000003fu);
    return 3;
  }

  // Two bytes?
  if((ch_32 >= 0x80) && (ch_32 <= 0x7ff)) {
    *str_utf8++ = (uint8_t) 0xc0 | (uint8_t) ((ch_32 & 0x000007c0u) >> 6);
    *str_utf8++ = (uint8_t) 0x80 | (uint8_t)  (ch_32 & 0x0000003fu);
    return 2;
  }

  // One byte?
  if((ch_32 >= 0) && (ch_32 <= 0x7f)) {
    *str_utf8++ = (uint8_t) (ch_32 & 0x0000007fu);
    return 1;
  }

  // Otherwise, an invalid char. Encode nothing.
  return 0;
}

//! Homerolled UTF-8 -> 32 bit int string decoder. Returns strlen of str_32.
//! Allocates a new decoded string in str_32; deallocate with delete[].
static unsigned int local_utf8_decode(const uint8_t *str_utf8,uint32_t* &str_32)
{
  uint32_t *scratch = new uint32_t[local_strlen(str_utf8) + 1];
  uint32_t *sc_runner = scratch;

  do { str_utf8 += local_utf8_decode_char(str_utf8, *sc_runner); }
  while(*sc_runner++);

  const unsigned int my_strlen = local_strlen(scratch);
  str_32 = new uint32_t[my_strlen + 1];
  local_strcpy(str_32, scratch);

  delete[] scratch;
  return my_strlen;
}

//! Homerolled 32 bit int string -> UTF-8 encoder. Returns strlen of str_utf8.
//! Allocates a new encoded string in str_32; deallocate with delete[].
static unsigned int local_utf8_encode(const uint32_t *str_32,uint8_t* &str_utf8)
{
  uint8_t *scratch = new uint8_t[local_strlen(str_32)*4 + 1];
  uint8_t *sc_runner = scratch;

  do { sc_runner += local_utf8_encode_ch32(*str_32, sc_runner); }
  while(*str_32++);

  const unsigned int my_strlen = local_strlen(scratch);
  str_utf8 = new uint8_t[my_strlen + 1];
  local_strcpy(str_utf8, scratch);

  delete[] scratch;
  return my_strlen;
}


/////////////////////////////////////
//// GlyphMapping implementation ////
/////////////////////////////////////

// Equality test for GlyphMapping objects (compares the str member)
bool GlyphMapping::equals(const GlyphMapping &gm) const
{
  const uint8_t *my_rnr = str.get();
  const uint8_t *gm_rnr = gm.str.get();

  for(;;) {
    if(!*my_rnr) {
      if(!*gm_rnr) return true;
      else return false;
    }
    else if(!*gm_rnr) return false;
    else if(*my_rnr != *gm_rnr) return false;

    ++my_rnr;
    ++gm_rnr;
  }

  return true; // should never get here
}

// Comparison for GlyphMapping objects (compares the str_w member)
bool GlyphMapping::lt(const GlyphMapping &gm) const
{
  const uint32_t *my_rnr = str_w.get();
  const uint32_t *gm_rnr = gm.str_w.get();

  for(;;) {
    if(!*my_rnr) {
      if(!*gm_rnr) return false; // strings equal
      else return true; // we're a prefix of gm
    }
    else if(!*gm_rnr) return false; // gm is a prefix of us
    else if(*my_rnr > *gm_rnr) return false; // gm is < us.
    else if(*my_rnr < *gm_rnr) return true;  // gm is > us.

    ++my_rnr;
    ++gm_rnr;
  }

  return true; // should never get here.
}

// Initializes GlyphMapping from a character string
void GlyphMapping::initFromStr(const uint8_t *my_str)
{
  str.reset(new uint8_t[local_strlen(my_str) + 1]);
  local_strcpy(str.get(), my_str);

  uint32_t *my_str_w;
  local_utf8_decode(str.get(), my_str_w);
  str_w.reset(my_str_w);
}

// Constructor: converts an array of 32-bit ints to a GlyphMapping.
GlyphMapping::GlyphMapping(const uint32_t *my_str_w)
{
  str_w.reset(new uint32_t[local_strlen(my_str_w) + 1]);
  local_strcpy(str_w.get(), my_str_w);

  uint8_t *my_str;
  local_utf8_encode(str_w.get(), my_str);
  str.reset(my_str);
}

// Duplicates this GlyphMapping s.t. the pointers point at new copies of
// the string data.
GlyphMapping GlyphMapping::dup() const
{
  GlyphMapping my_dup;

  my_dup.str.reset(new uint8_t[local_strlen(str.get() + 1)]);
  my_dup.str_w.reset(new uint32_t[local_strlen(str_w.get() + 1)]);

  local_strcpy(my_dup.str.get(), str.get());
  local_strcpy(my_dup.str_w.get(), str_w.get());

  return my_dup;
}


///////////////////////////////
//// Default character set ////
///////////////////////////////

//! Make the default character set
inline Charset make_default_charset()
{
  Charset charset(default_charset_name);

  const unsigned int num_chars = sizeof(default_charset_dots) / sizeof(char);
  for(unsigned int i=0; i<num_chars; ++i)
    charset.set(default_charset_dots[i], (uint8_t*) default_charset_letters[i]);

  return charset;
}

// An unmodifiable instance of the default charset
static const Charset default_charset = make_default_charset();


////////////////////////////////
//// Charset implementation ////
////////////////////////////////

// Sets or resets an individual mapping between a dot pattern and a Unicode
// string/letter. To clear a mapping, either set the first parameter to
// INVALID_DOTS or the second parameter to the empty wstring.
void Charset::set(const unsigned char &dots, const GlyphMapping &letter)
{
  // Useful variable
  std::map<GlyphMapping, unsigned char>::iterator i;

  // The user wants to erase an entry, indexed by the letter.
  if(dots == INVALID_DOTS) {
    i = letters_to_dots.find(letter);

    // Nothing doing---no such entry
    if(i == letters_to_dots.end()) return;

    // Erase the entry
    dots_to_letters[i->second] = GlyphMapping();
    letters_to_dots.erase(i);
  }
  // The user wants to erase an entry, indexed by the dot pattern
  else if(letter.isEmpty()) {
    // Nothing doing---no such entry
    if(dots_to_letters[dots].isEmpty()) return;

    // Look up the counterpart in the reverse mapping
    i = letters_to_dots.find(dots_to_letters[dots]);
    assert(i != letters_to_dots.end());

    // Erase the entry
    dots_to_letters[dots] = GlyphMapping();
    letters_to_dots.erase(i);
  }
  // The user genuinely wants to change a mapping or insert a new one
  else {
    dots_to_letters[dots] = letter;
    letters_to_dots[letter] = dots;
  }
}

// Facilitates lookup/retrieval of mirrored dot patterns
DotsMirror Charset::mir() const { return DotsMirror(*this); }

// Returns a const reference to the default character set
const Charset &Charset::defaultCharset() { return default_charset; }


////////////////////////////////
//// File I/O State Machine ////
////////////////////////////////

//! Input type for File IO state machines---characters read from the input
typedef uint8_t FIOSM_inputT;

//! Output type for File IO state machines---boolean value

//! The File IO state machine emits true iff it is finished processing the
//! current input string (end of file or charset record reached).
typedef bool FIOSM_outputT;

//! Data type for File IO state machines

//! Stores temporary information while the state machine parses input from the
//! charset data stream (file).
struct FIOSM_dataT {
  //! Character set we're modifying as we load the file
  Charset *charset;
  //! Scratch space for one character
  uint8_t tmpchr;
  //! Vector for multiple-character
  std::vector<uint8_t> tmpchrs;
  //! Current input line number (1-indexed)
  unsigned int lineno;
  //! Line numbers (1-indexed) of lines containing syntax errors
  std::vector<unsigned int> error_lines;

  //! Adds the current line number to the list of lines with errors
  inline void markError() { error_lines.push_back(lineno); }

  //! No-arg ctor sets tmpchr to '\0'
  inline FIOSM_dataT() : charset(NULL), tmpchr('\0'), lineno(1) { }
};

//! Name type for File IO state machines---just strings
typedef std::string FIOSM_stateNameT;

//! State abstract base type for File IO state machines
typedef SMState<FIOSM_inputT, FIOSM_outputT,
		FIOSM_dataT, FIOSM_stateNameT> FIO_State;
//! State machine abstract base type for BrailleTutor state machines.
typedef StateMachine<FIOSM_inputT, FIOSM_outputT,
		     FIOSM_dataT, FIOSM_stateNameT> FIO_StateMachine;

//! Useful abbreviation for the end of each state object
#define STATE_END(name) \
inline virtual name *clone() const { return new name(*this); } \
inline virtual ~name() { }

//! Initial state. Loop and look for the "!UTF-8 BEGIN" note
struct FIOS_Hdr : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Hdr"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = getName(); // may change later
    out  = true;      // this too.

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case '!': data.tmpchr = in; break;
    case 'U':
      if(data.tmpchr == '!') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'T':
      if(data.tmpchr == 'U') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'F':
      if(data.tmpchr == 'T') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case '-':
      if(data.tmpchr == 'F') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case '8':
      if(data.tmpchr == '-') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case ' ':
      if(data.tmpchr == '8') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'B':
      if(data.tmpchr == ' ') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'E':
      if(data.tmpchr == 'B') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'G':
      if(data.tmpchr == 'E') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'I':
      if(data.tmpchr == 'G') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'N':
      if(data.tmpchr == 'I') dest = "WS1"; else data.tmpchr = '\0';  break;
    // Handle line counting
    case '\r':
      if(data.tmpchr != '\n') { ++data.lineno; data.tmpchr = in; } break;
    case '\n':
      if(data.tmpchr != '\r') { ++data.lineno; data.tmpchr = in; } break;
    default: data.tmpchr = '\0'; break;
    }
  }

STATE_END(FIOS_Hdr)
};

//! A whitespace skipping state.

//! The name of this whitespace-skipping state is configurable, the jump
//! target is configurable, and the target to jump to on error (detecting an
//! end of line) is too. Clears the data.tmpchrs member and then inserts the
//! first non-whitespace character.
struct FIOS_Whitespace : public FIO_State {
  std::string my_name, jump_name, err_name;

  inline virtual const FIOSM_stateNameT getName() const { return my_name; }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = getName(); // may change later
    out  = true;      // this too
    data.tmpchrs.clear();

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case  ' ':
    case '\t': break;
    case '\n':
    case '\r':
      ++data.lineno; data.tmpchr = in; // handle line counting
      data.markError(); data.tmpchrs.clear(); dest = err_name; break;
    default:   data.tmpchrs.push_back(in); dest = jump_name; break;
    }
  }

  //! Constructor: specify state name, name of state to jump to on
  //! non_whitespace, and name of state to jump to on error.
  inline FIOS_Whitespace(const std::string &my_my_name, 
			 const std::string &my_jump_name,
			 const std::string &my_err_name)
  : my_name(my_my_name), jump_name(my_jump_name), err_name(my_err_name) { }

STATE_END(FIOS_Whitespace)
};

//! State builds up the name of the charset
struct FIOS_Nam : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Nam"); }
 
  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = getName(); // may change later
    out  = true;      // this too

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case '\n':
    case '\r': {
      // Erase old character set data
      data.charset->clear();

      // Prepare to set new character set name
      uint8_t *newname = new uint8_t[data.tmpchrs.size() + 1];
      newname[data.tmpchrs.size()] = '\0';
      for(unsigned int i=0; i<data.tmpchrs.size(); ++i)
	newname[i] = data.tmpchrs[i];

      // Set new character set name
      data.charset->setName(newname);

      // Increment line number.
      // Note last char so we don't count redundantly for CR/LF.
      ++data.lineno;
      data.tmpchr = in;

      // Jump to new line state, other cleanup
      delete[] newname;
      data.tmpchrs.clear();
      dest = "NewLine";
      break;
    }
    default:   data.tmpchrs.push_back(in); break;
    }
  }

STATE_END(FIOS_Nam)
};

//! New line state: expects a new mapping or the end of the mapping block.
struct FIOS_NewLine : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("NewLine"); }
 
  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = getName(); // may change later
    out  = true;      // this too

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case  '0': dest = "Hx1"; break;
    case  '1': case '2': case '3': case '4':
    case  '5': case '6': case '7': case '8':
      data.tmpchrs.clear(); data.tmpchrs.push_back(in); dest = "CDL"; break;
    case 0xe2: dest = "Uni1"; break;
    case  '!': dest = "Ftr"; data.tmpchr = in; break;
    // Handle line counting
    case '\r':
      if(data.tmpchr != '\n') { ++data.lineno; data.tmpchr = in; } break;
    case '\n':
      if(data.tmpchr != '\r') { ++data.lineno; data.tmpchr = in; } break;
    // On error skip to the next line
    default:
   data.markError(); dest = "ErrEOL"; break;
    }
  }

STATE_END(FIOS_NewLine)
};

//! Input error: skip to the end of the line and go back to NewLine
struct FIOS_ErrEOL : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("ErrEOL"); }
 
  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = getName(); // may change later
    out  = true;      // this too

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case '\r':
    case '\n':
      ++data.lineno;
      dest = "NewLine"; data.tmpchr = in; break;
    default: break;
    }
  }

STATE_END(FIOS_ErrEOL)
};

//! The 'x' in an 0x.. hex dot mask
struct FIOS_Hx1 : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Hx1"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = "Hx2"; // may change later
    out  = true;  // this too

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case '\r':
    case '\n':
      ++data.lineno; data.markError();
      dest = "NewLine"; data.tmpchr = in; break;
    case  'x':
    case  'X': break;
    default:   data.markError(); dest = "ErrEOL"; break;
    }
  }

STATE_END(FIOS_Hx1)
};

//! The first (most significant) hex digit in an 0x.. hex dot mask
struct FIOS_Hx2 : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Hx2"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = "Hx3"; // may change later
    out  = true;  // this too

    switch(tolower(in)) {
    case '\0': data.markError(); out = false; break;
    case '0': data.tmpchr = 0x00; break; case '1': data.tmpchr = 0x10; break;
    case '2': data.tmpchr = 0x20; break; case '3': data.tmpchr = 0x30; break;
    case '4': data.tmpchr = 0x40; break; case '5': data.tmpchr = 0x50; break;
    case '6': data.tmpchr = 0x60; break; case '7': data.tmpchr = 0x70; break;
    case '8': data.tmpchr = 0x80; break; case '9': data.tmpchr = 0x90; break;
    case 'a': data.tmpchr = 0xa0; break; case 'b': data.tmpchr = 0xb0; break;
    case 'c': data.tmpchr = 0xc0; break; case 'd': data.tmpchr = 0xd0; break;
    case 'e': data.tmpchr = 0xe0; break; case 'f': data.tmpchr = 0xf0; break;
    case '\r':
    case '\n':
      ++data.lineno; data.markError();
      dest = "NewLine"; data.tmpchr = in; break;
    default:   data.markError(); dest = "ErrEOL"; break;
    }
  }

STATE_END(FIOS_Hx2)
};

//! The second (least significant) hex digit in an 0x.. hex dot mask
struct FIOS_Hx3 : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Hx3"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = "WS2"; // may change later
    out  = true;  // this too

    switch(tolower(in)) {
    case '\0': data.markError(); out = false; break;
    case '0': data.tmpchr |= 0x00; break; case '1': data.tmpchr |= 0x01; break;
    case '2': data.tmpchr |= 0x02; break; case '3': data.tmpchr |= 0x03; break;
    case '4': data.tmpchr |= 0x04; break; case '5': data.tmpchr |= 0x05; break;
    case '6': data.tmpchr |= 0x06; break; case '7': data.tmpchr |= 0x07; break;
    case '8': data.tmpchr |= 0x08; break; case '9': data.tmpchr |= 0x09; break;
    case 'a': data.tmpchr |= 0x0a; break; case 'b': data.tmpchr |= 0x0b; break;
    case 'c': data.tmpchr |= 0x0c; break; case 'd': data.tmpchr |= 0x0d; break;
    case 'e': data.tmpchr |= 0x0e; break; case 'f': data.tmpchr |= 0x0f; break;
    case '\r':
    case '\n':
      ++data.lineno; data.markError();
      dest = "NewLine"; data.tmpchr = in; break;
    default:   data.markError(); dest = "ErrEOL"; break;
    }
  }

STATE_END(FIOS_Hx3)
};

//! Digit in a comma-delimited list of "on" Braille dots. Actually, commas
//! are optional. You can just use the digits 0-8.
struct FIOS_CDL : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("CDL"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = "CDL"; // may change later
    out  = true;  // this too

    switch(tolower(in)) {
    case '\0': data.markError(); out = false; break;
    case  ',': break;
    case  '1': case '2': case '3': case '4':
    case  '5': case '6': case '7': case '8': data.tmpchrs.push_back(in); break;
    case  ' ':
    case '\t':
      // Whitespace. Gotta convert stored digits to Braille dots
      data.tmpchr = 0;
      for(unsigned int i=0; i<data.tmpchrs.size(); ++i)
	switch(data.tmpchrs[i]) {
	case '1': data.tmpchr |= DOT_1; break;
	case '2': data.tmpchr |= DOT_2; break;
	case '3': data.tmpchr |= DOT_3; break;
	case '4': data.tmpchr |= DOT_4; break;
	case '5': data.tmpchr |= DOT_5; break;
	case '6': data.tmpchr |= DOT_6; break;
	case '7': data.tmpchr |= DOT_7; break;
	case '8': data.tmpchr |= DOT_8; break;
	}
      data.tmpchrs.clear();
      dest = "WS2";
      break;
    case '\r':
    case '\n':
      ++data.lineno; data.markError();
      dest = "NewLine"; data.tmpchr = in; break;
    default:   data.markError(); dest = "ErrEOL"; break;
    }
  }

STATE_END(FIOS_CDL)
};

//! The second octet in a UTF-8 Braille dot pattern
struct FIOS_Uni1 : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Uni1"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = "Uni2"; // may change later
    out  = true;  // this too

    switch(in) {
    case 0xa0: data.tmpchr = 0x00; break;
    case 0xa1: data.tmpchr = DOT_7; break;
    case 0xa2: data.tmpchr = DOT_8; break;
    case 0xa3: data.tmpchr = DOT_7|DOT_8; break;
    case '\r':
    case '\n':
      ++data.lineno; data.markError();
      dest = "NewLine"; data.tmpchr = in; break;
    default:   data.markError(); dest = "ErrEOL"; break;
    }
  }

STATE_END(FIOS_Uni1)
};

//! The third octet in a UTF-8 Braille dot pattern
struct FIOS_Uni2 : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Uni2"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = "WS2"; // may change later
    out  = true;  // this too

    switch(in) {
    case '\r':
    case '\n':
      ++data.lineno; data.markError();
      dest = "NewLine"; data.tmpchr = in; break;
    default:
      if((in & 0xc0) == 0x80) data.tmpchr |= (in & 0x3f);
      else { data.markError(); dest = "ErrEOL"; }
      break;
    }
  }

STATE_END(FIOS_Uni2)
};

//! State builds up the letter half of the transition
struct FIOS_Letr : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Letr"); }
 
  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = getName(); // may change later
    out  = true;      // this too

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case '\n':
    case '\r': {
      // Prepare to inject new mapping
      uint8_t *newletter = new uint8_t[data.tmpchrs.size() + 1];
      newletter[data.tmpchrs.size()] = '\0';
      for(unsigned int i=0; i<data.tmpchrs.size(); ++i)
	newletter[i] = data.tmpchrs[i];

      // Inject new mapping
      data.charset->set(data.tmpchr, newletter);

      // Increment line number.
      // Note last char so we don't count redundantly for CR/LF.
      ++data.lineno;
      data.tmpchr = in;

      // Jump to new line state, other cleanup
      delete[] newletter;
      data.tmpchrs.clear();
      dest = "NewLine";
      break;
    }
    default:   data.tmpchrs.push_back(in); break;
    }
  }

STATE_END(FIOS_Letr)
};

//! State loops and looks for the !UTF-8 END footer
struct FIOS_Ftr : public FIO_State {
  inline virtual const FIOSM_stateNameT getName() const
  { return std::string("Ftr"); }

  inline virtual void operator()(FIOSM_inputT &in, FIOSM_outputT &out,
				 FIOSM_dataT &data, FIOSM_stateNameT &dest)
  {
    dest = getName(); // may change later
    out  = true;      // this too

    switch(in) {
    case '\0': data.markError(); out = false; break;
    case 'U':
      if(data.tmpchr == '!') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'T':
      if(data.tmpchr == 'U') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'F':
      if(data.tmpchr == 'T') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case '-':
      if(data.tmpchr == 'F') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case '8':
      if(data.tmpchr == '-') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case ' ':
      if(data.tmpchr == '8') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'E':
      if(data.tmpchr == ' ') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'N':
      if(data.tmpchr == 'E') data.tmpchr = in; else data.tmpchr = '\0'; break;
    case 'D': // Done with input!
      if(data.tmpchr == 'N') out = false; break;
    case '\r':
    case '\n':
      ++data.lineno; data.markError();
      dest = "NewLine"; data.tmpchr = in; break;
    default:   data.markError(); dest = "ErrEOL"; break;
    }
  }

STATE_END(FIOS_Ftr)
};

//! Assembles a File IO state machine
static FIO_StateMachine assemble_FIOSM(Charset *charset)
{
  // First we generate the state machine itself...
  FIO_StateMachine fiosm;

  // ...then we insert some states.
  fiosm.addState(FIOS_Hdr());
  fiosm.addState(FIOS_Whitespace("WS1", "Nam", "Hdr"));
  fiosm.addState(FIOS_Nam());
  fiosm.addState(FIOS_NewLine());
  fiosm.addState(FIOS_ErrEOL());
  fiosm.addState(FIOS_Hx1());
  fiosm.addState(FIOS_Hx2());
  fiosm.addState(FIOS_Hx3());
  fiosm.addState(FIOS_CDL());
  fiosm.addState(FIOS_Uni1());
  fiosm.addState(FIOS_Uni2());
  fiosm.addState(FIOS_Whitespace("WS2", "Letr", "NewLine"));
  fiosm.addState(FIOS_Letr());
  fiosm.addState(FIOS_Ftr());

  // Change the data element to refer to the character set
  fiosm.getData().charset = charset;

  return fiosm;
}


/////////////////////////
//// Actual File I/O ////
/////////////////////////

// Load a mapping from a UTF-8 input stream
std::vector<unsigned int> Charset::read(std::istream &in)
{
  // Create the state machine itself...
  FIO_StateMachine fiosm(assemble_FIOSM(this));

  // Now go char by char through the input stream
  bool not_finished_yet = true;
  while(not_finished_yet) {
    uint8_t in_chr;
    if(!in.good()) in_chr = '\0';
    else in.read((char*) &in_chr, sizeof(in_chr));
    if(!in.good()) in_chr = '\0'; // yes, redundant on purpose

    fiosm.cycle(in_chr, not_finished_yet);
  }

  return fiosm.getData().error_lines;
}

// Load a mapping from a UTF-8 file
std::vector<unsigned int> Charset::read(const std::string &filename)
{

  // Open the file
  std::ifstream in(filename.c_str());

  if(!in.good())
    throw BTException(BTException::BT_EIO,
		      std::string("failed to open file ") +
		      " for reading");

  // Read in info
  return read(in);
}

// Write a mapping to a UTF-8 output stream
void Charset::write(std::ostream &out) const
{
  // Write the header text, charset name included
  out << "!UTF-8 BEGIN " << name.str.get() << std::endl;

  // Write out non-empty mappings
  for(unsigned int i=0; i<256; ++i)
    if(!dots_to_letters[i].isEmpty()) {
      // First the dots
      const uint8_t i_as_char = i;
      if(i_as_char & 0x01) out << '1';
      if(i_as_char & 0x02) out << '2';
      if(i_as_char & 0x04) out << '3';
      if(i_as_char & 0x08) out << '4';
      if(i_as_char & 0x10) out << '5';
      if(i_as_char & 0x20) out << '6';
      if(i_as_char & 0x40) out << '7';
      if(i_as_char & 0x80) out << '8';

      if(i_as_char == 0x00) out << "0x00";

      // Now the mapped "letter"
      out << '\t' << dots_to_letters[i].str.get() << std::endl;
    }

  // Footer text
  out << "!UTF-8 END" << std::endl;
}

// Write a mapping to a file
void Charset::write(const std::string &filename) const
{
  // Open the file
  std::ofstream out(filename.c_str());

  if(!out.good())
    throw BTException(BTException::BT_EIO,
		      std::string("failed to open file ") + filename +
		      " for writing");

  // If we're on Windows, add the silly Windows UTF-8 header
#ifdef BT_WINDOWS
  static const uint8_t win_utf8_hdr[3] = { 0xef, 0xbb, 0xbf };
  out.write((const char*) win_utf8_hdr, sizeof(win_utf8_hdr));
#endif

  // Write the data
  write(out);
}

} // namespace BrailleTutorNS
