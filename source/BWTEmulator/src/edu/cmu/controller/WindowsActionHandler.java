package edu.cmu.controller;

import java.io.IOException;
import java.net.URLDecoder;
import java.util.concurrent.ExecutionException;

import org.slf4j.Logger;

import edu.cmu.controller.TwoWaySerialComm;
import edu.cmu.logger.EmulatorLogger;

/**
 * A handler class that handles connection with the serial COM port.
 *
 * @author Nikhil
 *
 */
public class WindowsActionHandler extends AbstractActionHandler{

  private TwoWaySerialComm com;

  private static final String COM_PORT_NAME = "COM7";

  //a flag that marks if the emulator is trying a handshake.
  private boolean handshaking;

  private Logger logger = EmulatorLogger.getEmulatorInfoLogger();
  private Logger debugLogger = EmulatorLogger.getEmulatorDebugLogger();

  private byte[] writeBuffer;
  private int nWrite;

  private Thread writer;

  public WindowsActionHandler() {
    // Do any initialization here.
    // constructor is called as soon as the server starts
    handshaking = false;
    writer = new Thread(new SerialWriter());
    writer.start();
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

      writeBuffer = decodedName.getBytes();
      nWrite = decodedName.getBytes().length;
      writer.interrupt();

      debugLogger.debug("Button code send: " + buttonCode);
      debugLogger.debug("Decoded code    : " + decodedName);
    } catch (IOException e) {
      logger.error("Unable to decode the button code: " + buttonCode);
      EmulatorLogger.logException(logger, e);
    }
  }

  private void handShake() {
    // tries to handshake. Reject handshaking if another handshake is in progress.
    if (!handshaking) {
      try {
        handshaking = true;
        logger.info("Init caught. Start sending n's");
        logger.info("Server is blocking any further handshake until this one is done.");
        initialize();
      } catch (Exception e) {
        logger.error("Error occured when handshaking. ");
        EmulatorLogger.logException(logger, e);
        handshaking = false;
      } 
    } else {
      // another handshaking in process.
      logger.info("Init request caught " +
          "but another handshaking in being processed. Ignore this handshake.");
    }
  }

  @Override
  public void initSerialComm() throws Exception {
    if (com != null){
      //already initialized.
      return;
    }
    logger.info("Initializing TwoWaySerialComm");
    com = new TwoWaySerialComm(COM_PORT_NAME);// this line takes a while on some computers
    logger.info("TwoWaySerialComm initialized");
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
    Thread reader = new Thread(new SerialReader());
    reader.start();
    writeBuffer = "n".getBytes();
    nWrite = 1;
    while(reader.isAlive()){
      writer.interrupt();
      Thread.sleep(100);
    }
    writeBuffer="bt".getBytes();
    nWrite = 2;
    writer.interrupt();
    handshaking = false;
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

  private class SerialReader implements Runnable{
    @Override
    public void run() {
      byte[] buffer = new byte[1024];
      byte[] miniBuffer = new byte[2]; // buffer for the incoming bytes
                        // waiting for a b followed by a
                        // t
      int len = -1;
      boolean done = false;
      try {
        while (((len = com.getInputStream().read(buffer)) > -1) && (done == false)) {
          for (int i = 0; i < len; i++) {
            miniBuffer[0] = miniBuffer[1];
            miniBuffer[1] = buffer[i];
            if (((char) miniBuffer[0] == 'b')
                && ((char) miniBuffer[1] == 't')) {
              return;
            }
          }

        }
      } catch (IOException e) {
        logger.error("IOException when reading from serial port emulator");
        EmulatorLogger.logException(logger, e);
      }
    }
  }
}
