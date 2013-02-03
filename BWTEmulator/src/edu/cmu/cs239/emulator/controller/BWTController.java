package edu.cmu.cs239.emulator.controller;

import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.annotation.WebServlet;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * Servlet implementation class BWTController
 */
@WebServlet("/BWTController")
public class BWTController extends HttpServlet {
  private static final long serialVersionUID = 1L;

    /**
     * @see HttpServlet#HttpServlet()
     */
    public BWTController() {

        super();
    }

  /**
   *
   * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse response)
   */
  protected void doGet(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {

    printRequestInfo(request);
    sendJSON(response);
  }

  /**
   * Handle POST request.
   * TODO: Currently this is handled exactly the same as GET request. Need implementation
   * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse response)
   */
  protected void doPost(HttpServletRequest request, HttpServletResponse response) throws ServletException, IOException {
    System.out.println("Handling POST request as GET");
    doGet(request,response);
  }

  /**
   * Print out the incoming http request
   * @param request
   */
  private void printRequestInfo(HttpServletRequest request){
    if(request == null){
      System.out.println("Request is null.");
      return;
    }
    String url = request.getRequestURL().toString();
    String query = request.getQueryString();
    String port = Integer.toString(request.getLocalPort());
    java.util.Enumeration<String> e = request.getParameterNames();
    System.out.println("----Printing Request-----");
    System.out.println("url: " +url);
    System.out.println("query: " + query);
//    if(query!=null){
//      for(String s= "";e.hasMoreElements();s=e.nextElement()){
//        System.out.println(s);
//      }
//    }
    System.out.println("Port: " + port);

  }

  /**
   *
   * @param request
   * @param response
   */
  private void sendJSON(HttpServletResponse response){
    if(response == null) return;
    String text = "{ message: Java servlet says hello world}";
      response.setContentType("text/plain");
      response.setCharacterEncoding("UTF-8");
      try {
      response.getWriter().write(text);
    } catch (IOException e) {
      System.out.println("Exception in response.write");
      System.out.println(e.getMessage());
      e.printStackTrace();
    }
  }

}
