package org.techbridgeworld.bwt.api.events;

import javaEventing.EventObject;

import org.techbridgeworld.bwt.api.Board;

/**
 * CellsEvent is triggered whenever one of the bottom, smaller 32 cells
 * is touched
 * 
 * @author Salem
 *
 */
public class CellsEvent extends EventObject {
	
	private String message; // Raw button details 
	private int cell;		// The number of the cell
	private int dot;		// The number of the dot
	
	public CellsEvent(String message, Board board){
		this.message = message;
		
		String[] details = message.split(" ");
		
		int msgCell = Integer.parseInt(details[0].trim());
		this.cell = ((32-msgCell) + 16) %32 + 1;
		int msgDot =  Integer.parseInt(details[1].trim());
		this.dot = ((msgDot + 2) % 6) + 1;
		
	}
	
	// Getters
	public String getMessage(){
		return message;
	}
	public int getCell(){
		return cell;
	}
	public int getDot(){
		return dot;
	}

}
