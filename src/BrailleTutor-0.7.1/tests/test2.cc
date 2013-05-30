#include "Types.h"
#include "IOEvent.h"
#include "BrailleTutor.h"

#include "ShortStylusSuppressor.h"

#include <string>
#include <iostream>

#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>

using namespace BrailleTutorNS;

struct IOEventDemo : public IOEventHandler {
  // Condition variable that we use to notify main thread to quit.
  boost::condition	&cond_quit;
  // IOEventParser object that we use to flush glyphs under construction.
  IOEventParser		&iep;

  // The event handler
  virtual void operator()(std::deque<IOEvent> &events)
  {
    while(!events.empty()) {
      std::cout << '[' << (double) events.front().timestamp << "]\t";

      // Prints out events
      if(events.front().type == IOEvent::STYLUS_DOWN)
        std::cout << "STYLUS DOWN, cell " << events.front().cell
                  << ", dot " << (int) events.front().dot;
      else if(events.front().type == IOEvent::STYLUS_UP)
        std::cout << "STYLUS UP, cell " << events.front().cell
                  << ", dot " << (int) events.front().dot;
      else if(events.front().type == IOEvent::BUTTON_DOWN) {
        std::cout << "BUTTON DOWN, button " << events.front().button;
        if(events.front().dot != INVALID_DOT)
          std::cout << ", dot " << (int) events.front().dot;
      }
      else if(events.front().type == IOEvent::BUTTON_UP) {
        std::cout << "BUTTON UP, button " << events.front().button;
        if(events.front().dot != INVALID_DOT)
          std::cout << ", dot " << (int) events.front().dot;
	if(events.front().button == 0) {
	  std::cout << " (flushing glyph!) ";
	  iep.flushGlyph();
	}
      }
      else if(events.front().type == IOEvent::STYLUS)
	std::cout << "STYLUS, cell " << events.front().cell << ", dot "
		  << (int) events.front().dot << ", duration "
		  << (double) events.front().duration;
      else if(events.front().type == IOEvent::BUTTON) {
	std::cout << "BUTTON, button " << events.front().button;
	if(events.front().dot != INVALID_DOT)
	  std::cout << ", dot " << (int) events.front().dot;
	std::cout <<   ", duration " << (double) events.front().duration;
      }
      else if(events.front().type == IOEvent::CELL_START)
	std::cout << "CELL_START, cell " << events.front().cell;
      else if(events.front().type == IOEvent::CELL_DONE)
	std::cout << "CELL_DONE, cell " << events.front().cell
		  << ", duration " << (double) events.front().duration;
      else if(events.front().type == IOEvent::BUTTON_START)
	std::cout << "BUTTON_START";
      else if(events.front().type == IOEvent::BUTTON_DONE)
	std::cout << "BUTTON_DONE, duration "
		  << (double) events.front().duration;
      else if(events.front().type == IOEvent::CELL_DOTS)
	std::cout << "CELL_DOTS, cell " << events.front().cell
		  << ", duration " << (double) events.front().duration
		  << ", dots " << dot_string(events.front().dots);
      else if(events.front().type == IOEvent::BUTTON_DOTS)
	std::cout << "BUTTON_DOTS, duration "
		  << (double) events.front().duration
		  << ", dots " << dot_string(events.front().dots);
      else if(events.front().type == IOEvent::CELL_LETTER) {
	std::cout << "CELL_LETTER, cell " << events.front().cell
		  << ", duration " << (double) events.front().duration
		  << ", dots " << dot_string(events.front().dots)
		  << std::flush;
	std::cout << ", glyph (UTF-8) \""
		  << (std::string) events.front().letter << "\"" << std::flush;
      }
      else if(events.front().type == IOEvent::BUTTON_LETTER) {
	std::cout << "BUTTON_LETTER, duration "
		  << (double) events.front().duration
		  << ", dots " << dot_string(events.front().dots)
		  << std::flush;
	std::cout << ", glyph (UTF-8) \""
		  << (std::string) events.front().letter << "\"" << std::flush;
      }

      // Special event actions
      // Quit if the user enters the letter Q
      if(((events.front().type == IOEvent::BUTTON_LETTER) ||
	  (events.front().type == IOEvent::CELL_LETTER)) &&
	  (events.front().letter == "Q")) {
	std::cout << " QUIT!";
	cond_quit.notify_one();
      }

      std::cout << std::endl;
      events.pop_front();
    }
  }

  // Constructor---sets references
  IOEventDemo(boost::condition &my_cond_quit, IOEventParser &my_iep)
  : cond_quit(my_cond_quit), iep(my_iep) { }
};

int fakemain(int argc, char **argv)
{
  BrailleTutor          bt;
  boost::mutex          mutex_quit;
  boost::condition      cond_quit;

  std::cout << "Registering IOEventParser..." << std::endl;
  // The user can add the --debounce command line argument to debounce the
  // stylus inputs. Otherwise the BaseIOEvents are passed directly to the
  // IOEventParser.
  ShortStylusSuppressor debouncer(0.3);
  IOEventParser iep;
  if((argc > 1) && !strcmp(argv[1], "--debounce")) {
    std::cout << "[ DEBOUNCING ENABLED ]" << std::endl;
    debouncer.setBaseIOEventHandler(iep);
    bt.setBaseIOEventHandler(debouncer);
  }
  else bt.setBaseIOEventHandler(iep);

  std::cout << "Registering handler..." << std::endl;
  IOEventDemo demo_handler(cond_quit, iep);
  iep.setIOEventHandler(demo_handler);

  std::cout << "Subscribing to all events..." << std::endl;
  iep.wantEvent(IOEvent::STYLUS);
  iep.wantEvent(IOEvent::STYLUS_UP);
  iep.wantEvent(IOEvent::STYLUS_DOWN);
  iep.wantEvent(IOEvent::BUTTON);
  iep.wantEvent(IOEvent::BUTTON_UP);
  iep.wantEvent(IOEvent::BUTTON_DOWN);
  iep.wantEvent(IOEvent::CELL_START);
  iep.wantEvent(IOEvent::CELL_DONE);
  iep.wantEvent(IOEvent::BUTTON_START);
  iep.wantEvent(IOEvent::BUTTON_DONE);
  iep.wantEvent(IOEvent::CELL_DOTS);
  iep.wantEvent(IOEvent::CELL_LETTER);
  iep.wantEvent(IOEvent::BUTTON_DOTS);
  iep.wantEvent(IOEvent::BUTTON_LETTER);

  std::cout << "Initialization..." << std::endl;
  bt.init();

  std::cout << "Detection..." << std::endl;
  std::string io_port;
  unsigned int version;
  bt.detect(io_port, version);
  std::cout << "  found a version " << version
            << " tutor on " << io_port << std::endl;
  std::cout << "Punch in the letter Q to quit..." << std::endl;

  // Wait for the user to code 'Q" on the BT.
  boost::mutex::scoped_lock lock_quit(mutex_quit);
  cond_quit.wait(lock_quit);
  std::cout << "Quitting..." << std::endl;
  // See note in test.cc about calling exit() at the end of the program.
  exit(0);

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
