package edu.cmu.server;
import org.eclipse.jetty.server.Handler;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.handler.HandlerList;
import org.eclipse.jetty.server.handler.ResourceHandler;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;

import edu.cmu.controller.GenericServlet;

public class JettyServer {
	
	private Server server;
	private static final int DEFAULT_PORT = 8887;
	private int status;
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
		status = READY;
	}
	
	public void startServer(){
		try {
			server.start();
		} catch (Exception e) {
			System.out.println("Exception caught when starting server");
		
		}
		
	}
	
	public void restartServer(){
		
	}
	
	public void stopServer(){
		try {
			server.stop();
		} catch (Exception e) {
			System.out.println("Exception when stopping server");
			e.printStackTrace();
		}
	}
	
	public boolean isStarted(){
		return server.isStarted();
	}
	
	public boolean isStopped(){
		return server.isStopped();
	}
	
	public int getServerStatus(){
		return this.status;
	}
	
	public int getPort(){
		return port;
	}
	
	private void initServerHandler(){
		HandlerList handlers = new HandlerList();
		ResourceHandler resource_handler = new ResourceHandler();
        resource_handler.setDirectoriesListed(false);
	    resource_handler.setWelcomeFiles(new String[]{ "index.html" });
        resource_handler.setResourceBase(".");
        
        ServletContextHandler context = new ServletContextHandler(ServletContextHandler.SESSIONS);
        context.setContextPath("/");
        context.addServlet(new ServletHolder(new GenericServlet()),"/");
        
        handlers.setHandlers(new Handler[] { resource_handler, context});
        
        server.setHandler(handlers);
	}


}
