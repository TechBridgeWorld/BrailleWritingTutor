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

  /** @brief Attaches a button TODO: RENAME
   */
  var addButton = function($selector, button_id) {
    $selector.on('click', function(e) {
      e.preventDefault();
      e.stopPropagation();
      new ButtonPress({
        'type': 'GET',
        'button': button_id,
        'success': function(e){console.log(e);},
        'failure': function(e){console.log(e);}
      });
    });
  };

  /** @brief Attaches event handlers necessary for our app.
   */
  var attach_handlers = function() {
    addButton($("#button1"), "a n");
    addButton($("#button2"), "b n");
    addButton($("#button3"), "c n");
    addButton($("#button4"), "d n");
    addButton($("#button5"), "e n");
    addButton($("#button6"), "f n");
    addButton($("#button7"), "g n");
    addButton($("#button8"), "a n");
  };

  /** @brief Patches functions for our app (e.g. bind if running on iOS)
   */
  var patch = function() {
  };

  // run our main method
  main();
});
