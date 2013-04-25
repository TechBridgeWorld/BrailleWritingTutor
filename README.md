![TechBridgeWorld Logo](/source/JettyServer/src/html/assets/tbw_logo.png)

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

Tech Transfer
----------------------------------------------
1. To check out the code, just clone the repository from
<tt>https://github.com/CMU-15-239/BWTEmulator.git</tt> on to any folder of your
choosing.

2. Builds on Windows, Linux, Mac OSX.

3. The emulator is split up into 3 main components:
<h3>Server side</h3>
The server is built in Java, and is mainly used as the point of interaction
between the emulator (in javascript) and the virtual COM ports which interact
with the BWT software.

<h3>COM Port Emulation</h3>
COM Ports are emulated differently on Windows and on Linux. In both cases,
it provides a bridge between the BWT software and our Java server.

<h3>Emulator Frontend</h3>
The front end of the emulator is written in javascript/html, and handles
much of the emulation (bytecode timings, character mappings, etc.)

4.

5.

6.

7.

8. There are no config files

9. N/A

10. Using eclipse:
 * Create New Java Project
 * copy all source code under /source/JettyServer/src to the project's source folder
 * Right click on the project, "Build Path" > "Configure Build Path"
 * * Under the libraries tab: add external jars, add jar files #TODO: <MAKE SURE THESE ARE SOMEWHERE>
 * Right click on project, "refresh"
 * navigate to edu.cmu

11. The emulator can run on Windows and Linux. Though any browser can connect to
  the server once it's running.

12. Our source, and some files that we included are necessary for the project to run,
    but no additional files are needed.

13. N/A

14. N/A

15. Yes, the emulator generates log files.

16. Wherever the emulator runs, a folder called BWT_LOGS is created
    with the logs

17. No external hardware or mobile device.

18. Regarding mobile devices and/or external hardware, indicate if any of the generated binaries from item 10 need to be installed on this device and provide instructions on how to install these binaries.

19. Our Installer installs com0com to use port 1 <-> 7 (1 connected to 7). 
  However after com0com is
  installed, it can be configured to use any port you want(see the com0com readme).
  However, if you change the com port in com0com you will need to edit the source 
  file windowsActionHandler.java (line 27: COMM_PORT_NAME). Our suggestion is that you
  let it use 1 <-> 7

20. Windows: run emulator.jar
  Linux: sudo ./run.sh

21. Our project does not use a mobile device.

22. The timing is handled in javascript. If you scroll or have a lot of other tabs open,
  sometimes the timing can get messes up. If you are experiencing what you think are
	timing issues from the BWT Emulator, we suggest you close other tabs and avoid scrolling.
    
23. Provide a list of future features/capabilities that you would like to see included in the existing tool.
