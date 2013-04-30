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
<div>
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
</div>

4. To compile, you will need:
  * Java SDK 1.7

5. The following external libraries are required to build the emulator. Note
that all required libraries and external softwares are included in the
repository.
<div>
  <h3>Java Libraries</h3>
    * [NRJavaSerial 3.8.8](https://code.google.com/p/nrjavaserial/)
    * [Jetty Server 9.0.2](http://www.eclipse.org/jetty/)
    * [SLF4J 1.7.2](http://www.slf4j.org/)
    * [Logback 1.0.9](http://logback.qos.ch/)
    * [GSON 2.2.2](https://code.google.com/p/google-gson/)
    * [Apache Commons Lang3 3.1](http://commons.apache.org/proper/commons-lang/)

  <h3>External Programs</h3>
    * [com0com 2.2.2.0](http://pete.akeo.ie/2011/07/com0com-signed-drivers.html)
</div>

6. The tools can be acquired from the links above, or are also included in the repository.

7. com0com can be installed anywhere on your computer. To use the emulator as-is, you must connect
  ports 1 and 7 (see <a href="#brief-introduction-to-com0com-commands">com0com intro</a> for more on how to do this).
  All Java libraries need only be added to your build path during compilation, and do not require installation.

8. There are no config files.

9. N/A

10. To build and develop the project using Eclipse: 
  * Only the following eclipse versions have been tested
    <p>-Eclipse Classic. Version: 4.2.2</p>
    <p>-Eclipse IDE for Java Developers. Version: Juno Service Release 1</p>
    <p>-Eclipse IDE for Java EE Developers. Version: Indigo Service Release 2</p>
  * Download <tt>/bin/archives/emulator.zip</tt> from repository
  * Download <tt>/bin/archives/libs.zip</tt> from repository
  * Extract <tt>libs.zip</tt>
  * Open eclipse and go to File -> Import
  * Select General -> Existing Projects into Workspace
  * Choose 'Select archive file' and click 'browse'
  * Select<tt>emulator.zip</tt> just downloaded
  * Under 'Projects', make sure 'JettyServer (JettyServer)' is checked. Click 'Finish'
  * Wait for eclipse to build the workspace
  * Right click 'JettyServer' project and go to Build Path -> Configure Build Path
  * Go to 'Libraries' tab, click 'Add External JARs'
  * Select all .jar files extracted from <tt>libs.zip</tt>, click OK and wait eclipse to build the workspace
  * Nagivate to package <tt>edu.cmu.ui</tt> (JettyServer -> src > edu.cmu.ui)and run <tt>ControllerUI.java</tt> as 'Java Application'
  * Right click 'JettyServer' project and click Refresh to make sure eclipse project explorer shows the most up-to-date log files, which are saved under <tt>/BWT_LOGS</tt>

  To build and develop the project using other java IDE
  * Import all source files under <tt>/source/JettyServer/src</tt> and <tt>/source/JettyServer/resources</tt>
  * Make suer all .jar files in <tt>libs.zip</tt> are in the project's build path
  * Add <tt>/source/JettyServer/resources</tt> as a class folder so logback can get its configuration file
  * Run <tt>ControllerUI.java</tt> under package <tt>edu.cmu.ui</tt>

11. The emulator can run on Windows and Linux, though any browser can connect to
  the server once it's running.

12. Our source, and some files that we included are necessary for the project to run,
    but no additional files are needed.

13. N/A

14. N/A

15. Yes, the emulator generates log files. The configuration of logging is defined by <tt>/source/JettyServer/src/resources/logback.xml</tt>. Loggers are exposed to java server through <tt>/source/JettyServer/src/edu/cmu/logger/EmulatorLogger.java</tt>

16. Wherever the emulator runs, a folder called <tt>BWT_LOGS</tt> is created
    at the top level, and contains the logs.

17. No external hardware or mobile device.

18. N/A

19. Our Installer installs com0com to use port 1 <-> 7 (1 connected to 7). 
  However after com0com is
  installed, it can be configured to use any port you want (see below).
  If you change the com port in com0com you will need to edit the source 
  file windowsActionHandler.java (line 27: COMM_PORT_NAME). Our suggestion is that you
  let it use 1 <-> 7.

  <h3>Brief introduction to com0com commands:</h3>
  
    <tt>list</tt> : lists the virtual ports created.
  
    <tt>install - -</tt> : install com0com software.
  
    <tt>install PortName=COM1 PortName=COM7</tt> : create a virtual com port pair using COM1 and COM7.
  
    <tt>remove 0</tt> : removes the COM port pair with ID 0. Use <tt>list</tt> to see COM port pair IDs.

20. On Windows: Run <tt>emulator.jar</tt>

  On Linux: Execute <tt>sudo ./run.sh</tt>

21. Our project does not use a mobile device.

22. The timing is handled in javascript. If you scroll or have a lot of other tabs open,
  sometimes the timing can get messed up. If you are experiencing what you think are
  timing issues from the BWT Emulator, we suggest you close other tabs and avoid scrolling. The timing
  logic should eventually be run from the Java server to better guarantee specific timings.
    
23. As noted in 22, we would like to see the timing logic handled by the Java server, and not
  by the javascript frontend.
