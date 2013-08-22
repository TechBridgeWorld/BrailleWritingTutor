package org.techbridgeworld.bwt.api.events;

import javaEventing.EventObject;

/**
 * Submit Event is triggered in any of the three cases:
 * 		- The AltBtn was pressed after a non-AltBtn
 * 		- The cell was changed from a non-AltBtn to a non-AltBtn
 * 		- The INACTIVE_TIME timer ran out and last active cell was non-AltBtn
 * 
 * @author Jessica
 *
 */
public class SubmitEvent extends EventObject{

	int cell;
	int cellBits;
	int type;
	
	public SubmitEvent(int cell, int cellBits, int type){
		this.cell = cell;
		this.cellBits = cellBits;
		this.type = type;
	}
	
	/**
	 * Get the index of the cell submitted
	 * @return
	 */
	public int getCellInd() {
		return cell;
	}
	
	/**
	 * Get bits at the submitted cells
	 * @return
	 */
	public int getCellBits() {
		return cellBits;
	}
	
	/**
	 * Get the type of submission
	 * (change cell, alt button, or inactivity timer)
	 * 
	 * @return
	 */
	public int getType() {
		return type;
	}
}

