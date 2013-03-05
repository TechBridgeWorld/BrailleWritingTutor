/** @file util.js
 *  @brief General utility functions.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  /** @brief Logs at the info level.
   */
  window.LOG_INFO = function LOG_INFO(message) {
    // only log info messages if __DEBUG is on
    if (window.__DEBUG === true) {
      console.log("Info: " + message);
    };
  };

  /** @brief Logs at the warning level.
   */
  window.LOG_WARNING = function LOG_WARNING(message) {
    // by default, log all warnings
    console.log("Warning: " + message);
  };

  /** @brief Logs at the error level.
   */
  window.LOG_ERROR = function LOG_ERROR(message) {
    // throw errors
    throw "Error: " + message;
  };

  /** @brief Logs at the critical level.
   */
  window.LOG_CRITICAL = function LOG_CRITICAL(message) {
    // for now, throw criticals. May want to also alert some kind of
    // admin about this error
    throw "Critical: " + message;
  };
});
