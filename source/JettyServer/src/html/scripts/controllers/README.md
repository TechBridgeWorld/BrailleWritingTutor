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
  * Handles the recorder plugin of the emulator.
  	* Intiatialize plugin buttons and attach action handlers.
  	* Update the html dropdown after each recording .
  * TODO: Combine the recording plugin and the scripting plugin.
 

### scriptsLoader.js (ziwang@cmu.edu)
  * Handles the scripting plugin of the emulator.
  	* Attach button action handlers to plugin buttons.
  	* Load all .bwt scripts in the ./BWT_SCRIPTS folder.
  	* Process the compiled script sent from server.
  	* Run compiled scripts.
  * TODO: Combine the recording plugin and the scripting plugin.
