#ifndef VOICE_H_
#define VOICE_H_

#include <map>
#include <string>
#include <iostream>

#include <stddef.h>
#include <sys/types.h>
#include <dirent.h>

#include <SDL.h>
#include <SDL_mixer.h>

#include <IOEvent.h>
//#include "common/IBTApp.h"

#include "common/utilities.h"


//#include "C:\Users\iSTEP2013\Documents\BWT\src\common\IBTApp.h"
//#include "common/language_utils.h"
//#include "C:\Users\iSTEP2013\Documents\BWT\src\common\utilities.h"
//include "common/utilities.h"

void waitForChannel(int channel);

//used internally for sound_map
struct  string_lt{
  bool operator()(std::string s1, std::string s2){
    return s1 < s2;
  }
};

typedef std::map<std::string, Mix_Chunk*, string_lt> string_to_sound_map;

class Voice
{
 public:

  Voice();

  /*
   * creates a voice whose .wav files are in
   * ../resources/Voice/[rdir]
   */
  Voice(std::string rdir, IOEventParser& my_iep);

  ~Voice();

  /*
   * takes an utterance name (filename minus extension)
   * and speaks it on given channel, waiting for that
   * channel to be clear (-1 to make a random new channel).
   * Throws back utterance name if it wasn't in rdir
   * (programmer error, not valid exception state)
   */
  void say(std::string uname, int channel) const;

  void say(std::string uname) const;

	void play(std::string uname, int) const;

 
  bool hasSound(std::string uname) const;

  static void stopAllPlaying();
  IOEventParser& iep;

 private:
  mutable string_to_sound_map sound_map; //sound_map is used inside say().. and say() is declared const. So sound_map should also be const, ofcourse thats not possible because it doesnt make sense to have a const hashmap, so therefore it's mutable. So basically, this implies "I, the say() function, hereby garantee, by the power bestowed upon me by const, that I will not change anything inside me - ofcourse I am a liar because I am changing the sound_map"
  
};
#endif /* VOICE_H_ */
