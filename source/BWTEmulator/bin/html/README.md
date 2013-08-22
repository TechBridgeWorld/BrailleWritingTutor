![TechBridgeWorld Logo](/assets/tbw_logo.png)

BWT Emulator Frontend
==============================================

Overview
----------------------------------------------
The bulk of our emulation lies in the javascript application that you see here.
Much of the timing and button press logic lives in <tt>models/Button.js</tt> and
<tt>models/Processor.js</tt>. See documentation of those two files for more
specifics.

Code Layout
----------------------------------------------
See specific folders for more detail.

assets/
  * Assets (images, mappings, etc.).

css/
  * All css and scss used for the app.

  libs/
    * css files used in library files.

scripts/
  * All scripts we use in our app.

  libs/
    * Library scripts (jquery, etc.)

  models/
    * Standard models folder for MVC structure.

  views/
    * Standard views folder for MVC structure.

  controllers/
    * Standard controllers folder for MVC structure.

index.html
  * Main page of our app.
