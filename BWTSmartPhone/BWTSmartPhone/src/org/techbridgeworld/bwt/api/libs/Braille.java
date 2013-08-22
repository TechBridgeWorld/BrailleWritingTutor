package org.techbridgeworld.bwt.api.libs;

import java.util.Hashtable;

/**
 * Braill.java contains mappings from a character to a binary number that
 * represents dots in a Braille cell, and vice versa
 * 
 * @author Salem
 *
 */
public class Braille {
	
	private static final Hashtable<Character, Integer> CharToBraille = new Hashtable<Character, Integer>(){
		
		private static final long serialVersionUID = 1L;

	{
		put(' ', bin("00000000"));
	    put('a', bin("00000001"));
	    put('b', bin("00000011"));
	    put('c', bin("00001001"));
	    put('d', bin("00011001"));
	    put('e', bin("00010001"));
	    put('f', bin("00001011"));
	    put('g', bin("00011011"));
	    put('h', bin("00010011"));
	    put('i', bin("00001010"));
	    put('j', bin("00011010"));
	    put('k', bin("00000101"));
	    put('l', bin("00000111"));
	    put('m', bin("00001101"));
	    put('n', bin("00011101"));
	    put('o', bin("00010101"));
	    put('p', bin("00001111"));
	    put('q', bin("00011111"));
	    put('r', bin("00010111"));
	    put('s', bin("00001110"));
	    put('t', bin("00011110"));
	    put('u', bin("00100101"));
	    put('v', bin("00100111"));
	    put('w', bin("00111010"));
	    put('x', bin("00101101"));
	    put('y', bin("00111101"));
	    put('z', bin("00110101"));
	}};
	
	private static final Hashtable<Integer, Character> BrailleToChar = new Hashtable<Integer, Character>(){
		
		private static final long serialVersionUID = 1L;

	{
		put(bin("00000000"), ' ');
	    put(bin("00000001"), 'a');
	    put(bin("00000011"), 'b');
	    put(bin("00001001"), 'c');
	    put(bin("00011001"), 'd');
	    put(bin("00010001"), 'e');
	    put(bin("00001011"), 'f');
	    put(bin("00011011"), 'g');
	    put(bin("00010011"), 'h');
	    put(bin("00001010"), 'i');
	    put(bin("00011010"), 'j');
	    put(bin("00000101"), 'k');
	    put(bin("00000111"), 'l');
	    put(bin("00001101"), 'm');
	    put(bin("00011101"), 'n');
	    put(bin("00010101"), 'o');
	    put(bin("00001111"), 'p');
	    put(bin("00011111"), 'q');
	    put(bin("00010111"), 'r');
	    put(bin("00001110"), 's');
	    put(bin("00011110"), 't');
	    put(bin("00100101"), 'u');
	    put(bin("00100111"), 'v');
	    put(bin("00111010"), 'w');
	    put(bin("00101101"), 'x');
	    put(bin("00111101"), 'y');
	    put(bin("00110101"), 'z');
	}};
	
	/**
	 * Returns the braille representation of a given character
	 * @param character
	 * @return braille integer representation of the character
	 */
	public Integer get(char c){
		Integer query = CharToBraille.get(c);
		return (query == null? 0 : query);
	}
	
	/**	
	 * Returns the character interpretation of a braille integer code
	 * @param integer
	 * @return braille integer code
	 */
	public Character get(int i){
		Character query = BrailleToChar.get(i);
		return (query == null? /*'\u0000'*/ '-' : query);
	}
	
	/**	
	 * Takes a binary string and returns an integer value.
	 * Used as a helper function to construct the hashtables.
	 * @param String representation of binary
	 * @return base-10 integer representation of given binary
	 */
	private static int bin(String binary){
		return Integer.parseInt(binary, 2);
	}
	
	
	
}
