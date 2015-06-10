/*
 * main.cc
 *
 *  Created on: October 25, 2011
 *      Author: M. Freddie Dias <mfdias@ri.cmu.edu>
 *
 *  This sets up the main program to allow us to run the special hangman game as a 
 *  separate binary for the Interactive2011 event
 */

#include <iostream>
#include <unistd.h>
#include <ctime> //g++ 4.3.2

#include "common/IBTApp.h"
#include "hangman_int2011.h"

struct Int2011Dispatcher : public IOEventHandler
{
  BrailleTutor& bt;
  IOEventParser& iep;
  IBTApp* int2011_app;

  virtual void operator()(std::deque<IOEvent> &events) {
  
    while( !events.empty() ) {
      IOEvent e = events.front();
      events.pop_front();
      int2011_app->processEvent(e);
    }
  }

  explicit inline Int2011Dispatcher(BrailleTutor &my_bt, IOEventParser &my_iep) :
    bt(my_bt), iep(my_iep)
  {
    int2011_app = new EnglishInt2011(iep);
  }
};

int fakemain(int argc, char **argv);

// This hack is to fix the issue with the dll files for SDL already defining a "main" function 
// (only under windows and hasn't been tested thoroughly)
#ifdef BT_WINDOWS
#ifdef main
#undef main
#endif
#endif

// The main function
int main(int argc, char **argv)
{
  try
  {
    return fakemain(argc, argv);
  }
  catch (const BTException &e)
  {
    std::cerr << "A problem occurred: " << e.why << std::endl;
    std::cerr << "\n"
      "If the Braille Tutor is plugged in right now, please try unplugging it and\n"
      "plugging it back in. Then restart this program.\n\n"
      "--- Enter Ctrl-C to quit ---" << std::endl;
    std::string unused;
    std::getline(std::cin, unused);
    return -1;
  }
  catch (const std::string &s)
  {
    std::cerr << "Error: " << s << std::endl;
    return -1;
  }
  catch (...)
  {
    std::cerr << "Some other exception happened" << std::endl;
    return -1;
  }

  return 0;
}

int fakemain(int argc, char **argv)
{
  std::srand((unsigned) std::time(0));
  BrailleTutor bt;

  std::cout << "Registering IOEventParser..." << std::endl;
  // The user can add the --nodebounce command line argument to
  // turn off stylus input debouncing. If they turn off the
  // debouncind, the BaseIOEvents are passed directly to the
  // IOEventParser. This is a TSS mod!
  ShortStylusSuppressor debouncer(0.3);
  IOEventParser event_parser;
  if( (argc > 1) && !strcmp(argv[1], "--nodebounce") )
  {
    std::cout << "[ DEBOUNCING DISABLED ]" << std::endl;
    bt.setBaseIOEventHandler(event_parser);
  }
  else
  {
    debouncer.setBaseIOEventHandler(event_parser);
    bt.setBaseIOEventHandler(debouncer);
  }

  std::cout << "Subscribing to events..." << std::endl;

  event_parser.wantEvent(IOEvent::STYLUS);
  event_parser.wantEvent(IOEvent::STYLUS_UP);
  event_parser.wantEvent(IOEvent::STYLUS_DOWN);
  event_parser.wantEvent(IOEvent::BUTTON);
  event_parser.wantEvent(IOEvent::BUTTON_UP);
  event_parser.wantEvent(IOEvent::BUTTON_DOWN);
  event_parser.wantEvent(IOEvent::CELL_START);
  event_parser.wantEvent(IOEvent::CELL_DONE);
  event_parser.wantEvent(IOEvent::BUTTON_START);
  event_parser.wantEvent(IOEvent::BUTTON_DONE);
  event_parser.wantEvent(IOEvent::CELL_DOTS);
  event_parser.wantEvent(IOEvent::CELL_LETTER);
  event_parser.wantEvent(IOEvent::BUTTON_DOTS);
  event_parser.wantEvent(IOEvent::BUTTON_LETTER);

  // create a new event handler
  Int2011Dispatcher int2011d(bt, event_parser);

  // Initialize the braille tutor
  std::cout << "Initialization..." << std::endl;
  bt.init();
  std::cout << "Detection..." << std::endl;
  std::string io_port;
  unsigned int version;
  bt.detect(io_port, version);
  std::cout << "  found a version " << version << " tutor on " << io_port << std::endl;

  // set the default handler for IO events
  event_parser.setIOEventHandler(int2011d);

  bt.join();

  return 0;
}
