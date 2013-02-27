Compiling BWT Software on Windows
================================

1. Getting Mingw-Get
	1.1. Go here: http://sourceforge.net/projects/mingw/files/Installer/mingw-get/mingw-get-0.5-beta-20120426-1/
	1.2. Download a version

2. Installing MinGW with right compiler
	2.1. run `mingw-get install gcc-v3-c++` in command prompt.
	2.2. Either execute that command in the folder where mingw-get is located or add that folder to your path

3. Add the compiler to your path
	3.1. add C:\MinGW\bin to PATH
	3.2. START->right-click on COMPUTER -> Advanced System Settings -> Environment Variables -> System variables -> PATH.
	3.3. Click edit

4. Download boost 1.34.1
	4.1. http://www.boost.org/users/history/version_1_34_1.html

5. Compile the boost thread library
	5.1. open a command prompt
	5.2. navigate to the folder where boost is downloaded
	5.3. run `bjam install toolset=gcc --with-thread` in a command prompt
	5.4. C:\Boost\lib\libboost_thread-mgw34-mt-1_34_1.a should exist

6. Compiling BWT Software
	6.1. move C:\Boost\lib\libboost_thread-mgw34-mt-1_34_1.a  to 
	6.2. [PATH TO BWT SOFTWARE]...\BWT_SourceCode_From_AFS_(Windows)\externals.win\boost\lib
	6.3. open a command prompt and navigate to [PATH TO BWT SOFTWARE]...\BWT_SourceCode_From_AFS_(Windows)
	6.4. run `scons`
	
**DONE**
