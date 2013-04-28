package edu.cmu.controller;

import org.slf4j.Logger;

import edu.cmu.logger.EmulatorLogger;
import java.io.*;
import java.net.*;


/**
 * Action handler for Linux OS
 * @author Nikhil
 *
 */
public class LinuxActionHandler extends AbstractActionHandler {
  private Socket com;
  private final String host = "localhost";
  private final int port = 8081;

  private boolean handshaking;

  private Logger logger = EmulatorLogger.getEmulatorInfoLogger();
  private Logger debugLogger = EmulatorLogger.getEmulatorDebugLogger();

  private byte[] writeBuffer;
  private int nWrite;
  //private byte[] readBuffer;
  //private int nRead;

  private Thread writer;
  //private Thread reader;

  public LinuxActionHandler() {
    // Do any initialization here.
    // constructor is called as soon as the server starts
    handshaking = false;
    writer = new Thread(new SerialWriter());
    writer.start();
  }

  @Override
  public void initSerialComm() throws Exception {
    if (com != null){
      //already initialized.
      return;
    }
    logger.info("Initializing socket connection to USBemulator.py");
    com = new Socket(host, port);
    logger.info("serial port connection initialized");
  }

  @Override
  public void handleButtonCode(String buttonCode) {
    if (buttonCode == null || buttonCode.trim().length() == 0) {
      logger.warn("Button code is empty.Illegal button code.");
      return;
    }
    if (com == null) {
      logger.error("COM port is not initialized. Can't handle button press.");
      return;
    }

    if (buttonCode.equals("init")) {
      handShake();
      return;
    }

    String decodedName = null;
    try {
      decodedName = URLDecoder.decode(buttonCode, "UTF-8");
      logger.info("Decoded button code: " + decodedName);

      //TODO is this a bad idea?  Should write buffer be one fixed buffer
      //     that gets re-written to every time.
      writeBuffer = decodedName.getBytes();
      nWrite = decodedName.getBytes().length;
      writer.interrupt();

      debugLogger.debug("Button code send: " + buttonCode);
      debugLogger.debug("Decoded code    : " + decodedName);

    } catch (UnsupportedEncodingException e) {
      logger.error("Unable to decode the button code: " + buttonCode);
      EmulatorLogger.logException(logger, e);
    }
  }

  private void handShake() {
    // tries to handshake. Reject handshaking if another handshake is in progress.
    int success = 1;
    if(!handshaking){
      logger.info("Init caught. Start sending n's");
      logger.info("Server is blocking any further handshake until this one is done.");
      try {
        com.getOutputStream().write("_init".getBytes());
        success = com.getInputStream().read();
      } catch (IOException e) {
        logger.error("IOException when communicating with serial port emulator");
        EmulatorLogger.logException(logger, e);
      }
      finally{
        handshaking = false;// handshake done either way.
        if(success == 0){
          logger.warn("Error occurred while handhsaking. Try again");
        }
        logger.info("Server is open to accept handshake again.");
      }
    } else {
      // another handshaking in process.
      logger.info("Init request caught " +
          "but another handshaking in being processed. Ignore this handshake.");
    }
  }

  private class SerialWriter implements Runnable{
    @Override
    public void run() {
      while(true){
        try {
          Thread.sleep(10000);
        } catch (InterruptedException e) {
          try {
            com.getOutputStream().write(writeBuffer, 0, nWrite);
          } catch (IOException e1) {
            logger.error("IOException when writing to serial port emulator");
            EmulatorLogger.logException(logger, e);
          }
        }
      }
    }
  }
}
