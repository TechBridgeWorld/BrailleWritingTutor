5 week plan for BWT Emulator
============================

General notes:
* JAVA (and Swing) could be replaced by c++/any other language
* Regression testing to be done throughout

Week 1
----------------------------
* Come up with 5 week plan (all)
* Get BWT software running (all)
* Start getting virtual COM port working (Dylan / Nikhil)
  * Windows COM port first (Dylan / Nikhil)
* Begin work on JAVA Swing GUI and JAVA backend (Lucas / Zi)
  * Wireframing (Zi)
  * Concrete API for JAVA client (Lucas)

Week 2 - 3
----------------------------
* Get USB virtual COM port running on windows or linux (Dylan / Nikhil)
  * Get computer to recognize a virtual COM port, but no need to interact with
    anything (Dylan / Nikhil)
* Begin work on BWT firmware USB communication (Dylan / Nikhil)
* UI/UX testing of wireframe (Zi)
* Basic JAVA Swing GUI based on user testing results (Zi)
* Work on communication to file descriptors in JAVA (Lucas)
* Begin implementing API (Lucas)

Week 4 - 5
----------------------------
* Finish porting BWT firmware to use virtual I/O (Dylan / Nikhil)
* Get communication working between {firmware, software} and JAVA client
  (Dylan / Nikhil)
* Iterative improvement of GUI and help with API (Zi)
* Complete API implementation (Lucas)
