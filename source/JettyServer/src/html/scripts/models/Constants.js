/** @file Constants.js
 *  @brief Holds constants for our code.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  window.__bwt.Constants = function Constants() {};

  // Constants go here
  // Timer related constants
  window.__bwt.Constants.QUEUE_TIMER_RATE = 100;
  window.__bwt.Constants.PROCESSOR_TIMER_RATE = 50;
  window.__bwt.Constants.PROCESSOR_HOLDINGS_TIMER_RATE = 100;
  window.__bwt.Constants.TIME_BETWEEN_GLYPH_BUTTONS = 500; // this number obtained through trial

  // Bytecode related constants
  window.__bwt.Constants.PRESSDOWN_NUM_TO_SEND = 3;

  // UI constants
  window.__bwt.Constants.LENGTH_GLYPH_VISIBLE = 1000;
});
