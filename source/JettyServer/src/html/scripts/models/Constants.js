/** @file Constants.js
 *  @brief Holds constants for our code.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  window.Constants = function Constants() {};

  // Constants go here
  // Timer related constants
  window.Constants.QUEUE_TIMER_RATE = 100;
  window.Constants.PROCESSOR_TIMER_RATE = 50;
  window.Constants.PROCESSOR_HOLDINGS_TIMER_RATE = 100;

  // Bytecode related constants
  window.Constants.PRESSDOWN_NUM_TO_SEND = 3;

  // UI constants
  window.Constants.LENGTH_GLYPH_VISIBLE = 1000;
});
