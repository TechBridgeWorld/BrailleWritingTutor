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

import edu.cmu.logger.EmulatorLogger;

/**
 * A wrapper of an embedded Jetty 9.0 server.
 * @author ziw
 *
 */
public class JettyServer {
	
	private Server server;
	private HttpServlet genericServlet;
	public static final int DEFAULT_PORT = 8887;
	private int port;
	
	public static final int READY = 0;
	public static final int RUNNING = 1;
	public static final int OFFLINE = 2;
	public static final int ERROR = -1;
	
	private Logger logger = EmulatorLogger.getServerLogger();
	
	public JettyServer(HttpServlet servlet) throws Exception{
		this(DEFAULT_PORT, servlet);
	}
	
	public JettyServer(int port, HttpServlet servlet) throws Exception{
		this.genericServlet = servlet;
		this.port = (port>0) ? port : DEFAULT_PORT;
		server = new Server(this.port);
		
		logger.info("Initializing server. Port: " + port);
		initServerHandler();
		logger.info("Server and servlet initialized. Ready for JettyServer#startServer()");
	}
	
	public void startServer() throws Exception{
		logger.info("Starting server.");
		server.start();
		logger.info("Server started.");
	}
	
		
	public void stopServer() throws Exception{
		logger.info("Stopping server.");
		server.stop();
		logger.info("Server stopped.");
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
	
	private void initServerHandler() throws Exception{
		logger.info("Initializing servlet.");
		HandlerList handlers = new HandlerList();
		//handler for static files
		ResourceHandler resource_handler = new ResourceHandler();
        resource_handler.setDirectoriesListed(false);
        URL htmlPath = JettyServer.class.getClassLoader().getResource("html");
        if(htmlPath == null){
        	//no html folder found
        	logger.error("No 'html' folder found in jar file. No static files served. Fatal.");
        	throw new Exception("No 'html' folder found in jar file. No static files served. Fatal.");
        }
        resource_handler.setResourceBase(htmlPath.toExternalForm());
        
        //handler for ajax request
        ServletContextHandler context = new ServletContextHandler(ServletContextHandler.SESSIONS);
        context.setContextPath("/");
        context.addServlet(new ServletHolder(this.genericServlet),"/*");
        
        //add both handlers
        handlers.setHandlers(new Handler[] { resource_handler, context});
        
        server.setHandler(handlers);
	}

}
