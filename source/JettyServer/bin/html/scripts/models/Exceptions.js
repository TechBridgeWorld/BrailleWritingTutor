/** @file Exceptions.js
 *  @brief Describes all exceptions that can be thrown by our code.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  "use strict";

  /** @brief InvalidGlyphException exception is thrown if the user creates an
   *         invalid Glyph object.
   */
  window.InvalidGlyphException = window.extend(Error, {
    constructor: function constructor(message) {
      this.name = "InvalidGlyph";
      this.message = message || "An invalid glyph was created.";
    },
  });
});
