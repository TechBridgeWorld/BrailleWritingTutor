#include "Types.h"
#include "BrailleTutor.h"

#include "ShortStylusSuppressor.h"

#include <string>
#include <iostream>

#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>

using namespace BrailleTutorNS;

struct BaseIOEventDemo : public BaseIOEventHandler {
  // Condition variable that we use to notify main thread to quit.
  boost::condition	&cond_quit;
  // BrailleTutor object
  BrailleTutor		&bt;

  // The event handler
  virtual void operator()(std::deque<BaseIOEvent> &events)
  {
    // To test the BT library's ability to handle very long event processors,
    // uncomment the next three lines of code.
/*  std::cerr << "A 1s delay to make things interesting..." << std::flush;
    TimeInterval(1,0).sleep();
    std::cerr << " done!" << std::endl;   */

    while(!events.empty()) {
      std::cout << (double) events.front().timestamp << ":  ";

      // Prints out events
      if(events.front().type == BaseIOEvent::STYLUS_DOWN)
	std::cout << "STYLUS DOWN, cell " << events.front().cell
		  << ", dot " << (int) events.front().dot;
      else if(events.front().type == BaseIOEvent::STYLUS_UP)
	std::cout << "STYLUS UP, cell " << events.front().cell
		  << ", dot " << (int) events.front().dot;
      else if(events.front().type == BaseIOEvent::BUTTON_DOWN) {
	std::cout << "BUTTON DOWN, button " << events.front().button;
	if(events.front().dot != INVALID_DOT)
	  std::cout << ", dot " << (int) events.front().dot;
      }
      else if(events.front().type == BaseIOEvent::BUTTON_UP) {
	std::cout << "BUTTON UP, button " << events.front().button;
	if(events.front().dot != INVALID_DOT)
	  std::cout << ", dot " << (int) events.front().dot;
      }

      // Special event actions
      // Quit on button 0
      if((events.front().type == BaseIOEvent::BUTTON_UP) &&
	 (events.front().button == 0)) {
	std::cout << " QUIT!";
	cond_quit.notify_one();
      }
      // Beep briefly on button 1
      else if((events.front().type == BaseIOEvent::BUTTON_UP) &&
	      (events.front().button == 1)) {
	std::cout << " BEEP!";
	bt.beep(440.0, 0.05);
      }
      // Set IO pin high on button 2
      else if((events.front().type == BaseIOEvent::BUTTON_UP) &&
	      (events.front().button == 2)) {
	std::cout << " SET I/O pin high";
	bt.iopin(0, true);
      }
      // Set IO pin low on button 3
      else if((events.front().type == BaseIOEvent::BUTTON_UP) &&
	      (events.front().button == 3)) {
	std::cout << " SET I/O pin low";
	bt.iopin(0, false);
      }
      // Query IO pin on button 4
      else if((events.front().type == BaseIOEvent::BUTTON_UP) &&
	      (events.front().button == 4)) {
	std::cout << " QUERY I/O pin: " << (bt.iopin(0) ? "high" : "low");
      }
      else if((events.front().type == BaseIOEvent::BUTTON_UP) &&
	      (events.front().button == 5)) {
	std::cout << " QUERY I/O pin: " << (bt.iopin(0) ? "high" : "low");
      }

      std::cout << std::endl;
      events.pop_front();
    }
  }

  // Constructor---sets references
  BaseIOEventDemo(boost::condition &my_cond_quit, BrailleTutor &my_bt)
  : cond_quit(my_cond_quit), bt(my_bt) { }

  virtual ~BaseIOEventDemo() { }
};

int fakemain(int argc, char **argv)
{
  BrailleTutor		bt;
  boost::mutex		mutex_quit;
  boost::condition	cond_quit;

  std::cout << "Registering handler..." << std::endl;
  // The user can add the --debounce command line argument to debounce the
  // stylus inputs. Otherwise the BaseIOEvents are passed directly to the
  // demo handler.
  ShortStylusSuppressor debouncer(0.3);
  BaseIOEventDemo demo_handler(cond_quit, bt);
  if((argc > 1) && !strcmp(argv[1], "--debounce")) {
    std::cout << "[ DEBOUNCING ENABLED ]" << std::endl;
    debouncer.setBaseIOEventHandler(demo_handler);
    bt.setBaseIOEventHandler(debouncer);
  }
  else bt.setBaseIOEventHandler(demo_handler);

  std::cout << "Initialization..." << std::endl;
  bt.init();

  std::cout << "Detection..." << std::endl;
  std::string io_port;
  unsigned int version;
  bt.detect(io_port, version);
  std::cout << "  found a version " << version
	    << " tutor on " << io_port << std::endl;
  std::cout << "Press one of the outer buttons to quit..." << std::endl;

  // Wait for the user to press the quit button on the BT
  boost::mutex::scoped_lock lock_quit(mutex_quit);
  cond_quit.wait(lock_quit);
  std::cout << "Quitting..." << std::endl;
  // Using exit() to stop the program is important. On Windows it cleans up
  // the threads; just ending the program with return causes an error dialog.
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
