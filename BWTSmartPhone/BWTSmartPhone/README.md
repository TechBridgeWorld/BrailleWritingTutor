BWT + Smartphone: Student App
===============================================================================
***All the crunchy goodness of the BWT, without the wall socket.***  


The BWT student app replicates the full functionality of the BWT computer app on the Android platform through a Java-based API.



1. Checking out the code
-------------------------------------------------------------------------------

To get the code on your local machine, make sure you have ```git``` installed. If you don't have it, you can get it [here](http://git-scm.com/). Instructions on how to set up git can be found [here](https://help.github.com/articles/set-up-git).

Checking out the code is simple. First, ensure you have access to the 15-239 group on Github. Then:  

    git clone https://github.com/CMU-15-239/BWTSmartphone.git

Or, if you'd prefer to checkout via SSH:

    git@github.com:CMU-15-239/BWTSmartphone.git



2. Available Platforms
-------------------------------------------------------------------------------

This app will build on any platform that supports Java (most importantly, Windows, Mac OSX, and Linux). 


3. Project Facets
-------------------------------------------------------------------------------

Our project consists of three components: a web server, an Android application for teachers, and an Android application for the students. The interaction between the components is simple - the web server configures words that the teacher will access on the Android phone. The teacher will open BWT for Teachers and record his/her voice over the prerecorded Android sounds. The teacher will then plug the BWT into the Android device and BWT for Students will automatically open. Once open the teacher will select the game that will be played by the student. After the game is selected the BWT behaves just like it had prior its introduction to Android. 


4. Required Tools
-------------------------------------------------------------------------------

The app relies on Java, and thus you will need the Java Development Kit. If you don't have the JDK on your system, go [here](http://www.oracle.com/technetwork/java/javase/downloads/index.html) and install JDK v6.

To build the project, you will need the Android SDK. The Android SDK comes pre-bundled with Eclipse in the Android Developer Tools Bundle. The ADT Bundle can be downloaded for all platforms [here](http://developer.android.com/sdk/index.html). Unzip the downloaded file and place the contents wherever you'd like. Run the included Eclipse executable to start the development workspace.

You will also need to download and install the drivers for the phone you will be developing for, particularly if you are on Windows (Mac OSX tends to be more forgiving). These can be found on the manufacturer's website. 


5-7. Additional Libraries Required
-------------------------------------------------------------------------------

All additional libraries are included in the repository. This app requires two external libraries: 

 - **USB Serial for Android** - a driver library for communication with 
    Arduinos and other USB serial hardware on Android, using the Android USB 
    Host API available on Android 3.1+. More information can be found 
    [here](https://code.google.com/p/usb-serial-for-android/).
 
 - **Java Eventing** - an easy-to-use eventing library for Java. More   
    information can be found [here](https://code.google.com/p/javaeventing/). 


8-9. Config Files
-------------------------------------------------------------------------------

The BWT student app has no config files, but there is one setting that needs to be updated. In ```org.techbridgeworld.bwt.student.MyApplication.java```, the address of the web server needs to be set. Modify the ```SERVER_ADDRESS``` value with the address of the webserver. 


10-11. Building the Project
-------------------------------------------------------------------------------

To add the project to Eclipse, launch Eclipse and import the project. Specifically:

1. Go to ```File > Import > General > Existing Projects into Workspace```. 
   Click next.
2. Set the root directory to the directory in which the core resides.
3. Go to ```Projects > Select All```.
4. Uncheck "Copy projects into workspace" and "Add project to working sets"

Next, you will need to plug in your Android smartphone into the computer via USB. Right click the BWTForSmartphone project in the Package Explorer (usually visable on the left-hand side of the screen) and select ```Run As > Android Application```. Your phone should appear in a list of connected devices. Select it, and wait until the Console displays that the app has been successfully installed. 

Naturally, the produced .apk is designed to run exclusively on the Android platform. The packaged .apk file is located in ```BWTForSmartphone\bin``` after compiling.

**Note:** *The website must be launched before the teacher app. The teacher app makes calls to the server's database, and if the website isn't running, the Hangman game will not populate with any words. This is a known bug.*


12-16. Input and Output Files
-------------------------------------------------------------------------------

As specified before, the application attempts to read and import the audio files generated by the BWT Teacher app. Additionally, the app pulls data from the BWT server for hangman game. It is important to have the server running before the student app, or the Hangman game won't populate with a list of words.

The student application does not generate output files. 


17. External Hardware
-------------------------------------------------------------------------------

Once the application is installed on an Android smartphone, the BWT simply needs to be plugged into the smartphone. The app has an intent filter that is registered with the Android phone so that whenever a BWT is connected to the phone through it's usb-in port, the app automatically launches. If the app does not automatically launch, you may need to launch it manually.


18. Installing the App to an Android Phone
-------------------------------------------------------------------------------

Eclipse automatically compiles and installs the app all at once, so there is no need to additionally install the app. 


19. Device Settings
-------------------------------------------------------------------------------

The Android phone must be rooted and, optimally, running stock Android firmware (as opposed to stock vendor firmware), as the student app requires USB Host Mode. For testing purposes, you may find it helpful to enable the phone's accessibility features. In particular, it is suggested that you enable **TalkBack** and **Explore By Touch** under Settings > Accessibility.

Check to see if your smartphone supports USB Host Mode [here](http://usbhost.chainfire.eu/). If it doesn't, you'll need to root your phone. Rooting varies from device to device, but a common, all-in-one tool can be found [here](http://forum.xda-developers.com/showthread.php?t=2230934).

Once your phone is rooted, you'll need to create a file titled ```android.hardware.usb.host.xml```, add the following lines to the file: 

    <permissions>
      <feature name="android.hardware.usb.host"/>
    </permissions>
 
and add it to ```/system/etc/permissions``` on your phone. In that same directory, find a file named ```handlehd_core_hardware.xml``` or ```tablet_core_hardware.xml``` and add the following to the ```<permissions>``` section:

    <feature name="android.hardware.usb.host">
  
Reboot the device. It should now support USB Host Mode. 

**Warning: Rooting can be dangerous, and should be done carefully. Root at your own risk. If you don't feel comfortable rooting, you may need to find a pre-rooted phone.**


20. Running Instructions
-------------------------------------------------------------------------------

When the game launches, the user is prompted to pick a game. Select the game and follow the instructions. The app is designed to function in much the same way that the computer software functions.


21. Computer Interactions
-------------------------------------------------------------------------------

The device makes requests to the server/website. Make sure that this is launched and running before you launch the student app. Again be sure to set the ```SERVER_ADDRESS``` value in ```org.techbridgeworld.bwt.student.MyApplication.java``` to the address of the server before compiling. If the connection was successful, the words entered on the server will appear in the list of words to select from in the Hangman game.


22. Known Bugs
-------------------------------------------------------------------------------

 - The app does not cache a list of words that it last received from the server, so if it can't connect to the server, Hangman does not have any words.

 - You can't interrupt text to speach with input on the BWT. The audio files are delayed. To fix this, the audio files either need to be queued, or a way to end audio files early needs to be implemented. 

 - The app only runs on stock, rooted Android software. This is mostly a fault on the side of the phone's vendors, but ideally a work-around will be developed. Supposedly, Google India will ultimately be providing the phones that will be used. If this is the case, the provided phones can be rooted and flashed with CyanogenMod or something similar. 


23. Where to go from here
-------------------------------------------------------------------------------

 - Tackle the bugs list. They're really not all that bad.

 - Implement student profiles. The idea is that students would somehow login before they used the app, and their progress would be logged by the webserver so that the teachers can monitor their progress.

 - Implement difficulty levels. The level data can (and should) be pulled from the server. The server has the levels partially implemented already.
 
 - Add another game or two.
