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
  var __NUM_SLATEDOTS_LEFT = 3; // number of dots in left group
  var __GLYPH_MAP = {}; // object holding our glyphs
  var __CODE_TO_GLYPH_ID = {}; // object mapping keycodes to glyph IDs
  window.__BUTTON_MAP = {}; // object holding our buttons
  window.cur_mouseover = undefined;

  // initialize glyphs to off
  window.__GLYPHS_ENABLED = false;

  /** @brief Main method for load.
   */
  var main = function main() {
    // show the loading screen while loading
    window.show_loading();
    window.hide_alert();
    initializeRecording();
    patch();
    populate_dom();
    configure_plugins();
    attach_handlers();
    init_processor();
    add_tooltips();
    load_server();
  };

  /** @brief Used to load the emulator. The server needs to spawn a TwoWaySerialComm
   *         object, which takes some time, so wait until it has done so to show the
   *         UI.
   */
  var load_server = function load_server() {
    // to load, send an ajax request to /loading.do
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
        'class': 'slategroup shadow',
        'groupnumber': i + 1
      });

      // Add mouseover handler for the slategroup for our glyphs. See Glyph.js
      // for more info
      $slategroup.on('mouseover', (function(e) {
        e.preventDefault();
        e.stopPropagation();

        // clear current mouseover if any
        if (window.__GLYPHS_ENABLED === true) {
          if (window.cur_mouseover !== undefined) {
            window.cur_mouseover.removeClass('mouseover');
          };
          window.cur_mouseover = undefined;

          // set the mouse_cell and cur_mouseover to this slategroup
          window.mouse_cell = "_slate" + this.attr('groupnumber') + "_";
          window.cur_mouseover = this;
          window.cur_mouseover.addClass('mouseover');
        };
      }).bind($slategroup));

      // Add the buttons to each slate group
      var j;
      var $leftgroup = $('<div>', {
        'class': 'left subslate'
      });
      var $rightgroup = $('<div>', {
        'class': 'right subslate'
      });
      for (j = 0; j < __NUM_SLATEDOTS; j++) {
        // create the slate cell
        var $slatecell = $('<div>', {
          'class': 'slatebutton button',
          'id': '_slate' + (i + 1) + '_' + (j + 1)
        }).css('position', 'relative');

        if (j < __NUM_SLATEDOTS_LEFT) {
          // if in first __NUM_SLATEDOTS_LEFT dots, append to left group
          $leftgroup.append($slatecell);
        } else {
          // otherwise append the right group
          $rightgroup.append($slatecell);
        };
      };

      // append the left and right subslates to the slate group
      $slategroup.append($leftgroup);
      $slategroup.append($rightgroup);

      // append a glyph display
      var $glyph_display = $('<p>', {
        'class': 'glyph_display'
      });
      $slategroup.append($glyph_display);

      // add to row 1 if first 16, row 2 otherwise
      var $row;
      if (i < __NUM_SLATEGROUPS) {
        $row = $row1;
      } else {
        $row = $row2;
      };

      // append the slate group to the slate row
      $row.append($slategroup);
    };

    // add event handler for the board to remove window.mouse_cell when
    // not hovering over a cell
    $(window).on('mouseover', function(e) {
      e.preventDefault();
      e.stopPropagation();

      if (window.__GLYPHS_ENABLED === true) {
        window.mouse_cell = undefined;
        if (window.cur_mouseover !== undefined) {
          window.cur_mouseover.removeClass('mouseover');
        };
        window.cur_mouseover = undefined;
      };
    });
  };

  /** @brief Configures plugins. Adds minimize buttons, etc.
   */
  var configure_plugins = function configure_plugins() {
    // helped function to add a minimize button to the specified element
    var add_minimize = function add_minimize($el) {
      var $minimize = $('<div>', {
        'class': 'minimizer tips'
      });

      // click handler for the minimize button
      $minimize.on('click', function(e) {
        e.preventDefault();
        e.stopPropagation();

        // toggle the content from hidden to shown on click
        $el.find('.content').slideToggle();
//        window.timed_app_alert("Critical", "Some stuff", 2000);

        if ($el.hasClass('minimized')) {
          // if currently minimized, maximize it
          $el.removeClass('minimized');
          $el.addClass('active');
        } else {
          // otherwise, minimize it
          $el.addClass('minimized');
          $el.removeClass('active');
        };
      });

      // append the element
      $el.append($minimize);
    };

    // add a minimize button to each element
    $(".plugin").each(function(index, el) {
      add_minimize($(el));
    });
  };

  /** @brief Attaches event handlers to a specific button.
   *
   *  @param $dom_el jQuery object representing the DOM element we are attaching
   *                 this button to.
   */
  var add_button = function add_button($dom_el) {
    // the button id is always stored in the id field in the DOM
    var button_id = $dom_el.attr('id');

    // find the code from our mapping
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
        // if shift is being held, consider this click a hold
        __BUTTON_MAP[button_id].hold_down();
      } else {
        // otherwise, consider it a press
        __BUTTON_MAP[button_id].press_down();
      };
    });

    // on mouse up, trigger the press_up() method
    $dom_el.on('mouseup', function(e) {
      e.preventDefault();
      e.stopPropagation();
      __BUTTON_MAP[button_id].press_up();
    });
  };

  /** @brief Attaches event handlers necessary for our app.
   */
  var attach_handlers = function attach_handlers() {
    // default power tips to enabled
    window.powerTipsEnabled = true;

    // add buttons
    $(".button").each(function(ind, el) {
      add_button($(el));
    });

// @TODO: Get this working
//    // Add mouseover handler for the jumbocell for our glyphs. See Glyph.js
//    // for more info
//    $("#jumboslate").on('mouseover', (function(e) {
//      e.preventDefault();
//      e.stopPropagation();
//
//      // clear current mouseover if any
//      if (window.__GLYPHS_ENABLED === true) {
//        if (window.cur_mouseover !== undefined) {
//          window.cur_mouseover.removeClass('mouseover');
//        };
//        window.cur_mouseover = undefined;
//
//        // set the mouse_cell and cur_mouseover to this slategroup
//        window.mouse_cell = "_slate" + this.attr('groupnumber') + "_";
//        window.cur_mouseover = this;
//        window.cur_mouseover.addClass('mouseover');
//      };
//    }));

    attach_toggle_buttons();
    attach_glyph_handlers();
  };

  /** @brief Attaches handlers to buttons to send the glyph associated with that letter.
   */
  var attach_glyph_handlers = function attach_glyph_handlers() {
    // first, create glyphs for each of the letters in glyph_mapping
    var i;
    for (i in window.glyph_mapping) {
      var this_glyph = new window.Glyph({
        'id': i,
        'code': window.glyph_mapping[i]
      });

      __GLYPH_MAP[i] = this_glyph;
    };

    // attach a global keypress handler to the window to handle all glyph button
    // presses rather than per-letter handlers
    $(window).on('keypress', function(e) {
      e.preventDefault();
      e.stopPropagation();

      // only update if glyphs are enabled
      if (window.__GLYPHS_ENABLED === true) {
        var key_code = e.keyCode || e.which;
        try {
          var this_glyph = __GLYPH_MAP[__CODE_TO_GLYPH_ID[key_code]];
          this_glyph.send();
        } catch(err) {
          // Only throws if the key press isn't registered as a glyph button, so
          // just ignore it
          return;
        };
      };
    });

    // now bind keypresses to fire these glyphs

    /** @brief Helper to attach a specific letter to the window handler.
     *
     *  @param keycode Which ascii keycode should fire this glyph.
     *  @param id The ID of the glyph to fire (must match key in window.glyph_mapping).
     */
    var attach_letter = function attach_letter(keycode, id) {
      __CODE_TO_GLYPH_ID[keycode] = id;
    };

    // ENGLISH
    // add each of the english letters as glyphs
    var english_alphabet = "abcdefghijklmnopqrstuvwxyz".split('');
    english_alphabet.map(function(el) {
      attach_letter(el.charCodeAt(0), el);
    });

    // OTHER LANGUAGES
    // if you want to add support for glyphs in other languages, attach them here.
    // be sure to update /assets/glyph_mapping.js as well
  };

  /** @brief Helper function to attach the toggle button handlers.
   */
  var attach_toggle_buttons = function attach_toggle_buttons() {
    // helper used to manage toggle buttons
    // @param $dom_el The dom element representing the toggle button
    // @param toggle_bool The boolean used to keep track of this button's toggle status
    // @param onTrue Callback to be used when we switch to true
    // @param onFalse Callback to be usde when we switch to false
    var toggle_button_helper = function toggle_button_helper($dom_el, toggle_bool, onTrue, onFalse) {
      $dom_el.on('click', function(e) {
        e.preventDefault();
        e.stopPropagation();
        if (toggle_bool === false) {
          // if switching to true
          toggle_bool = true;

          // add the active class
          $dom_el.addClass('active');

          onTrue();
        } else {
          // if switching to false
          toggle_bool = false;

          // remove the active class
          $dom_el.removeClass('active');

          onFalse();
        };
      });
    };

    // handle the init button separately since it doesn't need to
    // adhere to strange emulator-specific timings
    var is_handshaking = false;
    toggle_button_helper($("#_initialize"), is_handshaking,
      function onTrue() {
        // send init code to the server
        window.LOG_INFO("Sending initialize");

        // update DOM to reflect press
        $('#handshake_status').html('ON').addClass('active');
        $.ajax({
          url: '/sendBytes.do?code=' + window.input_mapping['_initialize'],
          type: 'GET',
          success: function(data) {
            window.LOG_INFO("initialize succeeded");
          },
          error: function(data) {
            // TODO: handle this better
            window.LOG_ERROR("initialize failed");
          },
        });
      },
      function onFalse() {
        // otherwise send uninitialize to server
        window.LOG_INFO("Terminating initialize");

        // update DOM
        $('#handshake_status').html('OFF').removeClass('active');
        $.ajax({
          url: '/sendBytes.do?code=' + window.input_mapping['_uninitialize'],
          type: 'GET',
          success: function(data) {
            window.LOG_INFO("uninitialize succeeded");
          },
          error: function(data) {
            // TODO: handle this better
            window.LOG_ERROR("uninitialize failed");
          },
        });
      }
    );

    // handle the help button separately also
    var is_helping = true;
    toggle_button_helper($('#help_tooltips'), is_helping,
      function onTrue() {
        window.LOG_INFO("Turning help ON.");

        // update DOM
        $('#help_tooltips_status').html('ON').addClass('active');

        // show powertip
        window.powerTipsEnabled = true;
      },
      function onFalse() {
        window.LOG_INFO("Turning help OFF.");

        // update DOM
        $('#help_tooltips_status').html('OFF').removeClass('active');

        // hide tips
        window.powerTipsEnabled = false;
      }
    );

    // toggle button to enable glyphs
    toggle_button_helper($('#glyph_toggle'), window.__GLYPHS_ENABLED,
      function onTrue() {
        window.LOG_INFO("Turning glyphs ON.");

        // update DOM
        $('#glyphs_enabled_status').html('ON').addClass('active');
        window.__GLYPHS_ENABLED = true;
      },
      function onFalse() {
        window.LOG_INFO(window.__GLYPHS_ENABLED);
        window.LOG_INFO("Turning glyphs OFF.");

        // update DOM
        $('#glyphs_enabled_status').html('OFF').removeClass('active');
        window.__GLYPHS_ENABLED = false;
      }
    );
  };

  /** @brief Patches functions for our app (e.g. bind if running on iOS)
   */
  var patch = function patch() {
  };

  /** @brief Adds tooltips to items people may need help with.
   */
  window.add_tooltips = function add_tooltips() {
    window.add_info($('#_initialize'),
      'Handshaking',
      'Toggles the handshaking process. See the \'<a href="#" class="tooltip_link">Getting Started</a>\' tutorial for ' +
      'more information.', 'se'
    );

    window.add_info($('.minimizer'),
      'Maximize/Minimize',
      'Minimize or maximize this plugin.',
      'w'
    );

    window.add_info($('#help_tooltips'),
      'Helpful Tooltips',
      'Toggles helpful mouseover tooltips like this one.',
      'se'
    );

    window.add_info($('#glyph_toggle'),
      'Glyphs',
      'Toggles the use of glyphs. See \'<a href="#" class="tooltip_link">Glyphs</a>\' for' +
      'more information.',
      'se'
    );
  };


  // run our main method
  main();
});
