/** @file Glyph.js
 *  @brief Object representing a single glyph.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  /** @brief Helper function used to reflect cell numbers (to help fix issues
   *         caused by reading vs writing).
   *
   *  @param index The index we want to reflect.
   */
  var __reflect = function __reflect(index) {
    return (((index + 2) % 6) + 1);
  };

  /** @brief Constructor for a glyph.
   *
   *  @param options Options for this glyph. These include:
   *           -id: Identifier for this glyph (e.g. "a" to represent
   *                the letter "a")
   *           -code: comma-separated list of buttons to be pressed for this
   *                  glyph (e.g. "1,2" for "b", or "3,1,5" for "o")
   */
  window.__bwt.Glyph = function Glyph(options) {
    this.id = options.id;

    // parse options.code
    try {
      this.buttons = options.code.split(",")
        .map(function(el) {
          var this_int = parseInt(el);
          if ((this_int < 1) || (this_int > 6) || (isNaN(this_int))) {
            throw new Error(); // will be caught below and prettified
          };
          return this_int;
        }).sort();
    } catch(err) {
      window.__bwt.LOG_ERROR("Invalid glyph: " + this);
    };
  };

  /** @brief State machine that presses the next button to press in the input
   *         and then stalls to give this button time to complete.
   *
   *  @param cell_prefix The cell prefix of the slate to press.
   *  @param mouseover Current mouseover slate at time of glyph press.
   *  @param index The index of the button map we are on.
   */
  window.__bwt.Glyph.prototype.__button_step = function __button_step(cell_prefix, mouseover, index) {
    // The final call to __button_step will access an index of this.buttons that
    // is out of range. This is intended, as this is how we determine that this
    // glyph's send() is complete
    var to_press = this.buttons[index];

    // if we're done, handle the ui
    if (to_press === undefined) {
      // display that the user typed a glyph into this cell
      mouseover.find('.glyph_display').addClass('active').html(this.id);
      mouseover.addClass('glyphd');
      var self = this;
      var cur_mousecell = "_slate" + mouseover.attr('groupnumber') + "_";
      setTimeout((function() {
        this.removeClass('glyphd');
        self.buttons.map(function(el) {
          $('#' + cur_mousecell + __reflect(el)).removeClass('button_glyphd');
          (mouseover).find('.glyph_display').removeClass('active').html('');
        });
      }).bind(mouseover), window.__bwt.Constants.LENGTH_GLYPH_VISIBLE);
      return;
    };

    // otherwise, try pressing the button
    window.__bwt.LOG_INFO("Glyph sending button: " + __reflect(to_press));
    try {
      // press up and down immediately
      window.__bwt.__BUTTON_MAP[cell_prefix + __reflect(to_press)].press_down();
      window.__bwt.__BUTTON_MAP[cell_prefix + __reflect(to_press)].press_up();

      // display this button as being pressed down for a bit
      $('#' + cell_prefix + __reflect(to_press)).addClass('button_glyphd');

    } catch(err) {
      window.__bwt.LOG_ERROR("Cannot find button for glyph: " + this);
    };

    // wait and then call self on new button set
    setTimeout((function() {
      this.__button_step(cell_prefix, mouseover, index + 1);
    }).bind(this), window.__bwt.Constants.TIME_BETWEEN_GLYPH_BUTTONS);
  };

  /** @brief Sends this glyph to the server as a series of button presses in
   *         rapid succession.
   */
  window.__bwt.Glyph.prototype.send = function send() {
    // For now, only register glyph presses when mousing over a slate
    if (window.__bwt.cur_mouseover !== undefined) {
      var cur_mouseover = window.__bwt.cur_mouseover; // save locally in case cur_mouseover changes
      var cell_prefix = "_slate" + window.__bwt.cur_mouseover.attr('groupnumber') + "_";

      // clear the currently pressed keys
      var i;
      // @TODO: there is probably a more elegant way to do this
      for (i in [1, 2, 3, 4, 5, 6]) {
        $('#' + cell_prefix + __reflect(i)).removeClass('button_glyphd');
      };

      // Press each button. Buttons cannot be pressed simultaneously, so we
      // iterate over each button, only pressing the next button after the
      // previous one has had sufficient time to complete
      this.__button_step(cell_prefix, cur_mouseover, 0);
    };

    /** @brief toString method for glyphs.
     */
    window.__bwt.Glyph.prototype.toString = function toString() {
      return "Glyph {id : \"" + this.id + "\", code : \"" + this.buttons + "\"}";
    };
  };
});
