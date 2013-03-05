/** @file util.js
 *  @brief General utility functions.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  /** @brief Logs at the info level.
   *
   *  @param message The body of the message.
   */
  window.LOG_INFO = function LOG_INFO(message) {
    // only log info messages if __DEBUG is on
    if (window.__DEBUG === true) {
      console.log("Info: " + message);
    };
  };

  /** @brief Logs at the warning level.
   *
   *  @param message The body of the message.
   */
  window.LOG_WARNING = function LOG_WARNING(message) {
    // by default, log all warnings
    console.log("Warning: " + message);
  };

  /** @brief Logs at the error level.
   *
   *  @param message The body of the message.
   */
  window.LOG_ERROR = function LOG_ERROR(message) {
    // throw errors
    throw "Error: " + message;
  };

  /** @brief Logs at the critical level.
   *
   *  @param message The body of the message.
   */
  window.LOG_CRITICAL = function LOG_CRITICAL(message) {
    // throw criticals and alert user with an alert
    window.app_alert("Critical Error", message);
    throw "Critical: " + message;
  };

  /** @brief Wrapper for javascript alert() since default alert() is obtrusive.
   *
   *  @param header The header for this alert.
   *  @param message The body of the message.
   */
  window.app_alert = function app_alert(header, message) {
    alert(header + ": " + message);
  };
});
