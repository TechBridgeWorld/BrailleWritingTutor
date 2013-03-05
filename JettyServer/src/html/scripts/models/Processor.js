/** @file Processor.js
 *  @brief Main processor for our app. Handles the bytecode queue.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  /** @brief Constructor for a Processor.
   */
  window.Processor = function Processor(options) {
    this.__PROCESSING_QUEUE = [];
    this.__BUTTONS_HELD = [];

    // TODO: make less silent
    this.success = function(data) {
      console.log(data);
    };
    this.error = function(data) {
      console.log(data);
    };
  };

  /** @brief Runs the processor. Starts the main loop which occasionally
   *         flushes the processing queue and sends to server. Also starts
   *         the add_holdings loop to add buttons being held.
   */
  window.Processor.prototype.run = function run() {
    this.__holdings_step();
    this.__step();
  };

  /** @brief Represents 1 step in the main event loop which flushes the
   *         processing queue.
   */
  window.Processor.prototype.__step = function __step() {
    // flush the processing queue
    this.__flush();

    // sleep until the next timer event
    setTimeout((function() {
      this.__step();
    }).bind(this), window.Constants.PROCESSOR_TIMER_RATE);
  };

  /** @brief Represents 1 step in the secondary loop which adds all buttons
   *         being held to the main processing queue.
   */
  window.Processor.prototype.__holdings_step = function __holding_step() {
    // add buttons being held to the queue
    this.__add_holdings();

    // sleep until next holdings timer event
    setTimeout((function() {
      this.__holdings_step();
    }).bind(this), window.Constants.PROCESSOR_HOLDINGS_TIMER_RATE);
  };

  /** @brief Adds the input bytecode the specified number of times to the
   *         sending queue.
   */
  window.Processor.prototype.add_code = function add_code(code, num_to_send) {
    this.__PROCESSING_QUEUE.push(code);

    // if there's more to send, fire up a setTimeout event to send the next
    // code
    if (num_to_send > 1) {
      setTimeout((function() {
        this.add_code(code, num_to_send - 1);
      }).bind(this), window.Constants.QUEUE_TIMER_RATE);
    };
  };

  /** @brief Adds a button to the processor's holding queue.
   */
  window.Processor.prototype.add_hold = function add_hold(button) {
    // simply add to the queue
    this.__BUTTONS_HELD.push(button);
  };

  /** @brief Removes a button from the processor's holding queue.
   */
  window.Processor.prototype.remove_hold = function remove_hold(button) {
    var index = this.__BUTTONS_HELD.indexOf(button);
    // if the input button is in our map, remove it with a splice
    if (index >= 0) {
      this.__BUTTONS_HELD.splice(index, 1);
    };
  };

  /** @brief Flushes the processing queue by sending all codes to the
   *         server.
   */
  window.Processor.prototype.__flush = function __flush() {
    // TODO: should we split this up into n different ajax requests?
    if (this.__PROCESSING_QUEUE.length > 0) {
      var all_codes = this.__PROCESSING_QUEUE.join('');
      this.__PROCESSING_QUEUE = [];
      this.send(all_codes);
    };
  };

  /** @brief Sends the input bytecode to the server.
   */
  window.Processor.prototype.send = function send(code) {
    $.ajax({
      url: '/sendBytes.do?code=' + code,
      type: 'GET',
      success: this.success,
      error: this.error
    });
  };

  /** @brief Adds all buttons currently being held to the processing
   *         bytecode queue.
   */
  window.Processor.prototype.__add_holdings = function __add_holdings() {
    var i;

    // iterate through all buttons being held
    for (i = 0; i < this.__BUTTONS_HELD.length; i++) {
      var this_button = this.__BUTTONS_HELD[i];
      this.add_code(this_button.code, 1);
    };
  };
});
