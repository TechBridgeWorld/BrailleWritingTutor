package edu.cmu.server;
import org.eclipse.jetty.server.Handler;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.handler.HandlerList;
import org.eclipse.jetty.server.handler.ResourceHandler;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;

import edu.cmu.controller.GenericServlet;

public class JettyServer {
	
	public JettyServer(){
		
	}
	
	public void initServer(){

		try {
			Server server = new Server(8886);
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
			server.start();
//			server.join();
			System.out.println("Jetty Server started.");
			
			
//			System.out.println(server.isStarted());
//			Thread.sleep(3000);
//			server.stop();
//			System.out.println(server.isStopping());
//			System.out.println(server.isStopped());
		}catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		JettyServer js = new JettyServer();
		js.initServer();
		
	}

}
