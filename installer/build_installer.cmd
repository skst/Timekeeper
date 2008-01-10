@echo off

cd "%~dp0"

rem	N.B. xcopy does NOT copy read-only attribute
xcopy /y ..\release\Timekeeper.dll
xcopy /y ..\x64\release\Timekeeperx64.dll
xcopy /y ..\help\*.htm
xcopy /y ..\help\*.png
xcopy /y ..\help\*.gif

echo Building 32-bit and 64-bit installers...
"%ProgramFiles%\Install Creator\ic.exe" /b Timekeeper.iit 
"%ProgramFiles%\Install Creator\ic.exe" /b Timekeeper64.iit 

echo Updating Web site directory...
xcopy /-y timekeeper-i.exe "%userprofile%\My Documents\Web Sites\skst.com\htdocs\12noon\files"
xcopy /-y timekeeper-i64.exe "%userprofile%\My Documents\Web Sites\skst.com\htdocs\12noon\files"
