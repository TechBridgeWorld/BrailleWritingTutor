package edu.cmu.controller;

import java.io.IOException;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.slf4j.Logger;

import edu.cmu.logger.EmulatorLogger;
import edu.cmu.scripting.*;

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
	//parameter name
	private static final String BUTTON_CODE_PARAM_NAME = "code";
	private static final String SCRIPTS_NAME_PARAM_NAME= "name";
	
	//request names
	private static final String LOAD_REQUEST = "loading.do";
	private static final String LOAD_SCRIPTS_REQUEST = "loadScripts.do";
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

	//send the given String
	private void sendText(String text, HttpServletResponse response) {
		response.setContentType("text/plain");
		response.setCharacterEncoding("UTF-8");
		try {
			response.getWriter().write(text);
		} catch (IOException e) {
			logger.error("Exception when sending message to the server.");
			EmulatorLogger.logException(logger, e);
		}
	}

	private void handleError(HttpServletRequest request,
			HttpServletResponse response) {
		logger.error("Invalid request.");
		sendText(REQUEST_ERROR_MSG,response);
	}

	/**
	 * This method handles /loading.do.
	 * It is invoked each time index.html is loaded. It should call the 
	 * action handler to initialize Comm. port.
	 * On success this will send back a string and on error send back internal server error. 
	 * This is then caught and handled by the success/error function of jquery.ajax() call,respectively.
	 * @param request
	 * @param response
	 * @throws IOException
	 */
	private void handleInitialLoadingRequest(HttpServletRequest request,
			HttpServletResponse response) throws IOException{
		logger.info("Loading request captured. Intializing TwoWaySerialCom");
		try {					
			handler.initSerialComm();	
			//send back any non-error response will do.
			//the front end will hide the loading screen and load the emulator UI
			sendText("TwoWaySerialComm initialized.", response);
		} catch (Exception e) {
			logger.error("Exception when initializing TwoWaySerialCom");
			EmulatorLogger.logException(logger, e);
			//this will make the response be caught in the 'error' block of jquery.ajax()
			response.sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
		}
		return;
	}
	
	/**
	 * This method handles /sendBytes.do.
	 * The request should contain a button's byte code as parameter.
	 * It will then pass the code to action handler and send it to the BWT software.
	 * @param request
	 * @param response
	 */
	private void handleButtonRequest(HttpServletRequest request,
			HttpServletResponse response){
		
		String query = request.getQueryString();
		if (query == null || query.trim().length() == 0) {
			logger.error("send bytes request failed. Query is null. No button code found.");
			handleError(request, response);
		} else {
			//trim the query
			String buttonCode = request.getParameter(BUTTON_CODE_PARAM_NAME);
			handler.handleButtonCode(buttonCode);
		}
		
	}
	
	/**
	 * This method handles /loadScripts.do.
	 * It will call ScriptLoader or ScriptParser according to the parameter.
	 * After the loading/parsing send back a json string to be parsed by the front end.
	 * @param request
	 * @param response
	 */
	private void handleScriptingRequest(HttpServletRequest request,
			HttpServletResponse response){
		String scriptName = request.getParameter(SCRIPTS_NAME_PARAM_NAME);
		if(scriptName == null){
			logger.error("No script file name found with /loadScripts.do ");
			return;
		}
		else if(scriptName.equals("listing")){
			sendText(ScriptLoader.getAllScriptNames(),response);
			return;
		}
		String jsonString = ScriptLoader.loadScript(scriptName);
		sendText(jsonString, response);
	}

}
