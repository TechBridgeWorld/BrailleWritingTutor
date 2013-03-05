package edu.cmu.controller;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.slf4j.Logger;

import edu.cmu.controller.TwoWaySerialComm;
import edu.cmu.logger.EmulatorLogger;

/**
 * A handler class that handles connection with the serial COM port.
 * 
 * @author ziw
 * 
 */
public class ActionHandler {
	private TwoWaySerialComm com;
	private static final String COM_PROT_NAME = "COM7";
	private boolean handshaking;
	private Logger logger = EmulatorLogger.getEmulatorInfoLogger();
	private Logger debugLogger = EmulatorLogger.getEmulatorDebugLogger();
	
	
	public ActionHandler() {
		// Do any initialization here.
		// constructor is called as soon as the server starts
		handshaking = false;
	}

	/**
	 * Do something with buttonCode sent from ajax request. Requires input
	 * validation.
	 * 
	 * @param buttonCode
	 */
	public void handleButtonCode(String buttonName) {
		if (buttonName == null || buttonName.trim().length() <= 0) {
			logger.warn("Button code is empty.Illegal button code.");
			return;
		}
		if (com == null) {
			logger.error("COM port is not initialized. Can't handle button press.");
			return;
		}

		// TODO using indexOf() for now because of the implementation of the
		// front end
		if (buttonName.indexOf("init") != -1) {
			handShake();
			return;
		}

		String decodedName = null;
		try {
			decodedName = URLDecoder.decode(buttonName, "UTF-8");
			logger.info("Decoded button code: " + decodedName);
			com.getOutputStream().write(decodedName.getBytes());
			
			debugLogger.debug("Button code send: " + buttonName);
			debugLogger.debug("Decodes code    : " + decodedName);
			
		} catch (UnsupportedEncodingException e) {
			logger.error("Unable to decode the button code: " + buttonName);
			logger.error("Exception", e);
		} catch (IOException e) {
			logger.error("IOException when writing to com.out");
			logger.error("Exception", e);
		}

	}

	private void handShake() {
		// tries to handshake
		if (!handshaking) {
			try {
				handshaking = true;
				logger.info("Init caught. Start sending n's");
				logger.info("Server is blocking any further handshake until this one is done.");
				initialize();
			} catch (Exception e) {
				logger.error("Error when  handshaking. ");
				logger.error("Exception", e);
			} finally {
				handshaking = false;// handshake done.
				logger.info("Setting handshaking flag to false. Server is open to accept handshake again.");
			}
		} else {
			// another handshaking in process.
			logger.info("Init caught but another handshaking in being processed. Ignore this handshake.");
		}

	}

	/**
	 * Initialize the TwoWaySerialComm, if it has not been intialized.
	 * 
	 * @throws Exception
	 */
	public void initSerialComm() throws Exception {
		if (com != null)
			return;
		logger.info("Initializing TwoWaySerialComm");
		com = new TwoWaySerialComm(COM_PROT_NAME);// this line takes a while
		logger.info("SerialComm initialized");
	}

	/**
	 * Initialize the emulator. Start sending n's through the COM port to the
	 * BWT software
	 * 
	 * @throws IOException
	 * @throws InterruptedException
	 * @throws ExecutionException 
	 */
	public void initialize() throws IOException, InterruptedException, ExecutionException {
		ExecutorService executor = Executors.newCachedThreadPool();
		Future<?> reader = executor.submit(new SerialReader(com
				.getInputStream()));
		executor.submit(new SerialWriter(com.getOutputStream()));
		reader.get(); // block until reader is done
		executor.shutdownNow(); // kill writer
		while (!executor.isTerminated()); // block until writer is killed
		com.getOutputStream().write("bt".getBytes());
		logger.info("Wrote 'bt'. Hand shaken.");

	}

	private static class SerialReader implements Runnable {
		InputStream in;

		public SerialReader(InputStream in) {
			this.in = in;
		}

		public void run() {
			byte[] buffer = new byte[1024];
			byte[] miniBuffer = new byte[2]; // buffer for the incoming bytes
												// waiting for a b followed by a
												// t
			int len = -1;
			boolean done = false;
			try {
				while (((len = this.in.read(buffer)) > -1) && (done == false)) {
					for (int i = 0; i < len; i++) {
						miniBuffer[0] = miniBuffer[1];
						miniBuffer[1] = buffer[i];
						if (((char) miniBuffer[0] == 'b')
								&& ((char) miniBuffer[1] == 't')) {
							System.out.println("extending hand");
							return;
						}
					}

				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

	}

	private static class SerialWriter implements Runnable {
		OutputStream out;

		public SerialWriter(OutputStream out) {
			this.out = out;
		}

		public void run() {
			while (true) {

				try {

					System.out.print("n");
					this.out.write('n');
					this.out.write('n');
					this.out.write('n');
					this.out.write('n');
				} catch (IOException e1) {
					e1.printStackTrace();
				}

				try {
					Thread.sleep(100);
				} catch (InterruptedException e) {
					return;
				}
			}
		}
	}

}
