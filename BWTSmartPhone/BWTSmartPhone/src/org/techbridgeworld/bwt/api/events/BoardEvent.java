package org.techbridgeworld.bwt.api.events;

import javaEventing.EventObject;

/**
 * BoardEvent is triggered on any input from the user to the board.
 * 
 * @author Salem
 *
 */
public class BoardEvent extends EventObject{

	String message; // The raw message
	int cell;		// The relevant cell
	int cellState;	// The state of the cell
	int dot;		// The relevant dot in the cell
	
	public BoardEvent(String message, int cell, int cellState, int dot){
		this.message = message;
		this.cell = cell;
		this.cellState = cellState;
		this.dot = dot;
	}
	
	// Getters
	public String getMessage(){
		return message;
	}
	
	public int getCellInd() {
		return cell;
	}
	
	public int getCellState() {
		return cellState;
	}
	
	public int getDot() {
		return dot;
	}
}
