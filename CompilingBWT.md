# Compiling BWT Software on Windows

#### 1. Getting MinGW with GCC v3.4.5
1. Download mingw-get: http://sourceforge.net/projects/mingw/files/Installer/mingw-get/mingw-get-0.5-beta-20120426-1/  
2. Run `mingw-get install gcc-v3-c++` in a command prompt
3. Add C:\MinGW\bin to PATH

#### 2. Download and Compile Boost 1.34.1
1. Download from: http://www.boost.org/users/history/version_1_34_1.html
1. Open a command prompt and navigate to the boost folder
3. Run `bjam install toolset=gcc --with-thread` in a command prompt
4. C:\Boost\lib\libboost_thread-mgw34-mt-1_34_1.a should exist

#### 3. Compiling BWT Software
1. Move C:\Boost\lib\libboost_thread-mgw34-mt-1_34_1.a  to ...\BWT_SourceCode_From_AFS_(Windows)\externals.win\boost\lib
3. Open a command prompt and navigate to ...\BWT_SourceCode_From_AFS_(Windows)
4. Run `scons`
	
**DONE**
