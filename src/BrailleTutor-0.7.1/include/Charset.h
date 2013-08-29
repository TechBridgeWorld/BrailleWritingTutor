#ifndef _LIBBT_CHARSET_H_
#define _LIBBT_CHARSET_H_
/*
 * Braille Tutor interface library
 * Charset.h, started 24 May 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Mappings from cell dot patterns to byte sequences and vice versa,
 * as well as I/O code for loading these patterns from streams.
 *
 * NB: The original goal of this code was to map from cell dot patterns
 * to Unicode/wide character strings. Thanks to the deplorable state of
 * libstdc++ wide character I/O support on Windows, this code is no longer
 * so ambitious. Today dot patterns are mapped to a special GlyphMapping
 * object that contains both a regular null-terminated char array and an
 * array of 32-bit unsigned ints (also null-terminated). The unsigned ints
 * are made by converting the bytes (in my own code, &$^@!) as if the bytes
 * in the file are UTF-8. With luck you should be able to cast the unsigned
 * int string to a wchar_t string on your platform; otherwise you should be
 * able to convert it to one without too much trouble. So not quite passing
 * the portability buck, but a little bit.
 */

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <stdint.h>

#include <boost/shared_array.hpp>

namespace BrailleTutorNS {

//! Character/"wide char" structures to which Braille dot patterns are mapped.

//! Dot patterns are mapped to GlyphMapping objects, which contain pointers
//! to both a regular null-terminated char array and an array of 32-bit
//! unsigned ints (also null-terminated). The unsigned ints are made by
//! converting the char array bytes as if they are UTF-8. You may be able
//! to cast the unsigned int string to a wchar_t string on your platform;
//! otherwise you should be able to convert it to one without much trouble.
//!
//! The smart pointers in a GlyphMapping object may point to the same strings
//! as the pointers in other GlyphMapping objects. Change these strings with
//! caution, or best not at all.
struct GlyphMapping {
  //! Character string to which a Braille dot pattern is mapped
  boost::shared_array<uint8_t> str; // NB: You may have to cast to char*.

  //! 32-bit unsigned null-terminated "string" to which a Braille dot pattern
  //! is mapped
  boost::shared_array<uint32_t> str_w;

  //! Equality test for GlyphMapping objects (compares the str member)
  bool equals(const GlyphMapping &gm) const;

  //! Comparison for GlyphMapping objects (compares the str_w member)

  //! Note: this routine is more for STL containers that store GlyphMapping
  //! objects than for practical string sorting ('A' with an accent will be
  //! sorted after the whole unaccented alphabet, for example).
  bool lt(const GlyphMapping &gm) const;

private:
  //! Shared routine initializing a GlyphMapping from a character string
  void initFromStr(const uint8_t *my_str);

public:

  //! Constructor: converts an array of 32-bit ints to a GlyphMapping.
  GlyphMapping(const uint32_t *my_str_w);

  //! Constructor: converts an array of unsigned chars to a GlyphMapping.
  GlyphMapping(const uint8_t *my_str) { initFromStr(my_str); }

  //! Constructor: converts a regular ASCII std::string to a GlyphMapping.
  inline GlyphMapping(const std::string &my_str)
  {  initFromStr((uint8_t *) my_str.c_str()); }

  //! Constructor: converts an array of ASCII chars to a GlyphMapping.
  inline GlyphMapping(const char *my_str) {initFromStr((uint8_t *) my_str); }

  //! Constructor: creates an "empty mapping"
  inline GlyphMapping()
  : str(new uint8_t[1]), str_w(new uint32_t[1]) { str[0] = 0; str_w[0] = 0; }

  //! Returns a GlyphMapping whose pointers point at new copies of this
  //! GlyphMapping's information
  GlyphMapping dup() const;

  //! Returns an approximate std::string version of this GlyphMapping.
  //! Useful only for ASCII strings.
  inline operator std::string() const { return std::string((char*) str.get()); }

  //! Returns true if this GlyphMapping is the empty mapping
  inline bool isEmpty() const { return *(str.get()) == 0; }
};

//! GlyphMapping infix equality operator
inline bool operator==(const GlyphMapping &a, const GlyphMapping &b)
{ return a.equals(b); }

//! GlyphMapping infix less-than operator
inline bool operator<(const GlyphMapping &a, const GlyphMapping &b)
{ return a.lt(b); }

// Predefinition
class DotsMirror;

//! A two-way mapping between cell dot patterns and Unicode letters/strings

//! Contains a two-way mapping between cell dot patterns and Unicode
//! letters/strings represented by the patterns. Note that Unicode strings
//! will be truncated to a fixed length in IOEvent objects.
class Charset {
// We have to roll our own two-way map since Boost::Bimap doesn't exist yet.
private:
  //! Name of this character set. A GlyphMapping object works fine here too.
  GlyphMapping name;

  //! Mapping from cell dot pattern bitmaps to letters
  GlyphMapping dots_to_letters[256];
  //! Mapping from letters to cell dot pattern bitmaps
  std::map<GlyphMapping, unsigned char> letters_to_dots;

public:
  //! Constructor. Creates a new empty mapping.
  inline Charset(const GlyphMapping &my_name)
  : name(my_name) { clear(); }

  //! Named constructor. Loads a character set from a wide input stream.
  //! Errors are silently ignored.
  inline static Charset fromStream(std::istream &in)
  { Charset c(""); c.read(in); return c; }

  //! Named constructor. Loads a character set from a file. Errors are
  //! silently ignored.
  inline static Charset fromFile(const std::string &filename)
  { Charset c(""); c.read(filename); return c; }

  //! Clear out the entire mapping
  inline void clear()
  { for(unsigned int i=0; i<256; ++i) dots_to_letters[i] = GlyphMapping();
    letters_to_dots.clear(); }

  //! The number of entries in this mapping
  inline unsigned int size() { return letters_to_dots.size(); }

  //! Retrieve the name of this character set
  inline const GlyphMapping &getName() const { return name; }
  //! Set the name of this character set
  inline void setName(const GlyphMapping &my_name) { name = my_name; }

  //! Set or reset an individual mapping

  //! Sets or resets an individual mapping. To clear out a mapping, either
  //! set the first parameter to INVALID_DOTS or the second parameter to
  //! the empty GlyphMapping (e.g. GlyphMapping()).
  void set(const unsigned char &dots, const GlyphMapping &letter);

  //! Retrieve the letter onto which the dot pattern argument is mapped

  //! Retrieve the letter onto which the dot pattern argument is mapped. An
  //! empty GlyphMapping indicates that there is no letter for the dot pattern.
  inline const GlyphMapping &operator[](const unsigned char &dots) const
  { return dots_to_letters[dots]; }

  //! Retrieve the dot pattern onto which the letter argument is mapped

  //! Retrieve the dot pattern onto which the letter argument is mapped. The
  //! value INVALID_DOTS indicates that no such mapping exists.
  inline const unsigned char &operator[](const GlyphMapping &letter) const
  { std::map<GlyphMapping, unsigned char>::const_iterator i;
    i = letters_to_dots.find(letter);
    if(i==letters_to_dots.end()) return INVALID_DOTS; else return i->second;}

  //! Facilitates lookup/retrieval of mirrored dot patterns

  //! On the Braille Tutor, braille cell dot patterns are mirrored about the
  //! vertical axis. This routine facilitates dot pattern <-> Unicode string
  //! conversion (i.e. the operator[] operations) with the dot patterns
  //! mirrored. The calling convention for this routine is like so:
  //!   - <code>my_charset.mir()[dot_pattern]</code>, or
  //!   - <code>my_charset.mir()[Unicode_string]</code>.
  //!
  //! See also IOEvent.cc in the library source for examples.
  DotsMirror mir() const;

  //! Load a mapping from a UTF-8 input stream. Returns line numbers where
  //! errors were encountered.
  std::vector<unsigned int> read(std::istream &in);
  //! Load a mapping from a file. Returns line numbers where errors were
  //! encountered. For portability, only throws BT_EIO BTExceptions.
  std::vector<unsigned int> read(const std::string &filename);
  //! Write a mapping to a UTF-8 output stream
  void write(std::ostream &out) const;
  //! Write a mapping to a file. For portability, only throws BT_EIO
  //! BTExceptions.
  void write(const std::string &filename) const;

  //! Returns const reference to a default character set
  static const Charset &defaultCharset();
};

//! A class that intercepts and mirrors lookup operations on charsets

//! This class shadows the []-based lookup operations in Charset and
//! causes all instances of braille cell dots to be mirrored about the
//! vertical axis. Useful for the Braille Tutor itself, where users must
//! enter the mirror image of Braille dots into the slate cells.
struct DotsMirror {
  //! Refers to the Charset object to which this mirror is applied
  const Charset &charset;

  //! Mirrors the positions of Braille dots around the vertical axis

  //! Returns a new dot pattern with dots 1,2,3 swapped with dots 4,5,6
  //! and dot 7 swapped with dot 8. Useful for the Braille Tutor itself,
  //! where users must enter the mirror image of Braille dots into the
  //! slate cells.
  static const unsigned char mir(const unsigned char &dots)
  { return ((dots & 0x07) << 3) | ((dots & 0x38) >> 3) |
	   ((dots & 0x80) >> 1) | ((dots & 0x40) << 1); }

  //! Does Charset::operator[](const unsigned char&) with the dots mirrored
  inline const GlyphMapping &operator[](const unsigned char &dots) const
  { return charset[mir(dots)]; }

  //! Does Charset::operator[](const std::wstring&) with the dots mirrored
  inline const unsigned char operator[](const GlyphMapping &letter) const
  { const unsigned char orig = charset[letter];
    if(orig == INVALID_DOTS) return INVALID_DOTS; else return mir(orig); }

  //! Constructor: sets our const reference to the current charset
  inline DotsMirror(const Charset &my_charset) : charset(my_charset) { }
};

} // namespace BrailleTutorNS

#endif
