package edu.cmu.cs239.emulator.util;

public class Util {
	
	public static boolean isButtonAction(String url){
		if(url == null)
			return false;
		return url.endsWith(".do");
	}
	
	
}
