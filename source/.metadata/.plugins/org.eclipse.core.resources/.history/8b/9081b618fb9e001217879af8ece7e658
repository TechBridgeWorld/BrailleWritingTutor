package edu.cmu.controller;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;

import edu.cmu.logger.EmulatorLogger;

/**
 * A generic servlet that handles all HTTP requests except for serving static files. It
 * invokes {@link AbstractActionHandler} to talk to the serial COM port.
 * 
 * @author ziw
 * 
 */
public class GenericServlet extends HttpServlet {

	private static final long serialVersionUID = 895965106069516878L;
	private Logger logger = EmulatorLogger.getEmulatorInfoLogger();
	private AbstractActionHandler handler;
	
	
	private static final String BUTTON_CODE_PARAM_NAME = "code";
	private static final String LOAD_REQUEST = "loading.do";
	private static final String LOAD_SCRIPTS_REQUEST = "scripting.do";
	private static final String SEND_BYTES_REQUEST = "sendBytes.do";
	
	private static final String REQUEST_ERROR_MSG = "Invalid HTTP request.";
	
	public GenericServlet(AbstractActionHandler handler) {
		this.handler = handler;
	}
	
	

	@Override
	protected void doGet(HttpServletRequest request,
			HttpServletResponse response) throws ServletException, IOException {
		
		String requestURL = request.getRequestURL().toString();
		logger.info("Handling request: " + requestURL);
		if(requestURL == null){
			handleError(request,response);
		}
		else if(requestURL.endsWith(LOAD_REQUEST)){
			handleInitialLoadingRequest(request, response);
		}
		else if(requestURL.endsWith(LOAD_SCRIPTS_REQUEST)){
			handleScriptingRequest(request, response);
		}
		else if(requestURL.endsWith(SEND_BYTES_REQUEST)){
			handleButtonRequest(request, response);
		}
		else{
			handleError(request,response);
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
	
	private void handleInitialLoadingRequest(HttpServletRequest request,
			HttpServletResponse response) throws IOException{
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
	
	private void handleButtonRequest(HttpServletRequest request,
			HttpServletResponse response){
		
		String query = request.getQueryString();
		if (query == null || query.trim().length() == 0) {
			logger.error("send bytes request failed. Query is null. ");
			handleError(request, response);
		} else {
			//trim the query
			query = query.trim();
			String buttonCode = request.getParameter(BUTTON_CODE_PARAM_NAME);
			handler.handleButtonCode(buttonCode);
		}
		
	}
	
	private void handleScriptingRequest(HttpServletRequest request,
			HttpServletResponse response){
		
	}

}
