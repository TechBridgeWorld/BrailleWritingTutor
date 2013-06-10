import sys

def isLinux():
  import platform
  return  "Linux" in platform.system()

if isLinux():
  BOOST_HEADERS = "/usr/include/boost" # path to your Boost installation
  BOOST_LIB = "/usr/lib"
  SDL_HEADERS = "/usr/include/SDL"
  SDL_LIB = "/usr/lib"
else:
  # Gary: Original paths when I downloaded the code from AFS
  #BOOST_HEADERS = "C:/Program Files/boost/boost_1_34_1" # path to your Boost installation
  #BOOST_HEADERS = "C:/boost/boost_1_34_1" # path to your Boost installation

  # Path of the built boost libraries on my machine JimmyOlsen Boost version 1_53_0
  # After building Boost it said to inlcude these paths in my project if using 1.53.0
  #BOOST_HEADERS = "C:/boost/boost_1_53_0"

  # Path of the built boost libraries on my machine JimmyOlsen Boost version 1_49_0
  # After building Boost it said to inlcude these paths in my project if using 1.49.0
  #BOOST_HEADERS = "C:/boost/boost_1_49_0"
  BOOST_HEADERS = "C:/boost/boost_1_53_0"

  # User previously, but did not work 
  #BOOST_LIB     = "C:\Program Files (x86)\boost\boost_1_53_0\stage\lib"
  #BOOST_LIB     = "C:\Program Files (x86)\boost\boost_1_49_0\stage\lib"

  BOOST_LIB      = "./externals.win/boost/lib"	# I copied the binary thread library that I built into this directory

  #BOOST_HEADERS = "C:/boost/boost_1_53_0" #path to your Boost installation
  #BOOST_HEADERS = "./externals.win/boost/include" #path to your Boost installation

  SDL_HEADERS = "./externals.win/SDL/include" #Note: Make sure you have SDL_mixer.h here. Recall that SDL_Mixer is a separate project so you have to get it separately
  SDL_LIB = "./externals.win/SDL/lib"

colors = {}
colors['cyan']   = '\033[96m'
colors['purple'] = '\033[95m'
colors['blue']   = '\033[94m'
colors['green']  = '\033[92m'
colors['yellow'] = '\033[93m'
colors['red']    = '\033[91m'
colors['end']    = '\033[0m'

#If the output is not a terminal, remove the colors
if not sys.stdout.isatty():
   for key, value in colors.iteritems():
      colors[key] = ''

#If using windows, remove the colors
if not isLinux():
   for key, value in colors.iteritems():
      colors[key] = ''

compile_source_message = '%s\nCompiling %s==> %s$SOURCE%s' % \
   (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

compile_shared_source_message = '%s\nCompiling shared %s==> %s$SOURCE%s' % \
   (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

link_program_message = '%s\nLinking Program %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])

link_library_message = '%s\nLinking Static Library %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])

ranlib_library_message = '%s\nRanlib Library %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])

link_shared_library_message = '%s\nLinking Shared Library %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])

java_compile_source_message = '%s\nCompiling %s==> %s$SOURCE%s' % \
   (colors['blue'], colors['purple'], colors['yellow'], colors['end'])

java_library_message = '%s\nCreating Java Archive %s==> %s$TARGET%s' % \
   (colors['red'], colors['purple'], colors['yellow'], colors['end'])


#Defining the basic Environment that other targets will extend in order to compile their own stuff.
#Note: The reason I am doing it this way is because each component (ie, target) should ONLY set 
#the minimal number of compiler flags to compile itself
import os
if isLinux():
  env = Environment()
#  env["CXX"]="g++-4.2"
#  env["CXX"]="g++-4.2"
  env["CC"]="g++"
  env["CC"]="g++"
  env["CPPDEFINES"] = ["BT_LINUX"] #the -D flag
  env["ENV"]["PATH"] = os.environ['PATH']
  env["ENV"]["TERM"] = os.environ['TERM']
  env["ENV"]["HOME"] = os.environ['HOME']
else:
  env = Environment(tools=["mingw"])
  env["CXX"]="g++"
  env["CC"]="g++"
  env["CPPDEFINES"] = ["BT_WINDOWS", "BOOST_THREAD_USE_LIB"] #the -D flag
env["LIBPATH"] = ["."] #the -L flag
env["LIBS"] = [] #the -l flag.
env["CPPPATH"] = ["src"] #the -I flag 
env["CCFLAGS"] = ["-O3","-Wall","-g","-Wextra"] #other misc compiler flags


#Add the "verbose" flag.. and also setup the color scheme to use when the "verbose" flag is NOT specified
AddOption("--verbose",action="store_true", dest="verbose_flag",default=False,help="verbose output")
if not GetOption("verbose_flag"):  
  env["CXXCOMSTR"] = compile_source_message,
  env["CCCOMSTR"] = compile_source_message,
  env["SHCCCOMSTR"] = compile_shared_source_message,
  env["SHCXXCOMSTR"] = compile_shared_source_message,
  env["ARCOMSTR"] = link_library_message,
  env["RANLIBCOMSTR"] = ranlib_library_message,
  env["SHLINKCOMSTR"] = link_shared_library_message,
  env["LINKCOMSTR"] = link_program_message,
  env["JARCOMSTR"] = java_library_message,
  env["JAVACCOMSTR"] = java_compile_source_message,

#************************************************
# Compiling the BrailleTutor library.
# Command: scons braille
#************************************************
btEnv = env.Clone()
BT_SOURCES = Glob("src/BrailleTutor-0.7.1/lib/*.cc",strings=True)+Glob("src/BrailleTutor-0.7.1/extras/*.cc",strings=True)

# Overrides
#btEnv["LIBS"].extend(["boost_thread"])
#btEnv["LIBS"].extend(["boost_thread"]) if isLinux() else btEnv["LIBS"].extend(["boost_thread-mgw34-mt-1_34_1"])
#btEnv["LIBS"].extend(["boost_thread"]) if isLinux() else btEnv["LIBS"].extend(["boost_thread-mgw46-mt-1_49"])
btEnv["LIBS"].extend(["boost_thread"]) if isLinux() else btEnv["LIBS"].extend(["boost_thread-mgw47-mt-1_53", "libboost_system-mgw47-mt-1_53"])

btEnv["CPPPATH"].extend([BOOST_HEADERS,"src/BrailleTutor-0.7.1/include","src/BrailleTutor-0.7.1/extras"])
#

#BT_SOURCES is the list of files for compiling the Braille library. However, depending on the platform we remove the serial IO file from this list
file_to_remove = "serial_io_windows.cc" if isLinux() else "serial_io_unix.cc"
BT_SOURCES = filter(lambda file_path: file_to_remove not in file_path,BT_SOURCES)

BT_LIB = btEnv.StaticLibrary(target="bt",source=BT_SOURCES)
btEnv.Alias("braille", BT_LIB)

#************************************************
# Compiling the Sound/Voice code
# Command: scons voice
#************************************************
voiceEnv = env.Clone()
VOICE_SOURCES = "src/Voice/Voice.cc"

# Overrides
voiceEnv["CPPPATH"].extend([SDL_HEADERS])
#

VOICE_OBJ = voiceEnv.Object(target="voice",source=VOICE_SOURCES)
voiceEnv.Alias("voice",VOICE_OBJ)

#************************************************
# Compiling the common utility functions/classes. (KnowledgeTracer, sound language utils, etc)
# Command: scons common
#************************************************
commonEnv = env.Clone()
COMMON_SOURCES = Glob("src/common/*.cc",strings=True)

# Overrides
commonEnv["LIBS"].extend(["bt"])
commonEnv["CPPPATH"].extend([BOOST_HEADERS,SDL_HEADERS,"src/BrailleTutor-0.7.1/include","src/BrailleTutor-0.7.1/extras"])
#

COMMON_LIB = commonEnv.StaticLibrary(target="common",source=COMMON_SOURCES)
commonEnv.Alias("common",COMMON_LIB)

#************************************************
# Compiling the Application Dispatcher module
# Command: scons ad
#************************************************
adEnv = env.Clone()
AD_SOURCES = ["src/app_dispatcher/app_dispatcher.cc"]

# Overrides
adEnv["CPPPATH"].extend(["src/BrailleTutor-0.7.1/include","src/BrailleTutor-0.7.1/extras"])
adEnv["CPPPATH"].extend([SDL_HEADERS]) if isLinux() else adEnv["CPPPATH"].extend([BOOST_HEADERS,SDL_HEADERS]) #TODO XXX: check the app_dispatcher.cc to figure out why it needs Voice objects in the first place
#

AD_LIB = adEnv.StaticLibrary(target="app_dispatcher",source=AD_SOURCES+VOICE_OBJ)
adEnv.Alias("ad",AD_LIB)

#************************************************
# Compiling all the BrailleTutor applications (creates the executable)
# Command: scons
#************************************************
finalEnv = commonEnv.Clone() #Note: Clone()ing from commonEnv
SRC_DIRS=[".","dot_scaffold","household", "arithmetic_practice", "dot_practice","domino","learn_dots","letter_practice","learn_letters","letter_scaffold","animal","hangman","learn_numbers","number_scaffold"]
SRCS = []

for dir in SRC_DIRS:
  SRCS.extend(Glob("src/"+dir+"/*.cc",strings=True))

# Overrides
finalEnv["CPPPATH"].extend([BOOST_HEADERS])
finalEnv["LIBPATH"].extend([BOOST_LIB,SDL_LIB])
finalEnv["LIBS"].extend(["SDL","SDL_mixer"])

#finalEnv["LIBS"].extend(["boost_thread"]) if isLinux() else finalEnv["LIBS"].extend(["boost_thread-mgw34-mt-1_34_1"])
#finalEnv["LIBS"].extend(["boost_thread"]) if isLinux() else finalEnv["LIBS"].extend(["boost_thread-mgw46-mt-1_49"])	# Changed by Gary since I am trying to use boost 1.49.0
finalEnv["LIBS"].extend(["boost_thread"]) if isLinux() else finalEnv["LIBS"].extend(["boost_thread-mgw47-mt-1_53", "boost_system-mgw47-mt-1_53"])	# Changed by Gary since I am trying to use boost 1.53.0

# This is the main executable
FINAL_EXECUTABLE = finalEnv.Program(target="btbt",source=SRCS+VOICE_OBJ+COMMON_LIB+AD_LIB)

#************************************************
# Compiling the MusicMaker game. This exists as a separate binary
# Command: scons mm
#************************************************
musicEnv = finalEnv.Clone() #Note: Clone()ing from finalEnv
MUS_SRC = Glob("src/musicmaker/*.cc",strings=True)

MUSIC_EXECUTABLE = musicEnv.Program(target="musicmakergame", source=MUS_SRC+VOICE_OBJ+COMMON_LIB+AD_LIB)
musicEnv.Alias("mm",MUSIC_EXECUTABLE)

#*********************************************************
# Compiling the special hangman game for Interactive 2011
# This exists as a separate binary
# Command: scons int2011
#*********************************************************
int2011Env = finalEnv.Clone() #Note: Clone()ing from finalEnv
INT2011_SRC = Glob("src/interactive2011/*.cc",strings=True)

INT2011_EXECUTABLE = int2011Env.Program(target="interactive2011", source=INT2011_SRC+VOICE_OBJ+COMMON_LIB+AD_LIB)
musicEnv.Alias("int2011", INT2011_EXECUTABLE)


# Specify which targets to build by default
Default( FINAL_EXECUTABLE,MUSIC_EXECUTABLE)

#************************************************
# Misc
#************************************************
#Check if the user tried to issue a "clean" command (instead of "scons -c") - if so, we just do the cleaning
if "clean" in COMMAND_LINE_TARGETS:
  SetOption("clean", True)
  Alias("clean", DEFAULT_TARGETS)

#Run builds in parallel
import os
num_cpu = int(os.environ.get('NUM_CPU', 2))
SetOption('num_jobs', num_cpu)
print "INFO: Running with -j", GetOption('num_jobs')
