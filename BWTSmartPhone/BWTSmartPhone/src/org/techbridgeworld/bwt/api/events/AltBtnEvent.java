package org.techbridgeworld.bwt.api.events;

import javaEventing.EventObject;

/**
 * AltBtn is triggered when either of the alt buttons are pushed
 * @author Salem
 *
 */
public class AltBtnEvent extends EventObject {
	
	private String message; // Always "a", here for consistancy's sake.
	
	public AltBtnEvent(String message){
		this.message = message;
	}
	
	// Getters
	public String getMessage(){
		return message;
	}
	
}
