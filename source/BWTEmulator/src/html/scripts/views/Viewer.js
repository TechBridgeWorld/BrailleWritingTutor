/** @file Viewer.js
 *  @brief General js file to handle js-related views.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  /** @brief Shows the loading screen.
   */
  window.__bwt.show_loading = function show_loading() {
    $("#main").hide();
    $("#loading").show();
  };

  /** @brief Hides the loading screen.
   */
  window.__bwt.hide_loading = function hide_loading() {
    $("#loading").hide();
    $("#main").show();
  };

  /** @brief Hides the alert div.
   */
  window.__bwt.hide_alert = function hide_alert() {
    $("#alert").hide();
  };
});
