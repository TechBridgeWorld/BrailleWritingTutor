Overview
-----------------------------------------------------
models/: Standard models folder for MVC structure.

Files
-----------------------------------------------------
### Button.js (ltray@cmu.edu)
  * Represents a button of the emulator.
  * <tt>press\_down()</tt> represents a press (not hold), so the button is no longer
    considered being pressed on a <tt>press\_up()</tt> event.
  * <tt>press\_up()<tt> represents stopping pressing a button.
  * <tt>hold\_down()</tt> is essentially a <tt>press\_down()</tt> event that ignores the
    subsequent <tt>press\_up()</tt> event, so the button remains held.

### Constants.js (ltray@cmu.edu)
  * Global object window.Constants used to hold all constants we might need.

### Glyph.js (ltray@cmu.edu)
  * Represents a glyph.
  * Accessed primarily through its <tt>send()</tt>, which sends that glyph's
    bytecodes to the processing queue.
  * Rate limits to prevent bytecode overflow on the software. It was determined
    that a delay of 400ms between presses was too fast, so roughly 500ms seems
    to be the fastest allowable delay rate (defined in Constants.js).

### Processor.js (ltray@cmu.edu)
  * The main gateway between client code button presses and server.
  * Button presses don't send ajax requests to our server directly, but rather
    send them to the app's processor (window._Processor), as defined in Processor.js.
  * Rate-limits requests sent to the server (currently processes requests every
    50ms, as defined in Constants.js) to prevent overflow.
  * Handles button rate-limiting and for button hold events, to emulate the rate
    at which bytecodes are sent from the BWT.
