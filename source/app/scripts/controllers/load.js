/** @file load.js
 *  @brief Loads things necessary for our app (event handlers, patches, etc.)
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  var mapping;

  /** @brief Main method for load.
   */
  var main = function() {
    patch();
    get_mapping();
  };

  /** @brief Gets the mapping from the mapping.json file.
   */
  var get_mapping = function() {
    $.ajax({
      type: 'GET',
      url: './assets/input_mapping.json',
      success: function(data) {
        mapping = data;
        attach_handlers();
      },
      error: function(data) {
        console.log("error");
      }
    });
  };

  /** @brief Attaches a button TODO: RENAME
   */
  var addButton = function($selector, button_id) {
    var code = mapping[button_id];
    if (code === undefined) {
      console.log("Error loading button " + button_id);
    };

    $selector.on('click', (function(e) {
      e.preventDefault();
      e.stopPropagation();
      new ButtonPress({
        'type': 'GET',
        'button': this,
        'success': function(e){console.log(e);},
        'failure': function(e){console.log(e);}
      });
    }).bind(code));
  };

  /** @brief Attaches event handlers necessary for our app.
   */
  var attach_handlers = function() {
    addButton($("#button1"), "l"); // a n
    addButton($("#button2"), "j1"); // b n
    addButton($("#button3"), "j2"); // c n
    addButton($("#button4"), "j3"); // d n
    addButton($("#button5"), "j4"); // e n
    addButton($("#button6"), "j5"); // f n
    addButton($("#button7"), "j6"); // g n
    addButton($("#button8"), "r"); // a n
  };

  /** @brief Patches functions for our app (e.g. bind if running on iOS)
   */
  var patch = function() {
  };

  // run our main method
  main();
});
