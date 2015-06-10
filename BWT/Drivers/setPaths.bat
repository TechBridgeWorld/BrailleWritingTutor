@echo off
set current=%cd%
pushd ..
set parent=%cd%
popd

echo current %current%
echo parent %parent%

setx path "%path%;%parent%\externals.win\RequiredFiles;%parent%\externals.win\SDL\dll"