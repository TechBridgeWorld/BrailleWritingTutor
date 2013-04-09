Overview
-----------------------------------------------------
controllers/: Standard controllers folder for MVC structure.

Files
-----------------------------------------------------
### load.js (ltray@cmu.edu)
  * Loads key parts of our emulator and other plugins.
    * Adds all slate cells to the DOM.
    * Attaches event listeners to all buttons in the DOM.
  * TODO: Some of the code in this file is a bit messy, or could be
          abstracted into model initialization.

### util.js (ltray@cmu.edu)
  * Defines general methods useful for our app.
    * <tt>LOG\_\*()</tt> controls logging.
    * <tt>app\_alert()</tt> and associated methods convey information to the user.
    * <tt>add\_info()</tt> is used to control tooltip information.

### recording.js (ziwang@cmu.edu)
  * TODO: write this

### scriptsLoader.js (ziwang@cmu.edu)
  * TODO: write this
