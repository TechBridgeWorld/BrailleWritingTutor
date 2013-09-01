#include <sstream>
#include "utilities.h"

#if defined(BT_LINUX)
  #include <time.h>
#elif defined(BT_WINDOWS)
  #include <Windows.h>
#endif


int getDot(IOEvent e)
{
  int forward = isStylus(e) ? (int) e.dot + 1 : e.button;
  int flip[7] = { -1, 4, 5, 6, 1, 2, 3 };
  return flip[forward];
}

std::string getLetter(IOEvent e)
{
  return e.letter;
}

bool isDown(IOEvent e)
{
  return (e.type == IOEvent::STYLUS_DOWN || e.type == IOEvent::BUTTON_DOWN);
}

bool isUp(IOEvent e)
{
  return !isDown(e);
}

bool isButton(IOEvent e)
{
  return (e.type == IOEvent::BUTTON_UP || e.type == IOEvent::BUTTON_DOWN);
}

bool isStylus(IOEvent e)
{
  return !isButton(e);
}

bool isLetter(IOEvent e)
{
  return (e.type == IOEvent::CELL_LETTER || e.type == IOEvent::BUTTON_LETTER);
}

unsigned char my_dot_mask(const unsigned int &dotno)
{
  return dot_mask(dotno - 1);
}

void printEvent(IOEvent e)
{
  if( e.type == IOEvent::STYLUS_DOWN )
    std::cout << "STYLUS DOWN, cell " << e.cell << ", dot " << (int) e.dot;
  else if( e.type == IOEvent::STYLUS_UP )
    std::cout << "STYLUS UP, cell " << e.cell << ", dot " << (int) e.dot;
  else if( e.type == IOEvent::BUTTON_DOWN )
  {
    std::cout << "BUTTON DOWN, button " << e.button;
    if( e.dot != INVALID_DOT )
      std::cout << ", dot " << (int) e.dot;
  }
  else if( e.type == IOEvent::BUTTON_UP )
  {
    std::cout << "BUTTON UP, button " << e.button;
    if( e.dot != INVALID_DOT )
      std::cout << ", dot " << (int) e.dot;
  }
  else if( e.type == IOEvent::STYLUS )
    std::cout << "STYLUS, cell " << e.cell << ", dot " << (int) e.dot << ", duration " << (double) e.duration;
  else if( e.type == IOEvent::BUTTON )
  {
    std::cout << "BUTTON, button " << e.button;
    if( e.dot != INVALID_DOT )
      std::cout << ", dot " << (int) e.dot;
    std::cout << ", duration " << (double) e.duration;
  }
  else if( e.type == IOEvent::CELL_START )
    std::cout << "CELL_START, cell " << e.cell;
  else if( e.type == IOEvent::CELL_DONE )
    std::cout << "CELL_DONE, cell " << e.cell << ", duration " << (double) e.duration;
  else if( e.type == IOEvent::BUTTON_START )
    std::cout << "BUTTON_START";
  else if( e.type == IOEvent::BUTTON_DONE )
    std::cout << "BUTTON_DONE, duration " << (double) e.duration;
  else if( e.type == IOEvent::CELL_DOTS )
    std::cout << "CELL_DOTS, cell " << e.cell << ", duration " << (double) e.duration << ", dots " << dot_string(e.dots);
  else if( e.type == IOEvent::BUTTON_DOTS )
    std::cout << "BUTTON_DOTS, duration " << (double) e.duration << ", dots " << dot_string(e.dots);
  else if( e.type == IOEvent::CELL_LETTER )
  {
    std::cout << "CELL_LETTER, cell " << e.cell << ", duration " << (double) e.duration << ", dots " << dot_string(e.dots) << std::flush;
    std::cout << ", glyph \"" << (std::string) e.letter << "\"" << std::flush;
  }
  else if( e.type == IOEvent::BUTTON_LETTER )
  {
    std::cout << "BUTTON_LETTER, duration " << (double) e.duration << ", dots " << dot_string(e.dots) << std::flush;
    std::cout << ", glyph \"" << (std::string) e.letter << "\"" << std::flush;
  }
  std::cout << std::endl;
}

/*
 * The cool thing about UTF8 is that, you can determine how many bytes a letter occupies by examining it's first quartet. That is,
 *
 * 1111 0xxx = four-byte character
 * 1110 xxxx = three-byte character
 * 110x xxxx = two-byte character
 * otherwise, it's a one byte character
 */

size_t numBytesInUTF8Letter(const std::string& utf8letter)
{
  return numBytesInUTF8Letter(utf8letter.at(0));
}

size_t numBytesInUTF8Letter(const char c)
{
  if( (c & 0xF0) == 0xF0 ) //4 byte character
    return 4;
  if( (c & 0xE0) == 0xE0 ) //3 byte character
    return 3;
  if( (c & 0xC0) == 0xC0 ) //2 byte character
    return 2;
  return 1; //1 byte character
}

size_t alphabeticIndex(const std::vector<std::string>& alphabet, const std::string& word, int word_pos, const std::string& letter_to_search_for)
{
  return alphabeticIndex(alphabet, word, word_pos, letter_to_search_for.at(0));
}

//FIXME: dumb.
size_t alphabeticIndex(const std::vector<std::string>& alphabet, const std::string& word, int word_pos, const char letter_to_search_for)
{
  int num_bytes_in_letter = numBytesInUTF8Letter(letter_to_search_for);
  std::string correct_letter(word, word_pos, num_bytes_in_letter);
  return std::distance(alphabet.begin(), std::find(alphabet.begin(), alphabet.end(), correct_letter));
}

void BT_sleep(size_t seconds)
{
#if defined(BT_LINUX)
  sleep(seconds);
#elif defined(BT_WINDOWS)
  Sleep(seconds*1000);
#endif

}

DotSequence convertToDotSequence(const Charset& charset, int n)
{
  std::stringstream out;
  out << n;
  return charset[GlyphMapping(out.str())];
}
