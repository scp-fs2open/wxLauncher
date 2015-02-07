setlocal ENABLEEXTENSIONS

:contargs
if "%2"=="" goto doneargs
set %1=%2
shift
shift
goto contargs

:doneargs

call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
echo %INCLUDE%
echo %LIB%
echo %LIBPATH%

if defined WXVER (set ARGWXVER="-DWXVER=%WXVER%")
if defined BUILDTYPE (set ARGBUILDTYPE="-DBUILDTYPE=%BUILDTYPE%")

mkdir build
cd build

echo Configuring build
cmake .. -G "NMake Makefiles" %ARGBUILDTYPE% %ARGWXVER%
