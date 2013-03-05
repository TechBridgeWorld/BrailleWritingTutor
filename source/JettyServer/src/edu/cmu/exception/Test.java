package edu.cmu.exception;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import ch.qos.logback.classic.LoggerContext;
import ch.qos.logback.core.util.StatusPrinter;
import edu.cmu.logger.EmulatorLogger;

public class Test {

	
	public static void main(String[] args){
		Logger l1 = EmulatorLogger.getServerLogger();
		l1.info("This is the server log");
		
		Logger l2 = EmulatorLogger.getEmulatorDebugLogger();
		l2.debug("This is the emulator debug log");
		l2.error("This is the server lo errrrg");
		l2.trace("Tracing........");

		
		Logger l3 = EmulatorLogger.getEmulatorErrorLogger();
		l3.error("This is the emulator error log");
	}
}
