@echo off
set currDir=%~dp0
"%currDir%\Drivers\Driver Installer <CDM 2.02.04>.exe"
REM "only add drivers folder if drivers do not already exist"
IF NOT EXIST "C:\Program Files\Braille Tutor Drivers\SDL\dll" (
	xcopy "%currDir%\externals.win" "C:\Program Files\Braille Tutor Drivers" /E /I
)
REM "only add path if path does not already exist"
if "%path:C:\Program Files\Braille Tutor Drivers\SDL\dll=%"=="%path%" (
	setx path "%path%;C:\Program Files\Braille Tutor Drivers\RequiredFiles;C:\Program Files\Braille Tutor Drivers\SDL\dll"
)