setlocal ENABLEEXTENSIONS

echo on

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %ARCH%
echo %INCLUDE%
echo %LIB%
echo %LIBPATH%

set

cd %APPVEYOR_BUILD_FOLDER%
if not exist build (mkdir build)
cd build

echo.
echo Configuring build
cmake .. -G "NMake Makefiles" -DWXVER=%WXVER% -DCMAKE_BUILD_TYPE=%BUILDTYPE%
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

echo.
echo Building
nmake
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

echo.
echo Making package
nmake package
if ERRORLEVEL 1 exit /b %ERRORLEVEL%
