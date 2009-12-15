@echo off
rem %2 is build destination, %1 is the resources directory
echo Copying Resources to "%2"
xcopy %1\* %2 /E /D /C /Y