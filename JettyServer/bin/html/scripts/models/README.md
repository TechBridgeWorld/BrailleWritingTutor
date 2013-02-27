Overview
-----------------------------------------------------
models/: Standard models folder for MVC structure.

Files
-----------------------------------------------------
### Button.js
  * Represents a button of the emulator.
  * Each button keeps track of it's code, it's on success and on failure methods,
    and a <tt>setTimeout</tt> ID in case it is being held down.
  * <tt>press()</tt> contains much of the logic, and sends the button's code to
    our server. Interleaves the code given by the "\_l"/"\_r" buttons if they are
    being held down.
