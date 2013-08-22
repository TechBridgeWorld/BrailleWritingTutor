package edu.cmu.logger;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A class that provides three different loggers for the server and the emulator.
 * Logs are saved at PATH_TO_EMULATOR_JAR_FILE/BWT_LOGS
 * @author ziw
 *
 */
public class EmulatorLogger {

  private static final String SERVER_LOGGER_NAME = "edu.cmu.server";
  private static final String EMULATOR_DEBUG_LOGGER = "edu.cmu.emulator.debug";
  private static final String EMULATOR_INFO_LOGGER = "edu.cmu.emulator.info";
  private static final String EXCEPTION_MSG = "Exception occurred. Printing stack trace.";


  /**
   * Returns a logger that writes to ./BWT_LOGS/server.log.
   * It should be used to log any server-related issues.
   * @return
   */
  public static Logger getServerLogger(){
    return LoggerFactory.getLogger(SERVER_LOGGER_NAME);
  }

  /**
   * Returns a logger that writes to ./BWT_LOGS/emulator_debug.log.
   * This log is used only for client to debug when developing for the BWT.
   * Currently it only logs bytes sent to the hardware.
   * @return
   */
  public static Logger getEmulatorDebugLogger(){
    return LoggerFactory.getLogger(EMULATOR_DEBUG_LOGGER);
  }

  /**
   * Returns a logger that writes to ./BWT_LOGS/emulator_error.log. This log is used for any internal
   * errors(except for server errors) and warnings occurred when the emulator runs.
   * @return
   */
  public static Logger getEmulatorInfoLogger(){
    return LoggerFactory.getLogger(EMULATOR_INFO_LOGGER);
  }

  /**
   * Log an exception with the stack trace
   * @param logger
   */
  public static void logException(Logger logger, Exception e){
    if(logger == null){
      throw new NullPointerException("Can't log exception with null logger.");
    }
    logger.error(EXCEPTION_MSG, e);
  }
}
