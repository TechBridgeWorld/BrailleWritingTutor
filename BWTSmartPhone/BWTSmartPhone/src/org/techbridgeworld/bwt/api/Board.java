package org.techbridgeworld.bwt.api;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Locale;

import org.techbridgeworld.bwt.api.libs.Braille;

import android.util.Log;

/**
 * Board.java is in charge of keeping track of the user's input and is updated
 * by BWT.java
 * 
 * @author Jessica and Salem
 *
 */
public class Board {
	private static Braille braille = new Braille();
	
	private final Cell board[];
	private boolean altFlag = false;
	
	//Keeps track of input history from user (can be cleared)
	private LinkedList<Integer> inputInfo;
	
	//a Cell that contains input from any of the Braille cells
	private Cell universalCell;
	
	//index of the last active cell 
	private Integer currCellInd;

	/**
	 * Constructor. Initializes all the cells to 0, and sets 
	 * the current cell to -1.
	 */
	public Board() {
		Cell[] temp = new Cell[33];

		for (int i = 0; i < 33; i++) {
			temp[i] = new Cell();
		}
		this.universalCell = new Cell();
		this.board = temp;
		this.inputInfo = new LinkedList<Integer>();
		this.currCellInd = -1;
	}

	/**
	 * Gets the alt flag
	 */
	public boolean getAltFlag() {
		return this.altFlag;
	}

	/**
	 * Sets the alt flag to be as specified
	 * @param altFlag
	 */
	public void setAltFlag(boolean altFlag) {
		this.altFlag = altFlag;
	}

	/**
	 * Clears the alt flag to false
	 */
	public void clearAltFlag() {
		setAltFlag(false);
	}

	/**
	 * Converts a raw braille representation to a glyph.
	 * 
	 * @param i = raw int
	 * @return converted glyph from i.
	 */
	public char getBraille(int i) {
		return braille.get(i);
	}

	/**
	 * Converts a character to a raw braille representation.
	 * 
	 * @param c	= character to convert
	 * @return raw braille representation of c.
	 */
	public int getBraille(char c) {
		return braille.get(c);
	}

	/**
	 * Clears the board of all values.
	 */
	public void clearBoard() {
		for (final Cell c : board) {
			c.setValue(0);
		}
		this.universalCell.setValue(0);
		this.altFlag = false;
		this.inputInfo.clear();
		this.currCellInd = -1;
	}

	/**
	 * Getter method for the currently active cell.
	 * 
	 * @return currently active cell index.
	 */
	public Integer getCurrCellInd() {
		return currCellInd;
	}

	/**
	 * Called when SubmitEvent is successful, to show already submitted
	 */
	public void resetCurrCellInd() {
		currCellInd = -1;
	}
	
	/**
	 * Getter method for the last cell with inputed char.
	 * Different from currCellInd, in that it is only -1 if empty
	 * Will not be -1 on push of AltBtn or Submitting
	 * @return last inputed cell index.
	 */
	public Integer getLastInputInfoInd() {
		if(inputInfo.isEmpty()) return -1;
		return inputInfo.getLast();
	}
	/**
	 * Getter method for the number of used cells.
	 * 
	 * @return the number of used cells in the board at the time.
	 */
	public int getUsedCells() {
		return this.inputInfo.size();
	}

	/**
	 * Returns the braille code at the given cell index.
	 * 
	 * @param cellInd = cell index.
	 * @return braille code at board[cellInd].
	 */
	public int getBitsAtCell(int cellInd) {
		return board[cellInd].getBrailleCode();
	}
	
	/**
	 * Return bit representation of any pushed dots, regardless of cell
	 * @return
	 */
	public int getBitsAtUnivCell() {
		return this.universalCell.getBrailleCode();
	}
	
	/**
	 * Returns the glyph representation stored at the given cell index.
	 * 
	 * @param cellInd = cell index.
	 * @return glyph at board[cellInd].
	 */
	public char getGlyphAtCell(int cellInd) {
		return board[cellInd].getGlyph();
	}
	
	/**
	 * Return glyph representation of any pushed dots, regardless of cell
	 * @return
	 */
	public int getGlyphAtUnivCell() {
		return this.universalCell.getGlyph();
	}

	/**
	 * Sets a cell to a given value
	 * 
	 * @param cellInd = cell index.
	 * @param value = value to store in cell.
	 */
	public void setBitsAtCell(int cellInd, int value) {
		board[cellInd].setValue(value);
	}
	
	/**
	 * Sets the universal cell to a certain Braille Code
	 * @param value
	 */
	public void setBitsAtUnivCell(int value) {
		this.universalCell.setValue(0);
	}
	
	/**
	 * Print input info for debugging purposes
	 */
	public void printInputInfo() {
		int tmpInd = 0;
		StringBuffer buf = new StringBuffer();
		while(tmpInd < inputInfo.size()) {
			buf.append(inputInfo.get(tmpInd));
			tmpInd++;
		}
		Log.d("Check input", "inputInfo: '" + buf.toString() + "'");
	}
	
	/**
	 * Clears the touched cell's state on the board and empties
	 * inputInfo.
	 */
	public void clearTouchedCells() {
		while(!inputInfo.isEmpty()) {
			board[inputInfo.poll()].setValue(0);
		}
		this.universalCell.setValue(0);
		this.currCellInd = -1;
	}
	
	/**
	 * Gets the first cellInd accessed in inputInfo, and
	 * returns the trimmed String that includes glyphs from every
	 * cell ind that follows. 
	 * @return
	 */
	public String viewAsIndexed() {
		int index = inputInfo.getFirst();
		int end = inputInfo.getLast();
		StringBuffer buf = new StringBuffer();
		while(index <= end) {
			buf.append(getGlyphAtCell(index));
			index++;
		}
		return buf.toString();
	}

	/**
	 * Gets the first cellInd accessed in inputInfo, and
	 * returns the trimmed String that includes glyphs from every
	 * cell ind that follows. Then clears inputInfo
	 * @return
	 */
	public String viewAndEmptyAsIndexed() {
		int index = inputInfo.getFirst();
		int end = inputInfo.getLast();
		StringBuffer buf = new StringBuffer();
		while(index <= end) {
			buf.append(getGlyphAtCell(index));
			setBitsAtCell(index, 0);
			index++;
		}
		//clear buffer and board's cells within range
		inputInfo.clear();
		this.currCellInd = -1;
		return buf.toString();
	}
	
	/**
	 * Inputted refers to just the cells the user has touched
	 * ie: Won't include spaces in returned String
	 * @return
	 */
	public String viewAsInputted() {
		int tmpInd = 0;
		StringBuffer buf = new StringBuffer();
		while(tmpInd != inputInfo.size()) {
			int cellInd = inputInfo.get(tmpInd);
			buf.append(this.getGlyphAtCell(cellInd));
			tmpInd++;
		}
		return buf.toString();
		
	}
	
	/**
	 * Empties the inputInfo AND clears the cells touched
	 * @return
	 */
	public String viewAndEmptyAsInputted() {
		StringBuffer buf = new StringBuffer();
		while(!inputInfo.isEmpty()) {
			int cellInd = inputInfo.poll();
			buf.append(this.getGlyphAtCell(cellInd));
			
			//clear cells touched
			setBitsAtCell(cellInd, 0);
		}
		this.currCellInd = -1;
		return buf.toString();
	}

	/**
	 * Inputted refers to just the cells the user has touched
	 * ie: Won't include spaces in returned String
	 * EXCEPT for the current cell
	 * @return
	 */
	public String viewAsInputtedExceptCurrent() {
		int tmpInd = 0;
		StringBuffer buf = new StringBuffer();
		while(tmpInd < inputInfo.size() - 1) {
			int cellInd = inputInfo.get(tmpInd);
			buf.append(this.getGlyphAtCell(cellInd));
			printInputInfo();
			Log.i("Check input", "viewAsInputtedExceptCurrent: " + buf.toString());
			tmpInd++;
		}
		return buf.toString();
		
	}
	
	/**
	 * Empties the inputInfo AND clears the cells touched
	 * EXCEPT for the current cell
	 * @return
	 */
	public String viewAndEmptyAsInputtedExceptCurrent() {
		StringBuffer buf = new StringBuffer();
		while(inputInfo.size() > 1) {
			int cellInd = inputInfo.poll();
			buf.append(this.getGlyphAtCell(cellInd));
			
			//clear cells touched
			setBitsAtCell(cellInd, 0);
		}
		return buf.toString();
	}
	
	/**
	 * View the inputInfo as an array of 6 bits
	 * @return
	 */
	public ArrayList<Integer> viewBitsAtInputtedCells() {
		ArrayList<Integer> bits = new ArrayList<Integer>();
		for(Integer cellInd : inputInfo) {
			bits.add(board[cellInd].getBrailleCode());
		}
		return bits;
	}

	/**
	 * View and empty the input info as array of 6 bits
	 * @return
	 */
	public ArrayList<Integer> viewAndEmptyBitsAtInputtedCells() {
		ArrayList<Integer> bits = new ArrayList<Integer>();
		while(!inputInfo.isEmpty()) {
			int cellInd = inputInfo.remove();
			bits.add(board[cellInd].getBrailleCode());
			board[cellInd].setValue(0);
		}
		this.currCellInd = -1;
		return bits;
	}
	
	/**
	 * View the inputInfo as an array of 6 bits
	 * EXCEPT for current cell
	 * @return
	 */
	public ArrayList<Integer> viewBitsAtInputtedCellsExceptCurrent() {
		ArrayList<Integer> bits = new ArrayList<Integer>();
		for(Integer cellInd : inputInfo) {
			if(cellInd >= inputInfo.size() - 1) break;
			bits.add(board[cellInd].getBrailleCode());
		}
		return bits;
	}

	/**
	 * View and empty the input info as array of 6 bits
	 * EXCEPT for current cell
	 * @return
	 */
	public ArrayList<Integer> viewAndEmptyBitsAtInputtedCellsExceptCurrent() {
		ArrayList<Integer> bits = new ArrayList<Integer>();
		while(inputInfo.size() > 1) {
			int cellInd = inputInfo.remove();
			bits.add(board[cellInd].getBrailleCode());
			board[cellInd].setValue(0);
		}
		return bits;
	}
	
	/**
	 * Allows user to delete the last cell they've touched
	 */
	public void backspaceByInput() {
		int lastCell = inputInfo.pop();
		setBitsAtCell(lastCell, 0);
	}
	
	/**
	 * Update board based on given cellInd and buttonInd
	 * Triggers changeCellEvent if necessary
	 * 
	 * @param cellInd - Correlates with cellInd from Firmware
	 * @param buttonNum - ranges from 1 to 6
	 * @return the 6-bits of what's raised in cellInd
	 */
	public int handleNewInput(int cellInd, int buttonNum) {
		if(cellInd == -1) {
			altFlag = true;
			return -1;
		}
		
		if(cellInd != 0) {
			// Update cell to reference the correct cell.
			// Cell 1 should be the first one to write in, i.e. the top right
			// cell. (Writing right to left)
			cellInd = ((32-cellInd) + 16) %32 + 1;
		}
		
		int prevCellInd = inputInfo.peekLast();
		if(prevCellInd != cellInd) {
			inputInfo.push(cellInd);
		}
		
		this.currCellInd = cellInd;
		return board[cellInd].setDot(buttonNum);
	}

	/**
	 * Updates the board based on a string from the BWT.
	 * 
	 * @param
	 */
	public void handleNewInput(String message) {
		// If an alt button has been pressed, set the altFlag to true.
		if (message.equals("a")) {
			this.currCellInd = -1;
			altFlag = true;
		}
		
		// If it's a button, update the appropriate dot in cell 0 (i.e. the
		// button cell).
		else if ("bcdefg".indexOf(message) != -1) {
			this.currCellInd = 0;
			// If the button cells haven't been touched yet, add to inputInfo
			if (this.board[0].getBrailleCode() == 0) {
				inputInfo.add(0);
			}
			int dotNum = 0;
			
			switch (message.toLowerCase(Locale.getDefault()).charAt(0)) {
			case 'b':
				dotNum = 4;
				break;
			case 'c':
				dotNum = 5;
				break;
			case 'd':
				dotNum = 6;
				break;
			case 'e':
				dotNum = 1;
				break;
			case 'f':
				dotNum = 2;
				break;
			case 'g':
				dotNum = 3;
				break;
			default:
				// This should never ever fire.
				Log.e("Salem", "Received unhandled button event '" + message
						+ "'");
			}
			
			this.board[0].setDot(dotNum, true);
			this.universalCell.setDot(dotNum, true);
		}

		// It's two integers, one for the cell and one for the dot.
		else {
			String[] details = message.split(" ");

			int cell = Integer.parseInt(details[0].trim());
			int dot =  Integer.parseInt(details[1].trim());
			
			// Update cell to reference the correct cell.
			// Cell 1 should be the first one to write in, i.e. the top right
			// cell. (Writing right to left)
			cell = ((32-cell) + 16) %32 + 1;
			dot = (dot +2) %6 + 1;
			
			this.currCellInd = cell;
			// If the selected cell had nothing in it, push to inputInfo.
			if (board[cell].getBrailleCode() == 0) {
				inputInfo.add(cell);
			}
			this.board[cell].setDot(dot, true);
			this.universalCell.setDot(dot, true);
		}
	}
}
