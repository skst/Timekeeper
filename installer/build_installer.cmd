@echo off

setlocal

set versionApp=1.62.9.0

echo.
echo Remember to set the version in this script...
echo Version set to %versionApp%.
echo.
pause

cd "%~dp0"
cd

md files >nul:
del /q files\*.*

copy /y ..\Win32\Release\Timekeeper.dll	files
copy /y ..\x64\Release\Timekeeper.dll		files\Timekeeper64.dll
copy /y ..\help\*.htm							files
copy /y ..\help\*.png							files
copy /y ..\help\*.gif							files
copy /y ..\LICENSE								files

attrib -r files\*.*

if EXIST timekeeper-setup.exe del /q timekeeper-setup.exe

set pathEXE=makensis.exe
if not exist "%pathEXE%" (
	echo.
	echo Unable to find NSIS. Please install from http://nsis.sourceforge.net.
	echo.
	goto :EOF
)

"%pathEXE%" -DgVerInstaller=%versionApp% Timekeeper.nsi

endlocal
