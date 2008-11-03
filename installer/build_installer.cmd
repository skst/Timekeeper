@echo off

setlocal

set versionApp=1.42.131.0

echo.
echo Remember to set the version in this script...
echo Version set to %versionApp%.
echo.
filever ..\Release\Timekeeper.dll
filever ..\x64\Release\Timekeeper.dll
pause

cd "%~dp0"
cd

del /q files\*.*

REM copy /y ..\..\Documents\License-free.txt	files
REM copy /y ..\..\Documents\Warranty.txt     files

copy /y ..\Release\Timekeeper.dll		files
copy /y ..\x64\Release\Timekeeper.dll	files\Timekeeper64.dll
copy /y ..\help\*.png						files
copy /y ..\help\*.htm						files
copy /y ..\help\*.gif						files

attrib -r *.*

del /q timekeeper-setup.exe


set pathEXE=%ProgramFiles%\NSIS\makensis.exe
if not exist "%pathEXE%" (
	echo.
	echo Unable to find NSIS. Please install from http://nsis.sourceforge.net.
	echo.
	goto :EOF
)

"%pathEXE%" -DgVerInstaller=%versionApp% Timekeeper.nsi 
ECHO "%pathEXE" -DgVerInstaller=%versionApp% Timekeeper64.nsi 


if exist timekeeper-setup.exe (
	echo Updating Web site directory...
	xcopy /-y timekeeper-setup.exe "%userprofile%\My Documents\Web Sites\skst.com\12noon\files"
)

endlocal
