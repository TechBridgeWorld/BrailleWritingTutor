package edu.cmu.cs239.emulator.model;

import java.util.HashMap;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

public class ActionHandler {
	
	private final int timeOut = 10000;
	private HashMap<Button, Boolean> down;
	
	public ActionHandler(){
		down = new HashMap<Button, Boolean>();
	}
	
	public void handleRequest(HttpServletRequest request, HttpServletResponse response){
		String query = request.getQueryString();
		if(query == null || query.length()==0){
			//error
		}
		String buttonName = request.getParameter("button");
		
		//do something with the buttonName
		System.out.println(buttonName);
	}

}
