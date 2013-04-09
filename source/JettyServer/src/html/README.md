![TechBridgeWorld Logo](/assets/tbw_logo.png)

BWT Emulator
==============================================

Authors:
* Dylan Swen (dswen@andrew.cmu.edu)
* Lucas Ray (ltray@andrew.cmu.edu)
* Nikhil Bikhchandani (nbikcha@andrew.cmu.edu)
* Zi Wang (ziw@andrew.cmu.edu)

Developed for and with help by [TechBridgeWorld](http://www.techbridgeworld.org/).

Overview
----------------------------------------------
The Braille Writing Tutor (BWT) is a device used to help teach the blind and visually
impaired how to write in braille. See [TechBridgeWorld's website](http://www.techbridgeworld.org/brailletutor/about.html)
for more information.

The BWT Emulator is used to emulate the byte codes sent by the BWT so that developers
can create new code for the BWT without owning the actual hardware. This increases the
number of potential developers from the limited set that have access to a BWT to any
one motivated enough to undertake the project. This, in combination with awareness-raising
efforts and new projects and games being produced by students in 15-239, will hopefully
result in a number of new games and applications being developed to help the blind and
visually impaired.

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
