:contargs
@if "%2"=="" goto doneargs
set %1=%2
@shift
@shift
@goto contargs

:doneargs

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %Platform%

echo "Configuring..."
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
if ERRORLEVEL 1 exit /b %ERRORLEVEL%

echo "Building"
cmake .. --build .
if ERRORLEVEL 1 exit /b %ERRORLEVEL%
