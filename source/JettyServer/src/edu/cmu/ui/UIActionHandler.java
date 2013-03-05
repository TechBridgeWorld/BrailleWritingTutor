package edu.cmu.ui;


import edu.cmu.controller.AbstractActionHandler;
import edu.cmu.controller.GenericServlet;
import edu.cmu.controller.WindowsActionHandler;
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
	private AbstractActionHandler handler;
	private GenericServlet servlet;
	

	
	public UIActionHandler() throws Exception{
		handler = new WindowsActionHandler();
		servlet = new GenericServlet(handler);
		js = new JettyServer(servlet);
	}
	
	public UIActionHandler(int port) throws Exception{
		handler = new WindowsActionHandler();
		servlet = new GenericServlet(handler);
		js = new JettyServer(port,servlet);
	}
	
	public void startServer() throws Exception{
		js.startServer();
	}
	
	public void stopServer() throws Exception{
		js.stopServer();
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