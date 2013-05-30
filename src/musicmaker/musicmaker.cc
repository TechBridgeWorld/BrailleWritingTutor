/*
 * musicmaker.cc
 *
 */

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

#include "musicmaker.h"

#define MAXCELLS 32
#define MAXNOTES 6

#define MAXINSTRUMENTS 1

//For Tempo
#define SLOWEST 5
#define FASTEST 1

//For Nuking Buttons
#define NONEDOWN 0
#define THREEDOWN 1
#define SIXDOWN 2

// Thread functor that handles playing notes at each beat
struct FunctorTimerTaskMM {
  const MusicMaker& mm;
  unsigned int turn; //the turn at which this thread was invoked
  int &tempo;
  boost::mutex &tone_matrix_mutex;

  // constructor
  inline FunctorTimerTaskMM(const MusicMaker& my_mm, unsigned int my_turn, int my_tempo, boost::mutex &my_tone_matrix_mutex) :
      mm(my_mm), turn(my_turn), tempo(my_tempo), tone_matrix_mutex(my_tone_matrix_mutex) { }

  // main loop of thread
  inline void operator()()
  {
    while(mm.loop){
      BT_sleep(tempo);
      //sleep(7); //Good for testing the concurrency
      
      // grab the mutex for the tone matrix array
      boost::mutex::scoped_lock lock_m(tone_matrix_mutex);
      
      tempo = mm.playNote(turn++);
      if(turn == 32)
        turn = 0;
    }
    return; // thread ends when looping stops
  }
};


//boost::try_mutex MusicMaker::mx;
//boost::try_mutex mx;
//Voice soundPlayer("./resources/Voice/musicmaker/");
static unsigned int current_note;
//TimerTaskMM * t;
//boost::thread * b;
boost::scoped_ptr<boost::thread> timer_thread;

MusicMaker::MusicMaker(IOEventParser& my_iep, const std::string& path_to_mapping_file, SoundsUtil* my_su, std::string path_to_musicmakersounds) : 
  IBTApp(my_iep, path_to_mapping_file), loop(true), su(my_su), musicmakersounds(path_to_musicmakersounds), iep(my_iep), curr_instrument(piano), tempo(2), but_status(NONEDOWN)
{
	for(int i = 0; i < MAXCELLS; i++){
		for(int j = 0; j < MAXCELLS; j++){
			tone_matrix[i][j] = false;
		}
	}
	current_note = 0;
	//t = new TimerTaskMM(*this, 0, tempo);
  //b = new boost::thread(*t);
  timer_thread.reset(new boost::thread(FunctorTimerTaskMM(*this, 0, tempo, tone_matrix_mutex)));
}

MusicMaker::~MusicMaker()
{
	stop();
	delete su;
}

void MusicMaker::processEvent(IOEvent& e) {
	
	if(e.type == IOEvent::STYLUS_DOWN){
		changeKey(e);
	}
	
	if((e.type == IOEvent::BUTTON_DOWN)) {
		if(e.button == 1 || e.button == 4) {
			changeTempo(e);
		}
		else if(e.button == 2 || e.button == 5) {
			changeInstrument(e);
		}

  //  else if(e.button == 3 || e.button == 6)
  //    nukeKeys();
  //}

		else if(e.button == 3) {
			if (but_status == SIXDOWN) {
				but_status = NONEDOWN;
				nukeKeys();
			}
			else {
				but_status = THREEDOWN;
			}
		}
		else if (e.button == 6) {
			if(but_status == THREEDOWN) {
				but_status = NONEDOWN;
				nukeKeys();
			}
			else {
				but_status = SIXDOWN;
			}
		}
  }
  if(e.type == IOEvent::BUTTON_UP) {
    if(e.button == 3 && but_status == THREEDOWN)
      but_status = NONEDOWN;
    if(e.button == 6 && but_status == SIXDOWN)
      but_status = NONEDOWN;
  }
}

int MusicMaker::playNote(unsigned int note_to_play) const {

  //boost::try_mutex::scoped_try_lock lock(mx); //try to acquire a lock
    
  //printf("Inside playNote()\n");
  
  
  //if( lock.locked() && note_to_play == current_note ) {
	char playWhat[32];
	printf("Playing note %i\n", current_note);
	sprintf(playWhat, "mid");
	if (tone_matrix[current_note][0]){
		sprintf(playWhat, "%sC", playWhat);
	}
	if (tone_matrix[current_note][1]){
		sprintf(playWhat, "%sE", playWhat);
	}
	if (tone_matrix[current_note][2]){
		sprintf(playWhat, "%sG", playWhat);
	}
	sprintf(playWhat, "%sh", playWhat);
	if (tone_matrix[current_note][3]){
		sprintf(playWhat, "%sC", playWhat);
	}
	if (tone_matrix[current_note][4]){
		sprintf(playWhat, "%sE", playWhat);
	}
	if (tone_matrix[current_note][5]){
		sprintf(playWhat, "%sG", playWhat);
	}
	//sprintf(playWhat, "%s.wav", playWhat);
  if (strcmp(playWhat,"midh") != 0) {
	  //soundPlayer.play(playWhat, tempo/1e3);
    su->saySound(musicmakersounds, playWhat);
  }
	if (++current_note == MAXCELLS) {
		current_note = 0;
	}
//}
  return tempo;
}

void MusicMaker::changeTempo(IOEvent& e){
  boost::mutex::scoped_lock lock_m(tone_matrix_mutex);
	if(e.button == 1){
		if(tempo < SLOWEST){
			tempo += 1;
		}
	}
	else if(e.button == 4){
		if(tempo > FASTEST){
			tempo -= 1;
		}
	}
	printf("Tempo is now %d\n", tempo);
}

void MusicMaker::changeInstrument(IOEvent& e){
	if(e.button == 1){
		curr_instrument = (instrument) (curr_instrument - 1);
		if(curr_instrument == -1){
			curr_instrument = (instrument) (MAXINSTRUMENTS - 1);
		}
	}
	else if(e.button == 4){
		curr_instrument = (instrument) (curr_instrument + 1);
		if(curr_instrument == MAXINSTRUMENTS){
			curr_instrument = (instrument) (0);
		}
		
	}
}

void MusicMaker::nukeKeys(){
  printf("inside nukeKeys()\n");
  //boost::try_mutex::scoped_try_lock lock(mx); //try to acquire a lock
  //if( lock.locked() ) {
    boost::mutex::scoped_lock lock_m(tone_matrix_mutex);
    for(int i = 0; i < MAXCELLS; i++){
      for(int j = 0; j < MAXCELLS; j++){
        tone_matrix[i][j] = false;
      }
    }
  //}
  return;
}

void MusicMaker::changeKey(IOEvent& e) {
	int rownum;
	int tone;
	if(e.dot <= 2){
		if(e.cell > 16)
			rownum = ((e.cell - 17)*2)+1;
		else
			rownum = ((e.cell - 1)*2)+1;
	}
	else{
		if(e.cell > 16)
			rownum = (e.cell - 16) * 2;
		else
			rownum = e.cell * 2;
	}
	if(e.cell <= 16){
		if(rownum % 2 == 1)
			tone = e.dot;
		else
			tone = e.dot - 3;
	}
	else {
		if(rownum % 2 == 1)
			tone = e.dot + 3;
		else
			tone = e.dot;
	}
	if(e.type == IOEvent::STYLUS_DOWN){
		tone_matrix[rownum][tone] = !tone_matrix[rownum][tone];
		if(tone_matrix[rownum][tone])
			printf("row %i, tone %i is now on\n", rownum, tone);
		else
			printf("row %i, tone %i is now off\n", rownum, tone);
	}
}

void MusicMaker::stop(){
  printf("inside stop()\n");
	loop = false;
	BT_sleep(SLOWEST); // wait for notes to finish playing
	if(timer_thread.get()) timer_thread->join();
	//b->join();
	//delete b;
	printf("thread joined!\n");
	
	//delete t;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EnglishMusicMaker::EnglishMusicMaker(IOEventParser& my_iep) :
  MusicMaker(my_iep, "", new EnglishSoundsUtil, "./resources/Voice/musicmaker/")
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
KiswahiliMusicMaker::KiswahiliMusicMaker(IOEventParser& my_iep) :
  MusicMaker(my_iep, "", new KiswahiliSoundsUtil, "./resources/Voice/musicmaker/")
{
}
