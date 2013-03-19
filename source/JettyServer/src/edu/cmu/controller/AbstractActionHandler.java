package edu.cmu.controller;

public abstract class AbstractActionHandler {

	/**
	 * Initialization. This method is called each time a new emulator is launched.
	 * This is not the handshake, but rather what should be done when the emulator
	 * displays the loading screen.   
	 * @throws Exception Any exception that occurred during initialization. Once the 
	 * exception is thrown, the emulator would catch an error and notify the user 
	 * in the front-end. 
	 */
	public abstract void initSerialComm() throws Exception;

	
	/**
	 * Handle the buttonCode sent from http request.
	 * @param buttonCode
	 */
	public abstract void handleButtonCode(String buttonCode);

	
}
