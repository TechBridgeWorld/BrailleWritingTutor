BWT + Smartphone: Web Server
===============================================================================
***What's awesome without a server becomes awesome-er with one.***  


The BWT web server allows for easy modification of the lesson contents for the BWT student app through a simple browser interface.



1. Checking out the code
-------------------------------------------------------------------------------

To get the code on your local machine, make sure you have ```git``` installed. If you don't have it, you can get it [here](http://git-scm.com/). Instructions on how to set up git can be found [here](https://help.github.com/articles/set-up-git).

Checking out the code is simple. First, ensure you have access to the 15-239 group on Github. Then:  

    git clone https://github.com/CMU-15-239/BWTSmartphoneServer.git

Or, if you'd prefer to checkout via SSH:

    git@github.com:CMU-15-239/BWTSmartphoneServer.git


2. Available Platforms
-------------------------------------------------------------------------------

This app will build on any platform that supports Nodejs (most importantly, Windows, Mac OSX, and Linux). The front-end of the site is designed to work in all major browsers.


3. Project Facets
-------------------------------------------------------------------------------

Our project consists of three components: a web server, an Android application for teachers, and an Android application for the students. The interaction between the components is simple - the web server configures words that the teacher will access on the Android phone. The teacher will open BWT for Teachers and record his/her voice over the prerecorded Android sounds. The teacher will then plug the BWT into the Android device and BWT for Students will automatically open. Once open the teacher will select the game that will be played by the student. After the game is selected the BWT behaves just like it had prior its introduction to Android. 


4. Required Tools
-------------------------------------------------------------------------------

This app relies on NodeJS. If you don't have NodeJS installed on your system, go [here](http://nodejs.org/) and click "Install." Then, run the downloaded package. This installs both NodeJS and the Node Package Manger (or NPM for short).

You will additionally need to install MongoDB. Grab the latest production (stable) release [here](http://www.mongodb.org/downloads) and install it. 


5-7. Additional Libraries Required
-------------------------------------------------------------------------------

A number of additional libraries are required to get the server running, including

 - Mongoose
 - Passport
 - Passport-local / Passport-local-mongoose

Installing them is actually quite easy. ```cd``` into the directory you've copied the code into (the directory should have a ```package.json``` file) and run the following:

    npm install

This should get the correct versions of all the required dependencies needed. It may be necessary to run this as an administrator. 

You will next need to **set up MongoDB**. This is fairly straightforward. You need to create the directory for Mongo to store data in. By default, Mongo looks for ```/data/db``` on the drive it's installed in (for example, Mongo looks for ```C://data/db```). You need to create both the ```data``` and the ```data/db``` directories. For specific details, look at Mongo's [installation guide](http://docs.mongodb.org/manual/installation/). 


8-9. Config Files
-------------------------------------------------------------------------------

The project lists its dependencies within ```package.json```. Modify this file if you need to update any of its core dependencies. 

Within ```server.js```, various values, such as **port number** and **administrator login credentials** are set. They are the first items in the file and can be modified as needed.


10-11. Building the Project
-------------------------------------------------------------------------------

On Windows:
 - Run ```mongod.exe``` from your list of installed programs. This starts up MongoDB.
 - Open the NodeJS command prompt. This was installed with Node. From here, ```cd``` into the server's directory and run ```node server.js```. This should start the server and connect to MongoDB.


On Linux/Mac OSX:
 - From a terminal window, run ```mongodb [--dpath path_to_data/db]```, adding the ```--dpath``` flag if you created your ```data/db``` directory somewhere besides your root directory.
 - From a separate terminal window or tab, run ```node server.js``` from the project directory. This attempts to build and run the server.

 If all went well, browsing to ```localhost:[PORT_NUMBER]``` should display a login window. 


12-16. Input and Output Files
-------------------------------------------------------------------------------

Aside from the database's records, the server doesn't read or write any files.


17-19. External Hardware
-------------------------------------------------------------------------------

The BWT Server does not use external hardware, nor does it need to be run on any mobile device.


20. Running Instructions
-------------------------------------------------------------------------------

Assuming the server is running (see steps 10-11 for details), using the server is simple. Browse to ```[server address]:[port number]```. If running locally, server address would be ```localhost```, and if the port number is unaltered, the port should be ```3000```.


21. Computer Interactions
-------------------------------------------------------------------------------


The BWT Student and Teacher apps make requests to this server. Ensure that it is running before using either of the apps. Additionally, make sure that the value of ```SERVER_ADDRESS``` is set to the address of the server in both ```org.techbridgeworld.bwt.teacher.MyApplication.java``` and ```org.techbridgeworld.bwt.student.MyApplication.java```.


22. Known Bugs
-------------------------------------------------------------------------------

 - Deleting every word from the database makes it impossible to add words without restarting the server (which re-populates the database with dummy words), as the ability to create lessons (every word must be in a lesson) isn't yet implemented. 

 - Authenticating a mobile app is done through a secret key rather than a session, which is hard-coded into the app. This isn't very secure.


23. Where to go from here
-------------------------------------------------------------------------------

 - Tackle the bugs list. They're really not that bad. 

 - Implement lessons in full. 

 - Add student profiles. This means students will be able to log on. using the BWT keys as entry values, and access saved content, past position in game, targeted curriculum based on their proficiency and more. This will also allow the developers to track the usage and impact of each of the BWT’s features.

 - Add curriculum building abilities for teachers. With this and the feature above, teachers could administer tests using the BWT (a feature desired in interviews with the teachers)

