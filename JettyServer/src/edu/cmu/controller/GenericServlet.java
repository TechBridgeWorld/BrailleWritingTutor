package edu.cmu.controller;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * A generic servlet that handles all HTTP requests except for static files.
 * 
 * @author ziw
 *
 */
public class GenericServlet extends HttpServlet{
	

	private static final long serialVersionUID = 895965106069516878L;
	
	private ActionHandler handler;
	
	public GenericServlet(){
		
		super();
		System.out.println("Initializing Servlet...");
		System.out.println("Initializing ActionHandler...");
		handler = new ActionHandler();
		System.out.println("Servlet intialized.");
		
	}
	
	@Override
	 protected void doGet(HttpServletRequest request, HttpServletResponse response) 
			 throws ServletException, IOException{

//		System.out.println(request.getRequestURL()+request.getQueryString());
		String requestURL = request.getRequestURL().toString();
		
		if(requestURL!=null){
			String query = request.getQueryString();
			if(query == null || query.trim().length()<=0 || !requestURL.endsWith(".do")){
				//TODO
				//handle non-query request
//				System.out.println("Hanlde non query request...Ignore for now");
				return;
			}
			else{
				if(query == null || query.trim().length() <=0){
					handleError(request,response);
					return;
				}
				handler.handleButtonCode(request.getParameter("code"));
				System.out.println("Handle query request with query: " + query);
				System.out.println("Button code: " + request.getParameter("code"));
				return;				
			}
		}
		handleError(request, response);
		
	}
	
	private void handleError(HttpServletRequest request, HttpServletResponse response){
		//handle error. log invalid request / redirect to error page.
		//TODO
	}
	
}
