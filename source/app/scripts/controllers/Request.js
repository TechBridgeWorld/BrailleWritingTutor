/** @file Request.js
 *  @brief Class that represents an AJAX request to our local server. To be used
 *         to communicate between the JAVA servlet server and the front end.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  /** @brief Constructor for a request.
   *
   *  @param options Options for the request. These include:
   *           -url: URL of the request (e.g. "/login" or "/sendData")
   *           -data: JSON object representing data we want sent. If request is
   *                  a POST, this will be in the body. If a GET, it will be in
   *                  the URL.
   *           -type: "GET" or "POST"
   *           -success: Callback to be used on success.
  *            -failure: Callback to be used on failure.
   *           -TODO: more?
   */
  var Request = function(options) {
    this.url = options.url;
    this.data = options.data;
    this.type = options.type;
    this.success = options.success;
    this.failure = options.failure;
  };

  /** @brief Makes the request.
   */
  var __make_request = function() {
//    $.ajax({
//    });
  // TODO
  };
});
