package edu.cmu.ui;

import edu.cmu.server.JettyServer;

public class UIActionHandler {

	private JettyServer js;
	
	public UIActionHandler(){
		js = new JettyServer();
	}
	
	public void startServer(){
		js.startServer();
		System.out.println("Starting server..");
	}
	
	public void stopServer(){
		js.stopServer();
	}
	
	public void restartServer(){
		js.restartServer();
	}
	
	public int getServerStatus(){
		return js.getServerStatus();
	}

	public int getPort() {
		return js.getPort();
	}

}