setlocal ENABLEEXTENSIONS

:contargs
@if "%2"=="" goto doneargs
set %1=%2
@shift
@shift
@goto contargs

:doneargs

call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
echo %INCLUDE%
echo %LIB%
echo %LIBPATH%

if defined WXVER (set ARGWXVER="-DWXVER=%WXVER%")
if defined BUILDTYPE (set ARGBUILDTYPE="-DBUILDTYPE=%BUILDTYPE%")

if not defined LIBRARY_PATHS (set LIBRARY_PATHS=C:\libraries\library_paths.cmd)

if exist %LIBRARY_PATHS% (call %LIBRARY_PATHS%)

if not exist build (mkdir build)
cd build

echo.
echo Configuring build
"%CMAKE%" .. -G "NMake Makefiles" %ARGBUILDTYPE% %ARGWXVER% -Donlinehelpmaker_debug=ON
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

echo.
echo Building
nmake
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

echo.
echo Making package
nmake package
if ERRORLEVEL 1 exit /b %ERRORLEVEL%
