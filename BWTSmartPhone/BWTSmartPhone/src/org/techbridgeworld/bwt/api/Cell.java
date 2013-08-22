package org.techbridgeworld.bwt.api;

import org.techbridgeworld.bwt.api.libs.Braille;

/**
 * Cell.java keeps track of the dots on an individual Braille cell
 * 
 * @author Jessica and Salem
 *
 */
public class Cell {
	
	// Valid braille codes (dots one through six) 
	public static final int ONE 	= 0x01;	//top right button
	public static final int TWO 	= 0x02;
	public static final int THREE 	= 0x04;
	public static final int FOUR 	= 0x08;
	public static final int FIVE 	= 0x10;
	public static final int SIX 	= 0x20;	//bottom left button

	/*
	 * The braille code for this cell is the result of bitwise OR
	 * on the dot values.
	 * 
	 * So, if dots 1, 2, and 5 are pressed, we represent the resulting
	 * cell as 010011, or 0x13
	 * 
	 * We keep a static instance of the Braille library to do 
	 * cell-to-glyph conversions.
	 */
	private int brailleCode;
	
	//Library that contains mapping from alphabet/glyph to int
	private static Braille braille = new Braille();
	
	//Constructors
	public Cell(){
		this.brailleCode = 0;
	}
	public Cell(int brailleCode){
		this.brailleCode = brailleCode;
	}
	

	/** 
	 * Checks to see if a dot is marked.
	 * 
	 * i.e. if brailleCode represents 'a':
	 *		checkDot(1) == true;
	 * 		checkDot(4) == false;
	 */
	public boolean checkDot(int dot){
		return ((brailleCode >> (dot - 1)) & 0x01) == 1;
	}
	
	
	/**
	 * Sets a given dot to true.
	 * Returns the new 6-bit for the cell
	 * @param dot
	 */
	public int setDot(int dot){
		return setDot(dot, true);
	}
	
	/**
	 * Sets a given dot to the given value.
	 * Returns the new 6-bit for the cell
	 * @param dot
	 * @param value
	 */
	public int setDot(int dot, boolean value){
		// Clear the value
		brailleCode = (~(1 << (dot - 1)) & brailleCode);
		
		// Replace the value with new value.
		int val = value ? 1 : 0;
		return brailleCode = ((val << (dot - 1)) | brailleCode);
		
	}
	
	/**
	 *  Checks for equality between two cell classes.
	 */
	public boolean isEqual(Cell cell){
		return (cell.getBrailleCode() == this.brailleCode);
	}
	
	/**
	 *  Returns the raw Braille integer representation for the cell.
	 */
	public int getBrailleCode() {
		return brailleCode;
	}

	/**
	 * Given they push button 3, bit = 000100.
	 * OR's current brailleCode with brailleCode passed in
	 */
	public int pressDot(final int dot) {
		if (isValidDot(dot)) {
			brailleCode = brailleCode | dot;
		}
		return brailleCode;
	}

	/**
	 * Resets brailleCode to newValue passed in
	 */
	public void setValue(final int newValue) {
		brailleCode = newValue;
	}

	private boolean isValidDot(final int bit) {
		return 	ONE == bit ||TWO == bit || THREE == bit ||
				FOUR == bit || FIVE == bit || SIX == bit;
	}

	/**
	 * Return the character corresponding to this sequence of dots,
	 * according to the character map
	 * If the character is not present in the map, the return value is '-'
	 * 
	 * @return Character | '-'
	 */
	public Character getGlyph() {
		return braille.get(brailleCode);
	}

	/**
	 * Return true if this cell currently encodes a valid character
	 * 
	 * @return boolean true if this cell encodes a valid character
	 */
	public boolean isGlyph() {
		return getGlyph() != null;
	}

	/**
	 * Return true if this cell encodes the character passed as a parameter
	 * 
	 * @param targetChar
	 *            the character to check against
	 * @return boolean true if this cell is encoding the passed in character
	 */
	public boolean isGlyph(final Character targetChar) {
		return getGlyph() == targetChar;
	}
}
