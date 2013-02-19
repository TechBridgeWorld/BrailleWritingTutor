/** @file Button.js
 *  @brief Class that represents a button. Used to send an AJAX request to
 *         our local server.
 *
 *  Usage: var btn = new Button({
 *           'code': 'a n',
 *           'success': function(data) { console.log(data); },
 *           'failure': function(data) { console.log(data); }
 *         });
 *         btn.press();
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  var holding = undefined;
  /** @brief Constructor for a request.
   *
   *  @param options Options for the request. These include:
   *           -code: Byte code for the request (e.g. "b n")
   *           -success: Callback to be used on success.
   *           -failure: Callback to be used on failure.
   *           -TODO: more?
   */
  window.Button = function(options) {
    // the software expects 3 button presses when you click on slate cells, so
    // triple the bytecode
    this.code = options.code;

    this.success = options.success;
    this.failure = options.failure;
    this.hold_timeout = undefined;
  };

  /** @brief Represents a button press. Makes the request to our server.
   */
  window.Button.prototype.press = function() {
    if ((this !== holding) && (holding !== undefined)) {
      // if a button is currently being held, stop their hold timer and
      // interleave this code call with the holding call
      holding.hold_off(false);

      var to_send = this.code + holding.code +
                    this.code + holding.code +
                    this.code + holding.code;

      console.log("bytecode: \"" + to_send + "\"");
      $.ajax({
        url: '/sendBytes.do?code=' + to_send,
        type: 'GET',
        success: this.success,
        error: this.failure
      });

      holding.hold_on();
    } else {
      // if not holding, just send the bytecode 3 times as 1 request.
      var to_send = this.code + this.code + this.code;

      console.log("bytecode: \"" + to_send + "\"");
      $.ajax({
        url: '/sendBytes.do?code=' + to_send,
        type: 'GET',
        success: this.success,
        error: this.failure
      });
    };
  };

  /** @brief Some buttons require toggling. hold_on() toggles the button on.
   */
  window.Button.prototype.hold_on = function() {
    holding = this;
    this.press();

    // make a request every 100 milliseconds
    this.hold_timeout = setTimeout((function() {
      this.hold_on();
    }).bind(this), 100);
  };

  /** @brief Toggles off the hold_on function above.
   *
   *  @param clear True if you want holding cleared, false otherwise.
   */
  window.Button.prototype.hold_off = function(clear) {
    if (this.hold_timeout !== undefined) {
      window.clearTimeout(this.hold_timeout);
      this.hold_timeout = undefined;
    };

    if (clear === true) {
      holding = undefined;
    };
  };

  /** @brief Toggles this button's holding.
   */
  window.Button.prototype.toggle_hold = function() {
    if (this.hold_timeout === undefined) {
      this.hold_on();
    } else {
      this.hold_off(true);
    };
  };
});
