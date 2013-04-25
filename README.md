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

8.

9.

10.

11.

12.

13.

14.

15.

16.

17.

18.

19.

20.

21.

22.

23.
