package edu.cmu.controller;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * A generic servlet that handles all HTTP requests except for static files. It
 * invokes {@link ActionHandler} to talk to the serial COM port.
 * 
 * @author ziw
 * @see ActionHandler
 * 
 */
public class GenericServlet extends HttpServlet {

	private static final long serialVersionUID = 895965106069516878L;

	private ActionHandler handler;
	private static final String BUTTON_CODE = "code";

	public GenericServlet() {
		super();
		System.out.println("Initializing Servlet...");
		handler = new ActionHandler();
		System.out.println("Servlet intialized.");
	}

	@Override
	protected void doGet(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException {
		
		String requestURL = request.getRequestURL().toString();
		if (requestURL != null) {
			if(requestURL.endsWith("loading.do")){
				try {					
					handler.initSerialComm();	
					sendJSON("TwoWaySerialComm initialized.", response);
				} catch (Exception e) {
					response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
				}
				return;
			}
			
			String query = request.getQueryString();
			if (query == null || query.trim().length() <= 0
					|| !requestURL.endsWith(".do")) {
				// static files requests are handled via another handler.
				// This servlet should only handle ajax request with buttonCode query.
				// so handle this request as error.
				handleError(request, response);
				return;
			} else {
				if (query == null || query.trim().length() <= 0) {
					handleError(request, response);
					return;
				}
				String buttonCode = request.getParameter(BUTTON_CODE);
//				sendJSON(buttonCode, response);
				handler.handleButtonCode(buttonCode);
				System.out.println("Handle query request with query: " + query);
				System.out.println("Button code: "
						+ request.getParameter("code"));
				return;
			}
		}
		handleError(request, response);

	}
	
	@Override
	protected void doPost(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException {
		doGet(request, response);
	}

	private void sendJSON(String text, HttpServletResponse response) {
		if (response == null)
			return;
		response.setContentType("text/plain");
		response.setCharacterEncoding("UTF-8");
		try {
			response.getWriter().write("message from server: " +text);
		} catch (IOException e) {
			System.out.println("Exception in response.write");
			System.out.println(e.getMessage());
			e.printStackTrace();
		}
	}


	private void handleError(HttpServletRequest request,
			HttpServletResponse response) {
		// handle error. log invalid request / redirect to error page.
		// just redirect to welcome file.
		try {
			response.sendRedirect("/");
		} catch (IOException e) {
			System.out.println("Exception when redirecting to /");
			e.printStackTrace();
		}
	}

}
