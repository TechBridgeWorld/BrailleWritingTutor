package edu.cmu.server;
import java.net.URL;

import org.eclipse.jetty.server.Handler;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.handler.HandlerList;
import org.eclipse.jetty.server.handler.ResourceHandler;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;

import edu.cmu.controller.GenericServlet;

/**
 * A wrapper of an embedded Jetty 9.0 server.
 * @author ziw
 *
 */
public class JettyServer {
	
	private Server server;
	private static final int DEFAULT_PORT = 8887;
	private int port;
	
	public static final int READY = 0;
	public static final int RUNNING = 1;
	public static final int OFFLINE = 2;
	public static final int ERROR = -1;
	
	public JettyServer(){
		this(DEFAULT_PORT);
	}
	
	public JettyServer(int port){
		this.port =port>0? port : DEFAULT_PORT;
		server = new Server(this.port);
		initServerHandler();
	}
	
	public void startServer(){
		try {
			server.start();
		} catch (Exception e) {
			System.out.println("Exception caught when starting server");
		}
		
	}
	
		
	public void restartServer(){
		//TODO
	}
	
	public void stopServer(){
		try {
			server.stop();
		} catch (Exception e) {
			System.out.println("Exception when stopping server");
			e.printStackTrace();
		}
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
        context.addServlet(new ServletHolder(new GenericServlet()),"/");
        
        //add both handlers
        handlers.setHandlers(new Handler[] { resource_handler, context});
        
        server.setHandler(handlers);
	}

}
