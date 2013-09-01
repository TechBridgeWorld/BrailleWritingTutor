#include "Types.h"
#include "../lib/serial_io.h"

#include <deque>
#include <string>
#include <iostream>
#include <iterator>

using namespace BrailleTutorNS;

int fakemain(int argc, char **argv)
{
  std::deque<std::string> suggestions;
  if(argc <= 1) suggestions = serial_suggest_ports();
  else for(int i=1; i<argc; ++i) suggestions.push_back(argv[i]);

  // Open a suggested serial port
  serial_handle handle;
  while(!suggestions.empty()) {
    std::cerr << "Trying to open " << suggestions.front() << "..." << std::endl;
    try {
      serial_open(suggestions.front(), handle);
      break;
    }
    catch(const BTException &e) {
      if((e.type == BTException::BT_EBUSY) ||
	 (e.type == BTException::BT_ENOENT)) {
	std::cerr << "Non-fatal error: " << e.why << ". Trying another port."
		  << std::endl;
	suggestions.pop_front();
	continue;
      }
      throw;
    }
  }
  if(suggestions.empty()) throw std::string("no serial ports worked");

  // Print out incoming bytes at 100ms intervals
  for(unsigned int i=0; i<10; ++i) {
    serial_read(handle, std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;
    TimeInterval(.1).sleep();
  }

  // Send "bt" init to the BT
  const char *initstr = "bt";
  serial_write(handle, initstr, initstr+2);

  // Print out incoming bytes at 100ms intervals
  for(unsigned int i=0; i<10; ++i) {
    serial_read(handle, std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;
    TimeInterval(.1).sleep();
  }

  // Send a beep command to the BT
  char beepstr[5];
  beepstr[0] = 'b';
  beepstr[1] = '7';
  beepstr[2] = '7';
  beepstr[3] = 'n';
  beepstr[4] = '\0';
  char beepbyte = 15;
  for(unsigned int i=0; i<500; ++i) {
    beepstr[1] = beepbyte++;
    serial_write(handle, beepstr, beepstr+4);

    // Print out incoming bytes
    serial_read(handle, std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;
  }

  // Print out incoming bytes at 100ms intervals
  for(;;) {
    serial_read(handle, std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;
    TimeInterval(.1).sleep();
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
