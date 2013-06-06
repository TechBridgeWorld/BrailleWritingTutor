/*
 * Braille Tutor interface library
 * TimeInterval.cc, started 1 June 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Implements the now() named constructor of the TimeInterval class (see
 * Types.h), which creates a TimeInterval object representing the elapsed
 * time since some unspecified epoch (in this implementation, near the
 * start of the program).
 */

#include "Types.h"

#include <boost/thread/condition.hpp>
#include <boost/thread.hpp>

namespace BrailleTutorNS {

#ifdef BT_WINDOWS
//
// Windows implementation
//
#include "Windows.h"

// Returns the current time (Windows version).
inline static LARGE_INTEGER get_now()
{
  FILETIME now;
  GetSystemTimeAsFileTime(&now);
  LARGE_INTEGER now_as_lint;
  now_as_lint.LowPart  = now.dwLowDateTime;
  now_as_lint.HighPart = now.dwHighDateTime;
  return now_as_lint;
}

// The epoch---here, the exact moment of program start.
static LARGE_INTEGER epoch = get_now();

// Implementation of TimeInterval::now() for Windows.
TimeInterval TimeInterval::now()
{
  LARGE_INTEGER now_as_lint = get_now();
  LONGLONG interval = now_as_lint.QuadPart - epoch.QuadPart;

  LONGLONG secs = interval / ((LONGLONG) 1e7);
  LONGLONG usecs = (interval % ((LONGLONG) 1e7)) / ((LONGLONG) 1e4);

  return TimeInterval((unsigned int) secs, (unsigned short int) usecs);
}


#else
//
// UNIX implementation
//
#include <ctime>
#include <sys/time.h>

// The epoch---here, the nearest second less than the exact moment
// of program start.
static time_t epoch = time(NULL);

// Implementation of TimeInterval::now() for UNIX
TimeInterval TimeInterval::now()
{
  struct timeval tval;
  gettimeofday(&tval, NULL);

  return TimeInterval((unsigned int) (tval.tv_sec - epoch),
		      (unsigned int) (tval.tv_usec / 1000));
}

#endif


// Portable fine-resolution sleeping. This is KLUDGY, and Boost doesn't
// guarantee resolution better than a second for the technique we're using
// here. Nevertheless, this is nominally portable. We do it by making a
// mutex, grabbing it, and then waiting on a condition that's never satisfied.
void TimeInterval::sleep() const
{
  // First get the time right now
  boost::xtime time_begin;
  boost::xtime_get(&time_begin, boost::TIME_UTC_);	// Gary: Changed from TIME_UTC to TIME_UTC_ in 1.53.0

  // Now set up the dummy mutex and condition variable, then grab mutex
  boost::mutex			dummy_mutex;
  boost::condition		dummy_wait;
  boost::mutex::scoped_lock	dummy_lock(dummy_mutex);

  // Determine end time of wait
  boost::xtime time_end(time_begin);
  time_end.sec += secs;
  const unsigned int nsecs = msecs * 1000000;
  time_end.nsec += nsecs % 1000000000;
  time_end.sec  += nsecs / 1000000000;

  // Wait out the rest of our sentence
  dummy_wait.timed_wait(dummy_lock, time_end);
}

} // namespace BrailleTutorNS
