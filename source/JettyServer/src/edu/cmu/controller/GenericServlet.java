package edu.cmu.controller;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;

import edu.cmu.logger.EmulatorLogger;

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
	private Logger logger = EmulatorLogger.getEmulatorInfoLogger();
	private AbstractActionHandler handler;
	
	
	private static final String BUTTON_CODE_PARAM_NAME = "code";
	private static final String LOADING_REQUEST = "loading.do";
	
	private static final String REQUEST_ERROR_MSG = "Invalid HTTP request.";
	
	public GenericServlet(AbstractActionHandler handler) {
		this.handler = handler;
	}
	
	

	@Override
	protected void doGet(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException {
		
		String requestURL = request.getRequestURL().toString();
		logger.info("Handling request: " + requestURL);
		
		if (requestURL != null) {
			if(requestURL.endsWith(LOADING_REQUEST)){
				logger.info("Loading request captured. Intializing TwoWaySerialCom");
				try {					
					handler.initSerialComm();	
					sendText("TwoWaySerialComm initialized.", response);
				} catch (Exception e) {
					logger.error("Exception when initializing TwoWaySerialCom");
					EmulatorLogger.logException(logger, e);
					
					//this will make the response be caught in the 'error' block of jquery.ajax()
					response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
				}
				return;
			}
			
			String query = request.getQueryString();
			if (query == null || query.trim().length() == 0
					|| !requestURL.endsWith(".do")) {
				// static files requests are handled via another handler.
				// This servlet should only handle ajax request with buttonCode that ends with .do.
				// so handle this request as error.
				handleError(request, response);
			} else {
				//trim the query
				query = query.trim();
				String buttonCode = request.getParameter(BUTTON_CODE_PARAM_NAME);
				handler.handleButtonCode(buttonCode);
			}
		}
		else{
			handleError(request, response);
		}

	}
	
	@Override
	protected void doPost(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException {
		doGet(request, response);
	}

	//send message
	private void sendText(String text, HttpServletResponse response) {
		response.setContentType("text/plain");
		response.setCharacterEncoding("UTF-8");
		try {
			response.getWriter().write("message from server: " +text);
		} catch (IOException e) {
			logger.error("Exception when sending message to the server.");
			EmulatorLogger.logException(logger, e);
		}
	}

	private void handleError(HttpServletRequest request,
			HttpServletResponse response) {
		// handle error. log invalid request / redirect to error page.
		logger.error("Invalid request. Request is null.");
		sendText(REQUEST_ERROR_MSG,response);
	}

}
