@echo off

cd "%~dp0"

rem	N.B. xcopy does NOT copy read-only attribute
xcopy /y ..\help\*.htm
xcopy /y ..\help\*.png
xcopy /y ..\help\*.gif

echo Building 32-bit installer...
xcopy /y ..\release\Timekeeper.dll
"%ProgramFiles%\Install Creator\ic.exe" /b Timekeeper.iit 


echo Building 64-bit installer...
xcopy /y ..\x64\release\Timekeeper.dll
"%ProgramFiles%\Install Creator\ic.exe" /b Timekeeper64.iit 


echo Updating Web site directory...
xcopy /-y timekeeper-i.exe "%userprofile%\My Documents\Web Sites\skst.com\htdocs\12noon\files"
xcopy /-y timekeeper-i64.exe "%userprofile%\My Documents\Web Sites\skst.com\htdocs\12noon\files"
