package edu.cmu.controller;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * A class using the <a href="http://http://rxtx.qbang.org/">RXTX</a>
 *  library to create communication to a serial COM port.  <br/>
 *  The basic structure of this class is based on the example codes from RXTX.
 * @see <a href="http://rxtx.qbang.org/wiki/index.php/Two_way_communcation_with_the_serial_port">RXTX's example</a>
 */
public class TwoWaySerialComm {

	private OutputStream out;
	private InputStream in;

	public OutputStream getOutputStream(){
		return out;
	}
	
	public InputStream getInputStream(){
		return in;
	}
	
	public TwoWaySerialComm(String port) throws Exception {
		super();
		this.connect(port);
	}

	public void connect(String portName) throws Exception {
		CommPortIdentifier portIdentifier = CommPortIdentifier
				.getPortIdentifier(portName);
		if (portIdentifier.isCurrentlyOwned()) {
			System.out.println("Error: Port is currently in use");
		} else {
			CommPort commPort = portIdentifier.open(this.getClass().getName(),
					2000);

			if (commPort instanceof SerialPort) {
				SerialPort serialPort = (SerialPort) commPort;
				serialPort.setSerialPortParams(57600, SerialPort.DATABITS_8,
						SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);

				in = serialPort.getInputStream();
				out = serialPort.getOutputStream();

			} else {
				System.out
						.println("Error: Only serial ports are handled by this example.");
			}
		}
	}

}
