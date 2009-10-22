@echo off
rem %2 is build destination, %1 is the project directory
echo Copying Resources to "%2"
xcopy %1..\..\resources\* %2 /E /C /Y