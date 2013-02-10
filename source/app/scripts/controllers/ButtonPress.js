/** @file ButtonPress.js
 *  @brief Class that represents a button press and sends an AJAX request to
 *         our local server. To be used to communicate between the JAVA
 *         servlet server and the front end.
 *
 *  Usage: new ButtonPress({
 *           'code': 'a n',
 *           'type': 'GET',
 *           'success': function(data) { console.log(data); },
 *           'failure': function(data) { console.log(data); }
 *         });
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  /** @brief Constructor for a request.
   *
   *  @param options Options for the request. These include:
   *           -code: Byte code for the request (e.g. "b n")
   *           -type: "GET" or "POST"
   *           -success: Callback to be used on success.
   *           -failure: Callback to be used on failure.
   *           -TODO: more?
   */
  window.ButtonPress = function(options) {
    this.code = options.code;
    this.type = options.type;
    this.success = options.success;
    this.failure = options.failure;

    this.__make_request();
  };

  /** @brief Makes the request.
   */
  window.ButtonPress.prototype.__make_request = function() {
    // __TEMP__ is bad
    var __TEMP__ = "foobar";
    console.log("bytecode: \"" + this.code + "\"");
    $.ajax({
      url: '/' + __TEMP__ + '/sendBytes.do?code=' + this.code,
      type: this.type,
      success: this.success,
      error: this.failure
    });
  };
});
