Overview
-----------------------------------------------------
assets/: All assets go here. Backgrounds, images, etc.

<hr />

Files
=====================================================
### input\_mapping.js:
  * Contains the mapping from user input to bytecode produced

  * Encoding as follows:
    * _l, _r  : Main left and right buttons, respectively
    * _jx     : Button x of the jumbo buttons
    * _sx_y   : Button y of slate x, where slates are numbered left-to-right,
               top-to-bottom (e.g. 1  2  3  ... 16
                                   17 18 19 ... 32)
  * Corresponds to the ids of the DOM elements representing the buttons, so
   one can get the code for button "\_l", for example, via:

```js
var code = window.input_mapping[$("_l").attr('id')];
```

### *.png, *.jpg, *.gif
  * Various images used for displaying parts of the UI. Fairly self explanatory.
