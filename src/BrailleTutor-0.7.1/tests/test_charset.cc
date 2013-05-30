#include "Dots.h"
#include "Types.h"
#include "Charset.h"
#include "BrailleTutor.h"

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace BrailleTutorNS;

//! Homerolled templated strlen; real strlen is only for chars.
template <typename T>
inline static unsigned int local_strlen(const T *str_T)
{ unsigned int len = 0;
  while(*str_T++) ++len;
  return len; }

int fakemain(int argc, char **argv)
{
  {
    // Create a new charset
    GlyphMapping tc_name("Test Devanagari charset");
    Charset test_charset(tc_name);

    // Let's add some Devanagari characters
    uint32_t widestr[2];
    widestr[1] = 0U;

    widestr[0] = 0x927; // char 1
    test_charset.set(DOT_1 | DOT_2, widestr);

    widestr[0] = 0x90B; // char 2
    test_charset.set(DOT_1 | DOT_3, widestr);

    widestr[0] = 0x939; // char 2
    test_charset.set(DOT_2 | DOT_3, widestr);

    if((test_charset[DOT_1|DOT_2].str[0] == 0xe0) &&
       (test_charset[DOT_1|DOT_2].str[1] == 0xa4) &&
       (test_charset[DOT_1|DOT_2].str[2] == 0xa7) &&
       (test_charset[DOT_1|DOT_2].str[3] == '\0') &&
       (test_charset[DOT_1|DOT_3].str[0] == 0xe0) &&
       (test_charset[DOT_1|DOT_3].str[1] == 0xa4) &&
       (test_charset[DOT_1|DOT_3].str[2] == 0x8b) &&
       (test_charset[DOT_1|DOT_3].str[3] == '\0') &&
       (test_charset[DOT_2|DOT_3].str[0] == 0xe0) &&
       (test_charset[DOT_2|DOT_3].str[1] == 0xa4) &&
       (test_charset[DOT_2|DOT_3].str[2] == 0xb9) &&
       (test_charset[DOT_2|DOT_3].str[3] == '\0'))
      std::cerr << "Charset 1 UTF-32->UTF-8 conversion correct." << std::endl;

    test_charset.write("test_charset_1.charset");
    std::cerr << "Charset 1 written." << std::endl;
  }


  {
    // Create a new charset
    GlyphMapping tc_name("Test arabic charset");
    Charset test_charset(tc_name);

    // Let's add some arabic characters
    uint8_t str[3];
    str[2] = '\0';

    str[0] = 0xd8; // char 1
    str[1] = 0xaa;
    test_charset.set(DOT_1 | DOT_2, str);

    str[0] = 0xd8; // char 2
    str[1] = 0xb4;
    test_charset.set(DOT_1 | DOT_3, str);

    str[0] = 0xd9; // char 3
    str[1] = 0x8a;
    test_charset.set(DOT_2 | DOT_3, str);

    if((test_charset[DOT_1|DOT_2].str_w[0] == 0x62a) &&
       (test_charset[DOT_1|DOT_2].str_w[1] == 0) &&
       (test_charset[DOT_1|DOT_3].str_w[0] == 0x634) &&
       (test_charset[DOT_1|DOT_3].str_w[1] == 0) &&
       (test_charset[DOT_2|DOT_3].str_w[0] == 0x64a) &&
       (test_charset[DOT_2|DOT_3].str_w[1] == 0))
      std::cerr << "Charset 2 UTF-8->UTF-32 conversion correct." << std::endl;

    test_charset.write("test_charset_2.charset");
    std::cerr << "Charset 2 written." << std::endl;
  }

  { // Now we write a "torture test" file to disk. Has lots of weird Unicode,
    // CR+LF and LF line breaks mixed, and all three ways of encoding Braille
    // dots. There are also some nonsense lines mixed in as errors.
    uint8_t torture[] =
    { 0x48,0x65,0x72,0x65,0x20,0x69,0x73,0x20,0x73,0x6F,0x6D,0x65,0x20,0x69,
      0x72,0x72,0x65,0x6C,0x65,0x76,0x61,0x6E,0x74,0x20,0x74,0x65,0x78,0x74,
      0x21,0x0A,0x0A,0x21,0x55,0x54,0x46,0x2D,0x38,0x20,0x42,0x45,0x47,0x49,
      0x4E,0x20,0xE2,0x93,0x89,0xE2,0x85,0x87,0xE2,0x88,0xAB,0xE2,0x94,0xAC,
      0x20,0x4F,0x64,0x64,0x62,0x61,0x6C,0x6C,0x0D,0x0A,0xE2,0xA0,0xA3,0x09,
      0x09,0x20,0x09,0xE2,0x91,0xAB,0xE2,0x91,0xB9,0x0A,0x30,0x78,0x31,0x66,
      0x09,0x43,0x69,0x74,0x74,0xC3,0xA0,0x0A,0x45,0x72,0x72,0x21,0x09,0x45,
      0x72,0x72,0x6F,0x72,0x20,0x31,0x0A,0x35,0x36,0x37,0x38,0x09,0xE1,0xBA,
      0x94,0x0D,0x0A,0x36,0x38,0x39,0x09,0x09,0x45,0x72,0x72,0x6F,0x72,0x20,
      0x32,0x0A,0x21,0x55,0x54,0x46,0x2D,0x38,0x20,0x45,0x4E,0x44,0x4D,0x6F,
      0x72,0x65,0x20,0x69,0x72,0x72,0x65,0x6C,0x65,0x76,0x61,0x6E,0x74,0x20,
      0x74,0x65,0x78,0x74,0x21 };

    // Create the torture file
    std::ofstream out("test_torture.charset");
    out.write((char*) torture, sizeof(torture));
    out.close();
    std::cerr << "Wrote torture test charset file." << std::endl;

    // Read in the torture file
    Charset test_charset("");
    std::vector<unsigned int> err_lines =
      test_charset.read("test_torture.charset");
    std::cerr << "Loaded torture test charset file. Errors on lines";
    for(unsigned int i=0; i<err_lines.size(); ++i)
      std::cerr << ' ' << err_lines[i];
    std::cerr << " (should be 6 8)." << std::endl;

    // Write out the torture file from the loaded info
    test_charset.write("test_charset_3.charset");
    std::cerr << "Charset 3 written (the torture one again)." << std::endl;

    // Clone the test character set using the UTF-32 info only
    Charset test_charset_2("");
    test_charset_2.setName(test_charset.getName().str_w.get());
    for(unsigned int i=0; i<256; ++i) {
      if(test_charset[i].isEmpty()) continue;
      test_charset_2.set((uint8_t) i, test_charset[i].str_w.get());
    }

    // Write out the torture file from the copied data
    test_charset_2.write("test_charset_4.charset");
    std::cerr << "Charset 4 written (the torture one again)." << std::endl;
    std::cerr << "Should be identical to Charset 3 (check it!)." << std::endl;
  }

  return 0;
}

int main(int argc, char **argv)
{
  try { return fakemain(argc, argv); }
  catch(const BTException &e) {
    std::cerr << "BTException: " << e.why << std::endl;
    return -1;
  }
  catch(const std::string &s) {
    std::cerr << "String exception: " << s << std::endl;
    return -1;
  }
  catch(...) {
    std::cerr << "Some other exception happened" << std::endl;
    return -1;
  }

  return 0;
}
