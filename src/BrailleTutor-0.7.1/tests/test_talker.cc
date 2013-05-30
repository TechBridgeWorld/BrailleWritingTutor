#include "Types.h"
#include "IOEvent.h"
#include "BrailleTutor.h"

#include "ShortStylusSuppressor.h"

#include <string>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>

using namespace BrailleTutorNS;


// Speak a string using a program called "say".
void say(std::string phrase)
{
  // Translate phrases that sound confusing
  if((phrase == "a") || (phrase == "A")) phrase = "eh";

  const pid_t pid = fork();

  if(pid == 0) { // I am the child; let's do our thing
    daemon(0,0); // first let's background ourselves
    execlp("say", "say", "-v", "Fred", phrase.c_str(), NULL);
  }
  // not thread safe
  else if(pid == -1) throw std::string("fork() failed: ") + strerror(errno);
  else // I am the parent; wait for the child to die.
    for(;;) {
      int status;
      pid_t wpid = waitpid(pid, &status, WNOHANG);
      if((wpid == pid) && (WIFEXITED(status) || WIFSIGNALED(status))) break;
    }
}


struct TalkerDemo : public IOEventHandler {
  // Condition variable that we use to notify main thread to quit.
  boost::condition      &cond_quit;
  // BrailleTutor object that we use to make beeps
  BrailleTutor		&bt;
  // IOEventParser object that we use to flush glyphs under construction.
  IOEventParser         &iep;
  // True iff we're erasing a cell
  bool			cell_erase;

  // Phrase the user is entering
  std::string		phrase;

bool just_erased; // FIXME

  // The event handler
  virtual void operator()(std::deque<IOEvent> &events)
  {               
    while(!events.empty()) {

    // User entered a letter. Modify the phrase the user entered. Note:
    // assumes ASCII or other 8-bit representation. Also, BT must have
    // exactly 32 cells.
    if(events.front().type == IOEvent::CELL_LETTER) {
      char letter_chr[2];
      letter_chr[0] = (char) events.front().letter.str[0];
      letter_chr[1] = '\0';
      std::string letter = letter_chr;
if(just_erased) { just_erased = false; } else // FIXME
      if(letter.empty()) {
	say("Unknown dot pattern");
	std::cerr << "Skipping unknown glyph" << std::endl;
      }
      else {
        say(letter);
	if(events.front().cell <= 16)
	  phrase[16-events.front().cell] = letter[0];
        else
	  phrase[48-events.front().cell] = letter[0];
        std::cerr << "Insert letter " << letter
		  << " (int " << (int) letter[0] << ')' << std::endl;
        std::cerr << "Current phrase: [" << phrase << ']' << std::endl;
      }
    }
    // We only get stylus events if we are deleting one of the cell letters.
    // Delete the letter and then restore us back to taking in letters
    else if(events.front().type == IOEvent::STYLUS) {
      if(cell_erase) {
	if(events.front().cell <= 16) phrase[16-events.front().cell] = ' ';
        else phrase[48-events.front().cell] = ' ';
	say("zap!");
	iep.wantEvent(IOEvent::CELL_LETTER);
	cell_erase = false;
just_erased = true; // FIXME
	std::cerr << "Current phrase: [" << phrase << ']' << std::endl;
      }
      bt.beep(880.0, 0.1);
    }
    else if(events.front().type == IOEvent::BUTTON) {
      switch(events.front().button) {
      // button 0: say text
      case 0:
	say(phrase);
	break;

      // button 1: say help
      case 1:
	say("Write as you would with a normal slate and stylus. If you like, "
	    "you may overwrite letters. The buttons have special functions. "
	    "Press a side button and I will say what you wrote. Button 3 "
	    "will cause the next cell you touch with the stylus to be replaced "
	    "by a space. Button 5 clears everything. Button 6 quits. Press "
	    "button 2 and I will interpret the dots you've just entered into "
	    "a cell as a complete braille letter. I will also do this if "
	    "you move on to another cell or if you wait for a while without "
	    "doing anything. Thanks for trying this demo!");
      // button 2: force glyph insert
      case 2:
	iep.flushGlyph();
	break;

      // button 3: glyph erase. Pay attention to the stylus since that's how
      // we select cells to erase.
      case 3:
	say("cell erase");
	iep.ignoreEvent(IOEvent::CELL_LETTER);
	cell_erase = true;
	break;

      // button 4: clear all text
      case 5:
	say("clear");
	phrase = std::string(32,' ');
	std::cerr << "Current phrase: [" << phrase << ']' << std::endl;
	break;

      // button 6: quit
      case 6:
	say("quit");
	cond_quit.notify_one();
	break;
      default:
	break;
      }
    }
    // Always beep on stylus removal
    //else if(events.front().type == IOEvent::STYLUS_UP) bt.beep(440.0, 0.05);

  events.pop_front();
  } // while(!events.empty())
  }

  inline TalkerDemo(boost::condition &my_cond_quit,
		    BrailleTutor &my_bt, IOEventParser &my_iep)
  : cond_quit(my_cond_quit), bt(my_bt), iep(my_iep), cell_erase(false),
    phrase(32,' ')
,just_erased(false) // FIXME
 { }
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
  TalkerDemo handler(cond_quit, bt, iep);
  iep.setIOEventHandler(handler);

  std::cout << "Subscribing to events..." << std::endl;
  iep.wantEvent(IOEvent::BUTTON);
  iep.wantEvent(IOEvent::CELL_LETTER);
  iep.wantEvent(IOEvent::STYLUS);

  std::cout << "Initialization..." << std::endl;
  bt.init();

  std::cout << "Detection..." << std::endl;
  std::string io_port;
  unsigned int version;
  bt.detect(io_port, version);
  std::cout << "  found a version " << version
	    << " tutor on " << io_port << std::endl;

  // Hello, world!
  say("Ready to go! Press button 1 for help");

  // Wait for the user to quit
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
