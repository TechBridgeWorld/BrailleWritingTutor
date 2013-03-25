/** @file util.js
 *  @brief General utility functions.
 *
 *  @author Lucas Ray (ltray@cmu.edu)
 */
$(document).ready(function() {
  "use strict";

  /** @brief Logs at the info level.
   *
   *  @param message The body of the message.
   */
  window.LOG_INFO = function LOG_INFO(message) {
    // only log info messages if __DEBUG is on
    if (window.__DEBUG === true) {
      console.log("Info: " + message);
    };
  };

  /** @brief Logs at the warning level.
   *
   *  @param message The body of the message.
   */
  window.LOG_WARNING = function LOG_WARNING(message) {
    // by default, log all warnings
    console.log("Warning: " + message);
  };

  /** @brief Logs at the error level.
   *
   *  @param message The body of the message.
   */
  window.LOG_ERROR = function LOG_ERROR(message) {
    // throw errors
    throw "Error: " + message;
  };

  /** @brief Logs at the critical level.
   *
   *  @param message The body of the message.
   */
  window.LOG_CRITICAL = function LOG_CRITICAL(message) {
    // throw criticals and alert user with an alert
    window.app_alert("Critical Error", message);
    throw "Critical: " + message;
  };

  /** @brief Wrapper for javascript alert() since default alert() is obtrusive.
   *
   *  @param header The header for this alert.
   *  @param message The body of the message.
   */
  window.app_alert = function app_alert(header, message) {
  };

  /** @brief Sustained app alert. Returns a callback which, when called,
   *         removes the alert.
   *
   *  @param header The header for this alert.
   *  @param message The body of the message.
   */
  window.sustained_app_alert = function sustained_app_alert(header, message) {
    var $alert_header = $("#alert_header");
    var $alert_body = $("#alert_body");
    var $alert_el = $("#alert");

    // add the alert details
    $alert_header.text(header + ": ");
    $alert_body.text(message);
    $alert_el.addClass("active");
    $alert_el.removeClass("unactive");
    $alert_el.slideToggle();

    // return the callback to kill this alert
    return function sustained_app_alert_killer() {
      $alert_el.slideToggle();
      $alert_header.text('');
      $alert_body.text('');
      $alert_el.removeClass("active");
      $alert_el.addClass("unactive");
    };
  };

  /** @brief Timed app alert. Alerts for the specified number of milliseconds.
   *
   *  @param header The header for this alert.
   *  @param message The body of the message.
   *  @param duration The duration of the alert.
   */
  window.timed_app_alert = function timed_app_alert(header, message, duration) {
    // create a sustained app alert
    var callback = window.sustained_app_alert(header, message);

    // kill the sustained app alert after the specified duration
    setTimeout(function() {
      callback();
    }, duration);
  };

  /** @brief Adds an on-hover message about the input dom element.
   *
   *  @param $dom_el jquery object representing the DOM element you want to add
   *                 the message to.
   *  @param header Header of the message.
   *  @param body Body of the message.
   *  @param placement Placement position for this tooltip.
   */
  window.add_info = function add_info($dom_el, header, body, placement) {
    var $message = $('<div>', {
      class: 'tooltip_message'
    });

    $message.append(
        $('<h3>', {
          class: 'tooltip_header'
        }).html(header)
      ).append(
        $('<div>', {
          class: 'tooltip_body'
        }).html(body)
      );

    // add the tooltip
    $dom_el.powerTip({
      'placement': placement,
      mouseOnToPopup: true,
      intentPollInterval: 300,
      closeDelay: 50
    });
    $dom_el.data('powertipjq', $message);
  };
});
