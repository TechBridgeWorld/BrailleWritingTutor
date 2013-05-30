#include "Voice.h"

Voice::Voice()
{
}
;

Voice::Voice(std::string resource_path)
{

  if( SDL_Init(SDL_INIT_AUDIO) != 0 )
    fprintf(stderr, "Warning: unable to initialize audio: %s\n", SDL_GetError());

  if( Mix_OpenAudio(16000, AUDIO_S16, 2, 512) < 0 )
    fprintf(stderr, "Warning: Audio could not be setup for 16000 Hz 16-bit stereo.\n"
      "Reason: %s\n", SDL_GetError());

  DIR *dp;
  struct dirent *ep;

  dp = opendir(resource_path.c_str());
  if( dp != NULL )
  {
    while( (ep = readdir(dp)) )
    { //loop through entities
      if( ep->d_name[0] != '.' )
      { //real files
        std::string filename = std::string(ep->d_name);
        Mix_Chunk * sound = Mix_LoadWAV((resource_path + filename).c_str());
        if( sound != NULL ){
          sound_map[filename] = sound;
        }
		else
          std::cout << "WARNING: Sound file could not be loaded:" + resource_path + filename << std::endl;
          //std::cout<<"loaded "<<resource_path+filename<<std::endl;
	    
      }
    }
    (void) closedir(dp);
  }
  else
    throw resource_path + " not valid"; //fix me :/
}

Voice::~Voice()
{
  //free all Mix_Chunk*'s
  //std::cout<<"		(DEBUG)INSIDE VOICE DESTRUCTOR";
  for(string_to_sound_map::iterator iter = sound_map.begin(); iter != sound_map.end(); iter++)
  {
    if( iter->second != NULL )
    {
      ///std::cout<<"		(DEBUG)Trying to free:"<<iter->first<<std::endl;
      Mix_FreeChunk(iter->second);
    }
    else
    {
      std::cout << "segfault PREVENTED" << std::endl;
    }
  }
  
  // also need to clean up after SDL here
  Mix_CloseAudio();
  //SDL_Quit(); -- This causes an infinite loop when switching modes (SDL doesn't init after quit)

  //std::cout<<"    (DEBUG)Leaving VOICE DESTRUCTOR"<<std::endl;
}

void Voice::say(std::string uname, int channel) const
{
  if( sound_map.find(uname) == sound_map.end() ) { //if uname wasn't found
    //throw uname;
    std::cout<<"Error!: Sound file not found:"<<uname<<std::endl;

  }

  if( channel != -1 ) //if it's -1, don't wait (doesn't matter)
    waitForChannel(channel); //wait for channel to finish playing

  Mix_PlayChannel(channel, sound_map[uname], 0);
}

void Voice::say(std::string uname) const
{
  if( sound_map.find(uname) == sound_map.end() ) //if uname wasn't found
  {
    std::cout<<"Error!: Sound file not found:"<<uname<<std::endl;
    //throw uname;
  }

  waitForChannel(-1); //clear all channels

  Mix_PlayChannel(-1, sound_map[uname], 0);
}

void Voice::play(std::string uname, int ms) const
{
  //std::cerr<<"Trying to play sound file \"%s\"\n"<<uname<<std::endl;
  
	if( sound_map.find(uname) == sound_map.end() ) //if uname wasn't found
  {
    std::cout<<"Error!: Sound file not found:"<<uname<<std::endl;
    //throw uname;
  }

  Mix_PlayChannelTimed(-1, sound_map[uname], 0, ms);
}

//Check if this Voice object has a particular sound in its map
bool Voice::hasSound(const std::string uname) const
{
  return sound_map.find(uname) != sound_map.end();
}

void Voice::stopAllPlaying()
{
  Mix_HaltChannel(-1);
}

void waitForChannel(int channel)
{
  //wait for channel to finish playing (if -1, all channels)
  while( Mix_Playing(channel) );
}

