package edu.cmu.server;
import java.net.URL;

import javax.servlet.http.HttpServlet;

import org.eclipse.jetty.server.Handler;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.handler.HandlerList;
import org.eclipse.jetty.server.handler.ResourceHandler;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import edu.cmu.controller.GenericServlet;

/**
 * A wrapper of an embedded Jetty 9.0 server.
 * @author ziw
 *
 */
public class JettyServer {
	
	private Server server;
	private HttpServlet ajaxServlet;
	private static final int DEFAULT_PORT = 8887;
	private int port;
	
	public static final int READY = 0;
	public static final int RUNNING = 1;
	public static final int OFFLINE = 2;
	public static final int ERROR = -1;
	
		
	public JettyServer(HttpServlet servlet){
		this(DEFAULT_PORT, servlet);
	}
	
	public JettyServer(int port, HttpServlet servlet){
		this.ajaxServlet = servlet;
		this.port =port>0? port : DEFAULT_PORT;
		server = new Server(this.port);
		initServerHandler();
	}
	
	public void startServer() throws Exception{
		server.start();
	}
	
		
	public void restartServer(){
		//TODO
	}
	
	public void stopServer() throws Exception{
		server.stop();
	}
	
	private boolean isStarted(){
		return server.isStarted();
	}
	
	private boolean isStopped(){
		return server.isStopped();
	}
	
	public int getServerStatus(){
		if(isStarted())
			return RUNNING;
		if(isStopped())
			return OFFLINE;
		return ERROR;
	}
	
	/**
	 * Get the port on which the server is running.
	 * @return the port number if the server is running. -1 otherwise.
	 */
	public int getPort(){
		return getServerStatus()==RUNNING? port : -1;
	}
	
	private void initServerHandler(){
		HandlerList handlers = new HandlerList();
		//handler for static files
		ResourceHandler resource_handler = new ResourceHandler();
        resource_handler.setDirectoriesListed(false);
        URL htmlPath = JettyServer.class.getClassLoader().getResource("html");
        if(htmlPath == null){
        	//TODO html folder not found...
        }
        resource_handler.setResourceBase(htmlPath.toExternalForm());
        
        //handler for ajax request
        ServletContextHandler context = new ServletContextHandler(ServletContextHandler.SESSIONS);
        context.setContextPath("/");
        context.addServlet(new ServletHolder(this.ajaxServlet),"/");
        
        //add both handlers
        handlers.setHandlers(new Handler[] { resource_handler, context});
        
        server.setHandler(handlers);
	}

}
