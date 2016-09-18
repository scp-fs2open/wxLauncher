setlocal ENABLEEXTENSIONS

echo on

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %Platform%

cd %APPVEYOR_BUILD_FOLDER%
if not exist build (mkdir build)
cd build

echo.
echo Configuring build
cmake .. -G "NMake Makefiles" -DWXVER=%WXVER% ^
	-DCMAKE_BUILD_TYPE=%Configuration% ^
	-DOPENAL_LIBRARY=%OPENAL_LIBRARY% ^
	-DOPENAL_INCLUDE_DIR=%OPENAL_INCLUDE_DIR%
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

echo.
echo Building
nmake
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

echo.
echo Making package
nmake package
if ERRORLEVEL 1 exit /b %ERRORLEVEL%
