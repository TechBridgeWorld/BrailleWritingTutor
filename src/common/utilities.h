#ifndef UTILITIES_H_
#define UTILITIES_H_

#include <iostream>
#include <deque>
#include <vector>

#include "Dots.h"
#include "Types.h"
#include "Charset.h"
#include "BrailleTutor.h"
#include "IOEvent.h"
#include "ShortStylusSuppressor.h"

using namespace BrailleTutorNS;

typedef unsigned char DotSequence;

int getDot(IOEvent e);
std::string getLetter(IOEvent e);
bool isDown(IOEvent e);
bool isUp(IOEvent e);
bool isButton(IOEvent e);
bool isStylus(IOEvent e);
bool isLetter(IOEvent e);
unsigned char my_dot_mask(const unsigned int &dotno);
void BT_sleep(size_t seconds);

void printEvent(IOEvent e);


/*
 * Given a UTF8 encoded letter, this function determines how many bytes the letter occipies.
 */
size_t numBytesInUTF8Letter(const std::string& utf8letter);

/*
 * Given the FIRST character of an UTF8 encoded letter, this function determines how many bytes the letter occupies.
 * IMPORTANT: The character has to be the first character of the letter! This is merely a convinience function, it is the
 * responsibility of the client to ensure that the character he provides is the first character of the UTF8 encoded letter
 */
size_t numBytesInUTF8Letter(const char utf8letter);

/*
 * Determines the index of UTF8 character in an alphabet array
 * TODO: Write better explanation
 */
size_t alphabeticIndex(const std::vector<std::string>&, const std::string&, int, const std::string&);

size_t alphabeticIndex(const std::vector<std::string>&, const std::string&, int, const char);

DotSequence convertToDotSequence(const Charset&,int n);

#endif

