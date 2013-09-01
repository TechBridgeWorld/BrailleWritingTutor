/*
 * serial_io_windows.cc, started 6 Sept. 2007 by Tom Stepleton (tss@ri.cmu.edu)
 *
 * Implements serial port I/O for Windows systems. Might be easier than UNIX
 * since Windows serial ports are exclusive open.
 */

#include <deque>
#include <string>

#include <Windows.h>
#include <WinError.h>

#include "serial_io.h"

namespace BrailleTutorNS {

// Makes a listing of serial ports to try in detect().
// Some of the ports may not actually exist---serial_open() will just throw
// an exception and the autodetector should move on to the next port.
std::deque<std::string> serial_suggest_ports()
{
  std::deque<std::string> suggestions;

  suggestions.push_back("COM1");
  suggestions.push_back("COM2");
  suggestions.push_back("COM3");
  suggestions.push_back("COM4");
  suggestions.push_back("COM5");
  suggestions.push_back("COM6");
  suggestions.push_back("COM7");
  suggestions.push_back("COM8");
  suggestions.push_back("COM9");
  // NB: For COM ports higher than 9, MSDN suggests calling them
  // "\\\\.\\COM10", e.g. Huh.
  suggestions.push_back("\\\\.\\COM10");
  suggestions.push_back("\\\\.\\COM11");
  suggestions.push_back("\\\\.\\COM12");
  suggestions.push_back("\\\\.\\COM13");
  suggestions.push_back("\\\\.\\COM14");
  suggestions.push_back("\\\\.\\COM15");
  suggestions.push_back("\\\\.\\COM16");

  return suggestions;
}

// Open a serial port identified by a string identifier
// TODO: Add special case opening of a Windows pipe for local simulation.
void serial_open(const std::string &port, serial_handle &handle)
{
  // Well, here goes nothin'. Try to open the file.
  handle = CreateFile(port.c_str(), GENERIC_READ | GENERIC_WRITE,
		      0, // no sharing
		      NULL, // child procs don't inherit handles
		      OPEN_EXISTING, // don't create a new file
		      0, // no weird flags or nuthin'.
		      NULL); // no handle to copy params from
  if(handle == INVALID_HANDLE_VALUE) {
    // In all honesty I have no idea whether these errors are being properly
    // mapped onto our own set of errors. It's cute how Windows sticks them
    // all in one ginormous namespace, though.
    switch(GetLastError()) {
    case ERROR_DIRECTORY:
    case ERROR_BAD_PATHNAME:
    case ERROR_FILE_INVALID:
      win_io_barf(std::string(port)+" invalid: ", BTException::BT_EINVAL);
      break;

    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
    case ERROR_BAD_UNIT:
    case ERROR_NOT_SUPPORTED:
    case ERROR_SERIAL_NO_DEVICE:
      win_io_barf(std::string("serial port ")+port+" not found: ",
		  BTException::BT_ENOENT);
      break;

    case ERROR_ACCESS_DENIED:
    case ERROR_INVALID_ACCESS:
      win_io_barf(std::string("serial port ")+port+" access denied: ",
		  BTException::BT_EACCES);
      break;

    case ERROR_BUSY:
    case ERROR_NOT_READY:
    case ERROR_ALREADY_ASSIGNED:
    case ERROR_IRQ_BUSY:
    case ERROR_DEVICE_IN_USE:
      win_io_barf(std::string("serial port ")+port+" busy: ",
		  BTException::BT_EBUSY);
      break;

    default:
      win_io_barf(std::string("error opening serial port ")+port+": ");
    }
  }

  // Now we try to set the serial port parameters.
  DCB port_dcb;
  ZeroMemory(&port_dcb, sizeof(DCB));
  port_dcb.DCBlength = sizeof(DCB);
  if(!GetCommState(handle, &port_dcb))
    win_io_barf(std::string("error retrieving serial port ")+port+" state: ");

  port_dcb.BaudRate = CBR_57600;
  port_dcb.ByteSize = 8;
  port_dcb.Parity   = NOPARITY;
  port_dcb.StopBits = ONESTOPBIT;

  port_dcb.fBinary           = TRUE;
  port_dcb.fParity           = FALSE;
  port_dcb.fOutxCtsFlow      = FALSE;
  port_dcb.fOutxDsrFlow      = FALSE;
  port_dcb.fDtrControl       = DTR_CONTROL_ENABLE;
  port_dcb.fDsrSensitivity   = FALSE;
  port_dcb.fTXContinueOnXoff = TRUE;
  port_dcb.fOutX             = FALSE;
  port_dcb.fInX              = FALSE;
  port_dcb.fErrorChar        = FALSE;
  port_dcb.fNull             = FALSE;
  port_dcb.fRtsControl       = RTS_CONTROL_ENABLE;
  port_dcb.fAbortOnError     = FALSE;

  if(!SetCommState(handle, &port_dcb))
    win_io_barf(std::string("error setting serial port ")+port+" parameters: ");

  // Okay, well that sure was fun. Now to set the port timeouts.
  COMMTIMEOUTS port_timeouts;
  ZeroMemory(&port_timeouts, sizeof(COMMTIMEOUTS));
  if(!GetCommTimeouts(handle, &port_timeouts))
    win_io_barf(std::string("error getting serial port ")+port+" timeouts: ");

  // Are these right? Who knows?
  port_timeouts.ReadIntervalTimeout         = MAXDWORD;
  port_timeouts.ReadTotalTimeoutMultiplier  = 0;
  port_timeouts.ReadTotalTimeoutConstant    = 5;
  port_timeouts.WriteTotalTimeoutMultiplier = 0;
  port_timeouts.WriteTotalTimeoutConstant   = 50;

  if(!SetCommTimeouts(handle, &port_timeouts))
    win_io_barf(std::string("error setting serial port ")+port+" timeouts: ");
}

// Closes an open serial port. In Windows, the second arg is not used...
// there's no lock file to clean up.
void serial_close(serial_handle &handle, const std::string&)
{
  CloseHandle(handle);
}

} // namespace BrailleTutorNS
