package edu.cmu.exception;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class EmulatorException extends RuntimeException{

	/**
	 * 
	 */
	private static final long serialVersionUID = -6942535357235694896L;
	private String msg;
	
	public EmulatorException(){
		super();
	}
	
	public EmulatorException(String message){
		super();
		msg = message;
	}
	
	public EmulatorException(Exception e){
		Logger logger = LoggerFactory.getLogger(EmulatorException.class);
		logger.error("message in constructor",e);
	}
	
	
	
	
}
