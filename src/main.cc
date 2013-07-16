#include <iostream>
#include <unistd.h>
#include <ctime> //g++ 4.3.2
#include "common/IBTApp.h"
#include "common/language_utils.h"
#include "common/utilities.h"


#include "app_dispatcher/app_dispatcher.h"

int launch_bt(int argc, char **argv);

//This hack has something to do with SDL on Windows. Freddie might know the details
#ifdef BT_WINDOWS
#ifdef main
#undef main
#endif
#endif

int main(int argc, char **argv)
{ 
  /* establish the necessary parts to play sounds upon connection */
  IOEventParser event_parser;
  Voice teacher_voice("./resources/Voice/teacher/", event_parser);
  SoundsUtil* eng_su = new EnglishSoundsUtil;
  
  eng_su->saySound(teacher_voice, "starting_bt");
  try
  {
    return launch_bt(argc, argv);
  }
  catch (const BTException &e)
  {
    eng_su->saySound(teacher_voice, "not_connected"); // TODO: put something more intelligent here
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

int launch_bt(int argc, char **argv)
{
  IOEventParser event_parser;
  Voice teacher_voice("./resources/Voice/teacher/", event_parser);
  SoundsUtil* eng_su = new EnglishSoundsUtil;
  std::srand((unsigned) std::time(0));
  BrailleTutor bt;

  std::cout << "Registering IOEventParser..." << std::endl;
  // The user can add the --nodebounce command line argument to
  // turn off stylus input debouncing. If they turn off the
  // debouncind, the BaseIOEvents are passed directly to the
  // IOEventParser. This is a TSS mod!
  ShortStylusSuppressor debouncer(0.3);
  
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


  ApplicationDispatcher ad(bt,event_parser);

  event_parser.setIOEventHandler(ad);
  std::cout << "Initialization..." << std::endl;
  bt.init();

  std::cout << "Detection..." << std::endl;
  std::string io_port;
  unsigned int version;
  bt.detect(io_port, version);
  std::cout << "  found a version " << version << " tutor on " << io_port << std::endl;
  eng_su->saySound(teacher_voice, "connected"); // TODO put something more intelligent here
  eng_su->saySound(teacher_voice, "main_menu"); // announce that back in main menu
  bt.join();

  return 0;
}
