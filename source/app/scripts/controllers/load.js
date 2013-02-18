/** @file load.js
 *  @brief Loads things necessary for our app (event handlers, patches, etc.)
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  var mapping;  // holds the mapping from id to bytecode
  var __NUM_SLATEGROUPS = 16; // number of slate groups per row
  var __BUTTON_MAP = {}; // object holding our buttons

  /** @brief Main method for load.
   */
  var main = function() {
    patch();
    populate_dom();
    attach_handlers();
//    get_mapping(); // XXX: deprecated. leaving for now in case we need to revert
  };

  /** @brief Populates the DOM with objects we don't want to hardcode into
   *         index.html.
   */
  var populate_dom = function() {
    // Add slates to the DOM
    var $row1 = $("#slaterow1");
    var $row2 = $("#slaterow2");

    // Add all the slate groups to the rows
    var i;
    for (i = 0; i < 2 * __NUM_SLATEGROUPS; i++) {
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
      for (j = 0; j < 6; j++) {
        var $slatecell = $('<div>', {
          'class': 'slatebutton button',
          'id': '_s' + (i + 1) + '_' + (j + 1)
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

  /** @brief Gets the mapping from the mapping.json file.
   */
  var get_mapping = function() {
    // DEPRECATED. replaced input_mapping.json.
//    $.ajax({
//      type: 'GET',
//      url: './assets/input_mapping.json',
//      success: function(data) {
//        mapping = data;
//        attach_handlers();
//      },
//      error: function(data) {
//        console.log("Error loading input_mapping.json");
//      }
//    });
  };

  /** @brief Attaches a button TODO: RENAME
   */
  var addButton = function($selector, button_id, onclick) {
    var code = window.input_mapping[button_id];
    if (code === undefined) {
      console.log("Error loading button " + button_id);
    };

    // Generally shouldn't be adding same button twice, but it's okay to do so.
    // Just overwrites previous button with new one.
    if (__BUTTON_MAP[button_id] !== undefined) {
      console.log("Warning: adding button \"" + button_id + "\" twice");
    };

    // Add this button to our button map
    __BUTTON_MAP[button_id] = new Button({
      'code': code,
      'success': function(e){console.log(e);},
      'failure': function(e){console.log(e);} // TODO: fix success/failure
    });

    // Add the DOM event listener
    if (onclick !== undefined) {
      // If user selected own onclick, user theirs

      // Package up all the information we need so we can bind
      var pass_to_handler = {
        'onclick': onclick,
        'button': __BUTTON_MAP[button_id]
      };
      $selector.on('click', (function(e) {
        (this['onclick'])(e, this['button']);
      }).bind(pass_to_handler));
    } else {
      // Otherwise, use default click handler
      $selector.on('click', (function(e) {
        e.preventDefault();
        e.stopPropagation();
        this.press();
      }).bind(__BUTTON_MAP[button_id]));
    };
  };

  /** @brief Attaches event handlers necessary for our app.
   */
  var attach_handlers = function() {
    // add buttons
    $(".button").each(function(ind, el) {
      var $el = $(el);
      addButton($el, $el.attr('id'), undefined);
    });

    // add _l and _r buttons differently, as they require different click handlers
    var $_l = $("#_l");
    var $_r = $("#_r");

    /** @brief Small helper function used to add _l and _r buttons.
     */
    var addMenuButton = function($selector) {
      addButton($selector, $selector.attr('id'), (function(e, button) {
        e.preventDefault();
        e.stopPropagation();
        button.toggle_hold();
        this.toggleClass("active");
      }).bind($selector));
    };

    addMenuButton($_l);
    addMenuButton($_r);
  };

  /** @brief Patches functions for our app (e.g. bind if running on iOS)
   */
  var patch = function() {
  };

  // run our main method
  main();
});
