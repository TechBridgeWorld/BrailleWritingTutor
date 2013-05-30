#ifndef _LIBBT_SERIAL_IO_H_
#define _LIBBT_SERIAL_IO_H_
/*
 * serial_io.h, started 22 June 2007 by Tom Stepleton (tss@ri.cmu.edu).
 *
 * Implements serial port I/O.
 */

#include <deque>
#include <string>
#include <climits>
#include <stdint.h>

#include "Types.h"

#ifdef BT_WINDOWS
#include <Windows.h>
#else
#include <cerrno>
#include <cstdio>
#include <unistd.h>
#endif

#include <iostream>

namespace BrailleTutorNS {

#ifdef BT_WINDOWS
//! File handle type for the serial port
typedef HANDLE serial_handle;
//! Default initialization value for serial handle
static const serial_handle INVALID_SERIAL_HANDLE = INVALID_HANDLE_VALUE;

//! A handy routine for throwing communication exceptions on Windows
inline void win_io_barf(const std::string &s,
			const BTException::Type &type = BTException::BT_EIO)
{
  DWORD last_err = GetLastError();

  LPVOID lpMsgBuf;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, NULL);

  std::string errstr = s + (char*) lpMsgBuf;
  LocalFree(lpMsgBuf);
  throw BTException(type, errstr);
}
#else
//! File handle type for the serial port
typedef int serial_handle;
//! Default initialization value for serial handle
static const serial_handle INVALID_SERIAL_HANDLE = UINT_MAX;
#endif

//! Creates a listing of serial ports to try in detect()

//! Generates a list of serial ports to try out when attempting to detect
//! BrailleTutor devices. This list has to be made dynamically because some
//! platforms (MacOS) have a dynamic naming scheme for USB serial devices.
//! The returned values use this library's own string-based identifier for
//! serial ports; on UNIX systems this is HIGHLY likely to be the path to the
//! serial device.
std::deque<std::string> serial_suggest_ports();

//! Open a serial port identified by a string identifier

//! Opens a serial port identified by the library's own string representation
//! scheme (more notes in serial_suggest_ports()). On UNIX systems attempts
//! several locking methods, including UUCP lockfiles if a lockfile directory
//! can be found and POSIX locks. On Windows system serial port access is
//! exclusive anyway (check?). Throws appropriate exceptions on error.
//! BT_EIO exceptions always correspond to a critical I/O error; other
//! exceptions may refer to less urgent situations (e.g. port busy); see
//! code for details.
void serial_open(const std::string &port, serial_handle &handle);

//! Close an open serial port

//! Closes an open serial port; also, releases locks on UNIX systems if you
//! furnish the name of the serial port. Will set the handle to
//! INVALID_SERIAL_HANDLE.
//! Throws appropriate exceptions on error.
//! BT_EIO exceptions always correspond to a critical I/O error; other
//! exceptions may refer to less urgent situations (e.g. port busy); see
//! code for details.
void serial_close(serial_handle &handle, const std::string &port="");

//! Writes bytes in a container to a serial port

//! Writes bytes in a container one by one to a serial port. Not particularly
//! efficient, but simple and sufficient for the BT hardware. Throws
//! appropriate exceptions on error.
template <typename InputIterator>
void serial_write(serial_handle &handle,InputIterator begin,InputIterator end);
// Implementation below...

//! Read bytes from the serial port into a container

//! Reads "as many bytes as possible" in from the serial port. This read is
//! a nonblocking read---it will pull in as many bytes as it can from the
//! serial buffer, which may be none. Programs that use this routine will
//! need their own scheduling strategy for reads, which in the BT library
//! is polling at a relatively infrequent interval (30ms). Reads byte-by-byte;
//! again inefficient but simple and sufficient. Returns the number of bytes
//! read; throws appropriate exceptions on error.
template <typename OutputIterator>
unsigned int serial_read(serial_handle &handle, OutputIterator begin);
// Implementation below...

// Need:
// serial_flush? can just do lots of serial_read
// way to check that return serial_fd is valid

/////////////////////////////////////////
//// TEMPLATE METHOD IMPLEMENTATIONS ////
/////////////////////////////////////////
template <typename InputIterator>
void serial_write(serial_handle &handle, InputIterator begin, InputIterator end)
{
  while(begin != end) {
    const uint8_t outbyte = *begin;

#ifdef BT_WINDOWS
    DWORD num_written;
    if(!WriteFile(handle, &outbyte, sizeof(uint8_t), &num_written, NULL))
      win_io_barf("serial port write error: ");

    switch(num_written) {
    case sizeof(uint8_t):	++begin; break;
    case 0:				 break;
    default: throw BTException(BTException::BT_EIO,
			       "unexpeted return value in serial port write");
    }
#else
//std::cerr << "OUTBYTE: " << (unsigned int) outbyte << std::endl;
    switch(write(handle, &outbyte, sizeof(uint8_t))) {
    case sizeof(uint8_t):	++begin; break;
    case 0:				 break;
    case -1: throw BTException(BTException::BT_EIO,
		  std::string("serial port write error: ") + strerror(errno));
    default: throw BTException(BTException::BT_EIO,
				 "unexpeted byte count in serial port write");
    }
#endif

    // Evidently this delay is necessary to avoid overwhelming the BT.
    // The original value of 9ms was determined to be sufficient for
    // just sending bytes to the BT. However, a longer delay was deemed
    // necessary to avoid corruption when the BT was simultaneously
    // producing beeps and handling button presses.
    //TimeInterval(.009).sleep();
    TimeInterval(.02).sleep();
  }
}


template <typename OutputIterator>
unsigned int serial_read(serial_handle &handle, OutputIterator begin)
{
  unsigned int bytes_read = 0;

  bool done_reading = false;
  while(!done_reading) {
    uint8_t inbyte = 200;
#ifdef BT_WINDOWS
    DWORD num_read;
    if(!ReadFile(handle, &inbyte, sizeof(uint8_t), &num_read, NULL))
      win_io_barf("serial port read error: ");

    switch(num_read) {
      case sizeof(uint8_t): *begin++ = inbyte; ++bytes_read; break;
      case 0:				done_reading = true; break;
      default: throw BTException(BTException::BT_EIO,
				  "unexpeted byte count in serial port read");
    }
#else
    switch(read(handle, &inbyte, sizeof(uint8_t))) {
      case sizeof(uint8_t):  *begin++ = inbyte; ++bytes_read; break;
      case 0:				 done_reading = true; break;
      case -1:
	// Got interrupted during read; try again.
	if(errno == EINTR) break;
	// No bytes waiting for us; let's stop trying to read for now
	else if(errno == EAGAIN) { done_reading = true; break; }
	// Some other error happened
	throw BTException(BTException::BT_EIO,
		    std::string("serial port read error: ") + strerror(errno));
      default:
	throw BTException(BTException::BT_EIO,
			  "unexpeted return value in serial port read");
    }
#endif
//if(!done_reading) std::cerr << "INBYTE: " << (unsigned int)inbyte << std::endl;
  }

  return bytes_read;
}

} // namespace BrailleTutorNS

#endif
