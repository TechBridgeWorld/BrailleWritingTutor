BWT + Smartphone: Teacher App
===============================================================================
***Who doesn't love to customize?***  


The BWT teacher app allows users to record sounds for the games implemented in the BWT student app, allowing for a more tailored learning experience.



1. Checking out the code
-------------------------------------------------------------------------------

To get the code on your local machine, make sure you have ```git``` installed. If you don't have it, you can get it [here](http://git-scm.com/). Instructions on how to set up git can be found [here](https://help.github.com/articles/set-up-git).

Checking out the code is simple. First, ensure you have access to the 15-239 group on Github. Then:  

    git clone https://github.com/CMU-15-239/BWTSmartphoneTeacher.git

Or, if you'd prefer to checkout via SSH:

    git@github.com:CMU-15-239/BWTSmartphoneTeacher.git



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

This BWT teacher app requires no additional libraries.


8-9. Config Files
-------------------------------------------------------------------------------

The BWT teacher app has no config files, but there is one setting that needs to be updated. In ```org.techbridgeworld.bwt.teacher.MyApplication.java```, the address of the web server needs to be set. Modify the ```SERVER_ADDRESS``` value with the address of the webserver. 


10-11. Building the Project
-------------------------------------------------------------------------------

To add the project to Eclipse, launch Eclipse and import the project. Specifically:

1. Go to ```File > Import > General > Existing Projects into Workspace```. 
   Click next.
2. Set the root directory to the directory in which the core resides.
3. Go to ```Projects > Select All```.
4. Uncheck "Copy projects into workspace" and "Add project to working sets"

Next, you will need to plug in your Android smartphone into the computer via USB. Right click the BWTTeacher project in the Package Explorer (usually visable on the left-hand side of the screen) and select ```Run As > Android Application```. Your phone should appear in a list of connected devices. Select it, and wait until the Console displays that the app has been successfully installed. 

Naturally, the produced .apk is designed to run exclusively on the Android platform. The packaged .apk file is located in ```BWTTeacher\bin``` after compiling.

***Note:*** *The website must be launched before the teacher app. The teacher app makes calls to the server's database, and if the website isn't running, the Hangman game will not populate with any words. This is a known bug.*


12-17. Input and Output Files
-------------------------------------------------------------------------------

The teacher app does not require input files or generate output files, nor does it require external hardware.


18. Installing the App to an Android Phone
-------------------------------------------------------------------------------

Eclipse automatically compiles and installs the app all at once, so there is no need to additionally install the app. 


19. Device Settings
-------------------------------------------------------------------------------

For testing purposes, you may find it helpful to enable the phone's accessibility features. In particular, it is suggested that you enable **TalkBack** and **Explore By Touch** under Settings > Accessibility. 


20. Running Instructions
-------------------------------------------------------------------------------

When the game launches, the user is prompted to select a game, then a category of sounds, and then a specific sound. If **TalkBack** and **Explore by Touch** are enabled, then the user will be able to hover over a sound to hear its current recording. Upon selecting a sound, the user is given instructions to create a new recording.


21. Computer Interactions
-------------------------------------------------------------------------------

The device makes requests to the server/website. Make sure that this is launched and running before you launch the student app. Again be sure to set the ```SERVER_ADDRESS``` value in ```org.techbridgeworld.bwt.teacher.MyApplication.java``` to the address of the server before compiling. If the connection was successful, the words entered on the server will appear in the list of words under Hangman > Words.


22. Known Bugs
-------------------------------------------------------------------------------

 - The app does not cache a list of words that it last received from the server, so if it can't connect to the server, Hangman does not have any words.

 - When a new game is added to the student app, the teacher app must be manually updated with the game, its category of sounds, and its specific sounds. 

 - All persistent audio files (for example, the animal sounds in Animal Game) must be in the res > raw folder of the app. If new persistent audio files are added, the teacher app and the student app must be uninstalled from and reinstalled on the device.  

23. Where to go from here
-------------------------------------------------------------------------------

 - Tackle the bugs list. They're really not all that bad.

 - Implement difficulty levels. The level data can (and should) be pulled from the server. The server has the levels partially implemented already. 

 - Make the process of manually updating the teacher app with new game information less tedious. 
