/** @file load.js
 *  @brief Loads things necessary for our app (event handlers, patches, etc.)
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  "use strict";

  var __NUM_SLATEGROUPS = 16; // number of slate groups per row
  var __NUM_SLATEROWS = 2;  // number of slate rows
  var __NUM_SLATEDOTS = 6;  // number of dots per slate group
  var __BUTTON_MAP = {}; // object holding our buttons

  /** @brief Main method for load.
   */
  var main = function main() {
    // show the loading screen while loading
    window.show_loading();

    patch();
    populate_dom();
    attach_handlers();
    init_processor();
    load_server();
  };

  /** @brief Used to load the emulator. The server needs to spawn a TwoWaySerialComm
   *         object, which takes some time, so wait until it has done so to show the
   *         UI.
   */
  var load_server = function load_server() {
    $.ajax({
      url: '/loading.do',
      type: 'GET',
      success: function(data) {
        window.hide_loading();
      },
      error: function(data) {
        // TODO: handle this
        window.LOG_WARNING("Cannot load server");
        window.hide_loading();
      }
    });
  };

  /** @brief Initializes the main processor.
   */
  var init_processor = function init_processor() {
    window._Processor = new window.Processor();
    window._Processor.run();
  };

  /** @brief Populates the DOM with objects we don't want to hardcode into
   *         index.html.
   */
  var populate_dom = function populate_dom() {
    // Add slates to the DOM
    var $row1 = $("#slaterow1");
    var $row2 = $("#slaterow2");

    // Add all the slate groups to the rows
    var i;
    for (i = 0; i < __NUM_SLATEROWS * __NUM_SLATEGROUPS; i++) {
      var $slategroup = $('<div>', {
        'class': 'slategroup shadow'
      });

      // Add the buttons to each slate group
      var j;
      var $leftgroup = $('<div>', {
        'class': 'left subslate'
      });
      var $rightgroup = $('<div>', {
        'class': 'right subslate'
      });
      for (j = 0; j < __NUM_SLATEDOTS; j++) {
        var $slatecell = $('<div>', {
          'class': 'slatebutton button',
          'id': '_slate' + (i + 1) + '_' + (j + 1)
        }).css('position', 'relative');

        if (j < 3) {
          $leftgroup.append($slatecell);
        } else {
          $rightgroup.append($slatecell);
        };
      };
      $slategroup.append($leftgroup);
      $slategroup.append($rightgroup);

      // add to row 1 if first 16, row 2 otherwise
      var $row;
      if (i < __NUM_SLATEGROUPS) {
        $row = $row1;
      } else {
        $row = $row2;
      };
      $row.append($slategroup);
    };
  };

  /** @brief Attaches event handlers to a specific button.
   */
  var add_button = function add_button($dom_el) {
    var button_id = $dom_el.attr('id');
    var code = window.input_mapping[button_id];
    if (code === undefined) {
      window.LOG_ERROR("Cannot load button " + button_id);
    };

    // Generally shouldn't be adding same button twice, but it's okay to do so.
    // Just overwrites previous button with new one.
    if (__BUTTON_MAP[button_id] !== undefined) {
      window.LOG_WARNING("Adding button \"" + button_id + "\" twice");
    };

    // Add this button to our button map
    __BUTTON_MAP[button_id] = new Button({
      'code': code,
      'dom_el': $dom_el
    });

    // Add the on mouse down and mouse up events
    $dom_el.on('mousedown', function(e) {
      e.preventDefault();
      e.stopPropagation();

      if (e.shiftKey === true) {
        __BUTTON_MAP[button_id].hold_down();
      } else {
        __BUTTON_MAP[button_id].press_down();
      };
    });

    $dom_el.on('mouseup', function(e) {
      e.preventDefault();
      e.stopPropagation();
      __BUTTON_MAP[button_id].press_up();
    });
  };

  /** @brief Attaches event handlers necessary for our app.
   */
  var attach_handlers = function attach_handlers() {
    // add buttons
    $(".button").each(function(ind, el) {
      add_button($(el));
    });

    // handle the init button separately since it doesn't need to
    // adhere to strange emulator-specific timings
    $("#_initialize").on('click', function(e) {
      e.preventDefault();
      e.stopPropagation();

      // send init code to the server
      window.LOG_INFO("Sending initialize");
      $.ajax({
        url: '/sendBytes.do?code=' + window.input_mapping['_initialize'],
        type: 'GET',
        success: function(data) {
          window.LOG_INFO("initialize succeeded");
        },
        error: function(data) {
          window.LOG_WARNING("initialize failed");
        },
      });
    });
  };

  /** @brief Patches functions for our app (e.g. bind if running on iOS)
   */
  var patch = function patch() {
  };

  // run our main method
  main();
});
