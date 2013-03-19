package edu.cmu.ui;


import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import org.apache.commons.lang3.SystemUtils;

import edu.cmu.controller.AbstractActionHandler;
import edu.cmu.controller.GenericServlet;
import edu.cmu.controller.LinuxActionHandler;
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
public class UIActionHandler implements ActionListener{

	private JettyServer js;
	private AbstractActionHandler handler;
	private GenericServlet servlet;
	

	
	public UIActionHandler() throws Exception{
		this(JettyServer.DEFAULT_PORT);
	}
	
	public UIActionHandler(int port) throws Exception{
		if(SystemUtils.IS_OS_WINDOWS){
			handler = new WindowsActionHandler();
		}
		else if(SystemUtils.IS_OS_LINUX){
			handler = new LinuxActionHandler();
		}
		else{
//			throw new Exception("Invalid OS. Only support Windows/Linux.");
			//TODO  this line is here for debug/dev only because dev environment is Mac OS 
			//Should throw the exception when shipped.
			handler = new WindowsActionHandler();
		}
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

	@Override
	public void actionPerformed(ActionEvent e) {
		// TODO Auto-generated method stub
		
	}
	
}