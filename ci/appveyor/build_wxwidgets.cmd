:contargs
@if "%2"=="" goto doneargs
set %1=%2
@shift
@shift
@goto contargs

:doneargs

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %Platform%

nmake -f makefile.vc BUILD=%BUILD% UNICODE=1