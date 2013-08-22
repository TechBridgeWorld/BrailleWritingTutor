# Compiling BWT Software on Windows

#### 1. Getting MinGW with GCC v3.4.5
1. Download and extract mingw-get: http://sourceforge.net/projects/mingw/files/Installer/mingw-get/mingw-get-0.5-beta-20120426-1/ 
2. Navigate to \\ *path_to_MinGW* .  This is the folder you just extracted
3. Run `mingw-get install gcc-v3-c++` in a command prompt
3. Add \\ *path_to_MinGW*\bin to PATH
    + Run `set PATH=%PATH%;\path_to_MinGW\bin` in a command prompt

#### 2. Download and Compile Boost 1.34.1
1. Download and extract boost from: http://www.boost.org/users/history/version_1_34_1.html
2. Edit \\ *path_to_boost*\boost_1_34_1\tools\jam\src\build.bat
     + Change line 186:`     if not errorlevel 1 (` to `     if errorlevel 1 (`
3. Open a command prompt and navigate to \\ *path_to_boost*\boost_1_34_1\tools\jam\src
4. Run `.\build.bat gcc`
       + \\ *path_to_boost*\boost_1_34_1\tools\jam\src\bin.ntx86\bjam.exe should have been created
5. Move \\ *path_to_boost*\boost_1_34_1\tools\jam\src\bin.ntx86\bjam.exe to \\ *path_to_boost*\boost_1_34_1\
6. Open a command prompt and navigate to \\ *path_to_boost*\boost_1_34_1\
3. Run `bjam install toolset=gcc --with-thread`
       + C:\Boost\lib\libboost_thread-mgw34-mt-1_34_1.a should have been created

#### 2. Install Scons (much easier than MinGW or Boost)
1. Install a python (for example: http://code.google.com/p/pythonxy/)
2. Install Scons: http://www.scons.org/

#### 3. Compiling BWT Software
1. Move C:\Boost\lib\libboost_thread-mgw34-mt-1_34_1.a  to ...\BWT_SourceCode_From_AFS_(Windows)\externals.win\boost\lib
3. Open a command prompt and navigate to ...\BWT_SourceCode_From_AFS_(Windows)
4. Run `scons`
	
**DONE**
