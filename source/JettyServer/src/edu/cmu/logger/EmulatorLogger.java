package edu.cmu.logger;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * A class that provides three different loggers for the server or the emulator.  
 * @author ziw
 *
 */
public class EmulatorLogger {

	private static final String SERVER_LOGGER_NAME = "edu.cmu.server";
	private static final String EMULATOR_DEBUG_LOGGER = "edu.cmu.emulator.debug";
	private static final String EMULATOR_ERROR_LOGGER = "edu.cmu.emulator.error";
	
	
	public static Logger getServerLogger(){
		return LoggerFactory.getLogger(SERVER_LOGGER_NAME);
	}
	
	public static Logger getEmulatorDebugLogger(){
		return LoggerFactory.getLogger(EMULATOR_DEBUG_LOGGER);
	}
	
	public static Logger getEmulatorErrorLogger(){
		return LoggerFactory.getLogger(EMULATOR_ERROR_LOGGER);
	}
	
}
