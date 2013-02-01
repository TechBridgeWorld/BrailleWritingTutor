/** @file load.js
 *  @brief Loads things necessary for our app (event handlers, patches, etc.)
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  /** @brief Main method for load.
   */
  var main = function() {
    attach_handlers();
    patch();
  };

  /** @brief Attaches event handlers necessary for our app.
   */
  var attach_handlers = function() {
  };

  /** @brief Patches functions for our app (e.g. bind if running on iOS)
   */
  var patch = function() {
  };

  // run our main method
  main();
});
