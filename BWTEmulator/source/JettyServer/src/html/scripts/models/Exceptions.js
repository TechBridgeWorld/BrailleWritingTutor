/** @file Exceptions.js
 *  @brief Describes all exceptions that can be thrown by our code.
 *
 *  @TODO: this is unused, but could be useful in the future. Currently does not work
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  "use strict";

  /** @brief InvalidGlyphException exception is thrown if the user creates an
   *         invalid Glyph object.
   */
  window.__bwtInvalidGlyphException = window.__bwt.extend(Error, {
    constructor: function constructor(message) {
      this.name = "InvalidGlyph";
      this.message = message || "An invalid glyph was created.";
    },
  });
});
