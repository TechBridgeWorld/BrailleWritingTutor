/*
 * serial_io_unix.cc, started 24 June 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Implements serial port I/O for UNIX systems
 * XXX XXX XXX LICENSING NOTE XXX XXX XXX
 * Some code in this file was heavily inspired by minicom's main.c (where
 * noted). Those who would wish to license this code non-GPL should consider
 * replacing the code.
 */

#include <deque>
#include <string>
#include <cerrno>
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <termios.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring> //g++ 4.3.2

#ifdef BT_MACOS_X
#include <glob.h>
#endif

#include "serial_io.h"

namespace BrailleTutorNS {

//! The name of the lockfile directory.
const std::string LOCK_DIR("/var/lock");

// Makes a listing of serial ports to try in detect().
// Some of the ports may not actually exist---serial_open() will just throw
// an exception and the autodetector should move on to the next port.
std::deque<std::string> serial_suggest_ports()
{
  std::deque<std::string> suggestions;

#if defined(BT_LINUX)
  suggestions.push_back("/dev/ttyUSB0"); suggestions.push_back("/dev/ttyUSB1");
  suggestions.push_back("/dev/ttyUSB2"); suggestions.push_back("/dev/ttyUSB3");
  suggestions.push_back("/dev/ttyUSB4"); suggestions.push_back("/dev/ttyUSB5");
  suggestions.push_back("/dev/ttyUSB6"); suggestions.push_back("/dev/ttyUSB7");
#elif defined(BT_MACOS_X)
  // Serial port device files in OS X devfs are dynamically generated. We
  // look for them in /dev ourselves. If we don't match anything, we just
  // return the empty list.
  glob_t matched;
  if(glob("/dev/tty.usbserial*", 0, NULL, &matched)) return(suggestions);
  for(char **match=matched.gl_pathv; *match; ++match)
    suggestions.push_back(*match);
  globfree(&matched);
#else
#error "must #define or -D a host OS: current options are BT_LINUX, BT_MACOS_X"
#endif

  return suggestions;
}

// Find out the name of the UUCP lockfile to use when locking the
// serial port device. The lockfile renaming strategy is the same as
// minicom's.
// NOTE: SVR4 style locks NOT IMPLEMENTED---are these used anywhere? TODO?
// are serial ports used anywhere anymore?
std::string portname_to_lockname(const std::string &port)
{
  std::string devname;
  // normal behavior for a lockfile in dev
  if(port.substr(0, 5) == "/dev/") {
    devname = port.substr(5);
    for(unsigned int i=0; i<devname.size(); ++i)
      if(devname[i] == '/') devname[i] = '_';
  }
  // special handling for a lockfile not in /dev. In this case we just use
  // the text after the last /.
  else {
    std::string::size_type last_slash_pos = port.rfind('/');
    if(last_slash_pos == std::string::npos) devname = port;
    else if(last_slash_pos == (port.size() - 1)) devname = "ODD_TRAILING_SLASH";
    else devname = port.substr(last_slash_pos+1);
  }

  return LOCK_DIR + "/LCK.." + devname;
}

// Open a serial port identified by a string identifier
// Also: uses UUCP lockfiles (if possible) and POSIX file locking to try and
// lock access to the serial port.
// TODO: Add special case opening of a UNIX domain socket for local
// simulation.
void serial_open(const std::string &port, serial_handle &handle)
{
  // Useful in two spots
  struct stat stats;

  // Make sure this port exists. Some port "suggestions" don't.
  if(stat(port.c_str(), &stats))
    throw(BTException(BTException::BT_ENOENT,
		      std::string("serial port ")+port+" does not exist"));

  // UUCP locks first. Figure out if there's even a lockfile dir; if not,
  // don't even bother.
  if((!stat(LOCK_DIR.c_str(), &stats)) && (S_ISDIR(stats.st_mode))) {
    // First generate the name of the lockfile we'll use
    const std::string lockname(portname_to_lockname(port));

    // Next try to open the lockfile readonly and pull out the PID inside
    const int lf_ro_fd = open(lockname.c_str(), O_RDONLY);
    if(lf_ro_fd >= 0) {
      char buf[128];
      const unsigned int count = read(lf_ro_fd, buf, 127);
      buf[127] = '\0';

      int pid = -1;
      // Kermit-style lockfile
      if(count == 4) pid = *(int *)buf;
      // Regular ASCII lockfile
      else { std::istringstream pid_in(buf); pid_in >> pid; }

      // We got a pid; see if the process is there.
      if((pid > 0) && (kill((pid_t) pid, 0) < 0) && (errno == ESRCH)) {
	// Process isn't there. Try to delete the stale lockfile.
	// The minicom source has a one second sleep, so I guess
	// we'll do the same. Using TimeInterval because it's thread safe.
	TimeInterval(1.0).sleep();
	unlink(lockname.c_str());
      }
      // Process is there; throw an exception
      else throw(BTException(BTException::BT_EBUSY,
			   std::string("serial port ")+port+" already in use"));
    }

    // Next try to make the lockfile.
    const mode_t tmp_umask = umask(022);
    const int lf_wo_fd =
      open(lockname.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0666);
    // If this failed, we press on; it's risky, but we've already verified
    // by now that any lockfile present is stale. Attempts to create a
    // lockfile will likely fail anyway since this code shouldn't be SUID.
    // Since serial I/O is so rare on machines today anyway, I don't think
    // it's bad to risk stepping on someone's toes. UUCP lockfiles are
    // *really stupid*!
    if(lf_wo_fd >= 0) {
      std::ostringstream lf_out;
      // Getting a username in a thread-safe way is a bother. I'm not
      // gonna do it.
      lf_out << (long) getpid() << " braille_tutor_library root"
	     << std::endl;
      write(lf_wo_fd, lf_out.str().c_str(), lf_out.str().size());
      close(lf_wo_fd);
    }
    // Return umask to original value
    umask(tmp_umask);
  }


  // Thank heavens that's taken care of. UUCP locks sure are craptacular.
  // At last we can open the serial port itself.
  const std::string openerr("error opening serial port");
  handle = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
  if(handle < 0) {
    BTException::Type etype;

    switch(errno) {
    case EBUSY:		etype = BTException::BT_EBUSY;     break;
    case ENOENT:	etype = BTException::BT_ENOENT;    break;
    case EACCES:	etype = BTException::BT_EACCES;    break;
    case EALREADY:	etype = BTException::BT_EALREADY;  break;
    default:		etype = BTException::BT_EIO;       break;
    }

    throw(BTException(etype,
		      openerr + ' ' + port + " (1): " + strerror(errno)));
  }


  // Try out POSIX locking on the serial port file
  if(lockf(handle, F_TLOCK, 0)) {
    if(errno == EAGAIN) {
      close(handle);
      throw(BTException(BTException::BT_EBUSY,
			std::string("serial port ")+port+" already in use"));
    }
    else if((errno != ENOLCK) && (errno != EINVAL)) {
      close(handle);
      throw(BTException(BTException::BT_EIO,
			openerr + ' ' + port + " (2): " + strerror(errno)));
    }
    // We don't care about ENOLCK or EINVAL---again, we soldier on; if the
    // filesystem won't lock for us, then phooey on it. We'll hope the
    // computer only has Braille Tutors plugged into it---a quick listen on
    // a port that another device is already listening to will reveal no
    // "nnnnnnnnnnnnn" autodetect bytes, and we'll move on.
  }


  // Retrieve serial port configuration information and set appropriate options
  struct termios options;
  if(tcgetattr(handle, &options) == -1) {
    close(handle);
    throw(BTException(BTException::BT_EIO,
		      openerr + ' ' + port + " (3): " + strerror(errno)));
  }

  cfsetispeed(&options, B57600);
  cfsetospeed(&options, B57600);

  options.c_cflag |= (CLOCAL | CREAD);

  options.c_cflag &= ~(PARENB | PARODD);	// no parity
  options.c_cflag &= ~CSTOPB;			// one stop bit
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;			// eight bits/byte.

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // raw mode input
  options.c_iflag &= ~(IXON | IXOFF | IXANY); // no software flow control
  options.c_oflag &= ~OPOST; // raw mode output

  if(tcsetattr(handle, TCSANOW, &options) == -1) {
    close(handle);
    throw(BTException(BTException::BT_EIO,
		      openerr + ' ' + port + " (4): " + strerror(errno)));
  }
}


// Closes an open serial port and releases locks on the port.
void serial_close(serial_handle &handle, const std::string &port)
{
  struct stat stats;

  // Close the serial port
  close(handle);
  handle = INVALID_SERIAL_HANDLE;

  // If the user provided a port name, see if the lockfile directory exists;
  // if so, try to delete the UUCP lockfile.
  if((!port.empty()) &&
     (!stat(LOCK_DIR.c_str(), &stats)) && (S_ISDIR(stats.st_mode))) {
    // First generate the name of the lockfile we used...
    const std::string lockname(portname_to_lockname(port));
    // ... then try to kill it. If it worked, huzzah.
    unlink(lockname.c_str());
  }

  // POSIX lock releases automatically.
}

} // namespace BrailleTutorNS
