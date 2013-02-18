package edu.cmu.ui;

import edu.cmu.server.JettyServer;

/**
 * This action handler handles actions invoked from 
 * the ControllerUI and talks to the JettyServer to perform
 * various actions. <br>
 * It also exposes server status and ports to the UI.
 * @author ziw
 *
 */
public class UIActionHandler {

	private JettyServer js;
	
	public UIActionHandler(){
		js = new JettyServer();
	}
	
	public void startServer(){
		js.startServer();
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

	public boolean isServerRunning(){
		return getServerStatus() == JettyServer.RUNNING;
	}
	
	public int getPort() {
		return js.getPort();
	}

}