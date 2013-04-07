/** @file Button.js
 *  @brief Class that represents a button. Used to send an AJAX request to
 *         our local server.
 *
 *  Buttons attempt to emulate the hardware as much as possible. On button
 *  press down, the button's code is sent to the server, and this button
 *  is added to the __BUTTONS_HELD queue. A setInterval loop iterates over
 *  all buttons currently being held and sends their codes every
 *  QUEUE_TIMER_RATE ms. On button up, another code is sent to the server. This
 *  allows certain edge cases to arise that currently exist in the hardware. For
 *  example, pressing the slate very quickly on the physical hardware doesn't
 *  register on the existing software if the slate press is so short that only 2
 *  bytecodes are sent. This is emulated in the emulator by clicking such that
 *  the setInterval loop fails to send the bytecode before the button up event.
 *
 *  Usage: var btn = new Button({
 *           'code': 'a n',
 *           'success': function(data) { console.log(data); },
 *           'failure': function(data) { console.log(data); }
 *         });
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */

$(document).ready(function() {
  "use strict";

  /** @brief Constructor for a request.
   *
   *  @param options Options for this button. These include:
   *           -code: Byte code for the request (e.g. "b n")
   *           -$dom_el: The DOM element representing this button
   */
  window.Button = function Button(options) {
    // the software expects 3 button presses when you click on slate cells, so
    // triple the bytecode
    this.code = options.code;
    this.$dom_el = options.dom_el;
    this.__holding = false;

    // @TODO: this is a bit of a hack, but gets the job done. Exists to fix the
    // "press and drag off" bug that keeps adding the same button to the
    // processing queue
    this.received_up = true;
  };

  /** @brief Represents a button press down. On press down, send the byte code
   *         3 times.
   */
  window.Button.prototype.press_down = function press_down() {
    var prev_status = this.__holding;

    // if we never received the press_up event, that means the user dragged the
    // mouse off the button
    if (this.received_up === false) {
      // if we never received up, we count this as a hold
      this.__holding = true;
    };

    // if not holding down, start holding
    if (this.__holding === false) {
      this.$dom_el.addClass('active');
      window._Processor.add_code(this.code, window.Constants.PRESSDOWN_NUM_TO_SEND);
      window._Processor.add_hold(this);
      if(window.recording){
          appendToRecordingQueue(this,__KEY_DOWN);
      }
    }

    // if we were holding before, stop holding now
    this.__holding = false;
    this.received_up = false;
    return prev_status;
  };

  /** @brief Represents a button press up. Removes the button from the holdings
   *         queue.
   */
  window.Button.prototype.press_up = function press_up() {
    // mark that we received the up event
    this.received_up = true;
    if (this.__holding === false) {
      // remove the hold on press_up
      window._Processor.remove_hold(this);
      this.$dom_el.removeClass('active');
      if(window.recording){
          appendToRecordingQueue(this,__KEY_UP);
      }
    }

    // if holding (from a shift), ignore the press_up event
  };

  /** @brief Represents a button hold. Same as button press but press_up has
   *         no effect.
   */
  window.Button.prototype.hold_down = function hold_down() {
    var prev_status = this.press_down();
    this.__holding = !prev_status;
  };
});
