@echo off

setlocal

set versionApp=1.62.0.161

echo.
echo Remember to set the version in this script...
echo Version set to %versionApp%.
echo.
rem http://support.microsoft.com/kb/913111
filever ..\Win32\Release\Timekeeper.dll
filever ..\x64\Release\Timekeeper.dll
echo (Note: Filever 32-bit says: W32i is 32-bit. W32 and Wx64 are 64-bit.)
pause

cd "%~dp0"
cd

md files >nul:
del /q files\*.*

REM copy /y ..\..\Documents\License-free.txt	files
REM copy /y ..\..\Documents\Warranty.txt     files

copy /y ..\Win32\Release\Timekeeper.dll	files
copy /y ..\x64\Release\Timekeeper.dll		files\Timekeeper64.dll
copy /y ..\help\*.png							files
copy /y ..\help\*.htm							files
copy /y ..\help\*.gif							files

attrib -r files\*.*

del /q timekeeper-setup.exe


set pathEXE=%ProgramFiles(x86)%\NSIS\makensis.exe
if not exist "%pathEXE%" (
	echo.
	echo Unable to find NSIS. Please install from http://nsis.sourceforge.net.
	echo.
	goto :EOF
)

"%pathEXE%" -DgVerInstaller=%versionApp% Timekeeper.nsi 
REM "%pathEXE" -DgVerInstaller=%versionApp% Timekeeper64.nsi 


if exist timekeeper-setup.exe (
	echo Updating Web site directory...
	ECHO xcopy /-y timekeeper-setup.exe "%OneDrive%\Documents\Web Sites\skst.com\12noon\files"
)

endlocal
