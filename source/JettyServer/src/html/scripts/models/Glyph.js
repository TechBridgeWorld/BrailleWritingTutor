/** @file Glyph.js
 *  @brief Object representing a single glyph.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  /** @brief Constructor for a glyph.
   *
   *  @param options Options for this glyph. These include:
   *           -id: Identifier for this glyph (e.g. "a" to represent
   *                the letter "a")
   *           -code: comma-separated list of buttons to be pressed for this
   *                  glyph (e.g. "1,2" for "b", or "3,1,5" for "o")
   */
  window.Glyph = function Glyph(options) {
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
      window.LOG_ERROR("Invalid glyph: " + this);
    };
  };

  /** @brief Sends this glyph to the server as a series of button presses in
   *         rapid succession.
   */
  window.Glyph.prototype.send = function send() {
    if (window.mouse_cell !== undefined) {
      var this_mouseover = window.cur_mouseover;
      // if currently hovering over a cell, use that cell. Defaults to
      // the jumbo cell
      var cell_prefix = window.mouse_cell;

      // clear the currently pressed keys
      var i;
      for (i in [1, 2, 3, 4, 5, 6]) {
        $('#' + cell_prefix + i).removeClass('button_glyphd');
        console.log("removed " + cell_prefix + i);
      };

      this.buttons.map((function(el) {
        try {
          // press up and down immediately
          __BUTTON_MAP[cell_prefix + el].press_down();
          __BUTTON_MAP[cell_prefix + el].press_up();

          // display this button as being pressed down for a bit
          $('#' + cell_prefix + el).addClass('button_glyphd');

        } catch(err) {
          window.LOG_ERROR("Cannot find button for glyph: " + this);
        };
      }).bind(this));

      // display that the user typed a glyph into this cell
      window.cur_mouseover.find('.glyph_display').addClass('active').html(this.id);
      window.cur_mouseover.addClass('glyphd');
      var self = this;
      var cur_mousecell = window.mouse_cell;
      setTimeout((function() {
        this.removeClass('glyphd');
        self.buttons.map(function(el) {
          var cell_prefix = cur_mousecell || "_jumbo";
          $('#' + cell_prefix + el).removeClass('button_glyphd');
          (this_mouseover).find('.glyph_display').removeClass('active').html('');
        });
      }).bind(window.cur_mouseover), window.Constants.LENGTH_GLYPH_VISIBLE);
    };

    /** @brief toString method for glyphs.
     */
    window.Glyph.prototype.toString = function toString() {
      return "Glyph {id : \"" + this.id + "\", code : \"" + this.buttons + "\"}";
    };
  };
});
