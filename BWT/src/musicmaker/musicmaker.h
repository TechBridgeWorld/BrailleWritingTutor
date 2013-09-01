/*
 * musicmaker.h
 *
 */

#ifndef MUSICMAKER_H_
#define MUSICMAKER_H_

#include "common/utilities.h"
#include "common/IBTApp.h"
#include "common/language_utils.h"

class MusicMaker : public IBTApp, public boost::noncopyable
{
public:
  explicit MusicMaker(IOEventParser&, const std::string&, SoundsUtil*, std::string);
  virtual ~MusicMaker();
  void processEvent(IOEvent& e);
  int playNote(unsigned int) const;
	bool loop;

private:	
  SoundsUtil* su;
  Voice musicmakersounds;
  IOEventParser& iep; //So flushGlyph() can be called
	void changeKey(IOEvent& e);
	void changeTempo(IOEvent& e);
	void changeInstrument(IOEvent& e);
	void nukeKeys();
	void stop();
  enum instrument {piano};
  instrument curr_instrument;
  int tempo;
	int but_status;
  bool tone_matrix[32][6];
	boost::mutex tone_matrix_mutex;
  //boost::scoped_ptr<boost::thread> timer_thread;
};

class EnglishMusicMaker : public MusicMaker
{
public:
  explicit EnglishMusicMaker(IOEventParser&);
  ~EnglishMusicMaker()
  {
  }
};

class KiswahiliMusicMaker : public MusicMaker
{
public:
  explicit KiswahiliMusicMaker(IOEventParser&);
  ~KiswahiliMusicMaker()
  {
  }
};



#endif
