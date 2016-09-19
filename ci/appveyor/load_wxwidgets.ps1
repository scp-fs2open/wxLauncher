Get-Childitem -Path Env:* | Sort-Object Name

$start_dir = $pwd
switch ($ENV:WXVER) {
	"2.8" {$wxdir = ${ENV:WXWIDGETS2.8} }
	"2.8-stl" {$wxdir = ${ENV:WXWIDGETS2.8-stl} }
	"3.1" {$wxdir = ${ENV:WXWIDGETS3.1} }
	"3.1-stl" {$wxdir = ${ENV:WXWIDGETS3.1-stl} }
	default {
		echo "$($ENV:WXVER) is unhandled"
		exit 4
	}
}
$wxfile = $ENV:WXFILE
$wxurl = $ENV:WXURL
$wxsum = $ENV:WXSUM

if (Test-Path $wxdir) {
	echo "$($wxdir) already exists"
	exit 0
}
echo "$($wxdir) is missing. Getting wx"

if (Test-Path $wxfile) {
	echo "Zip exists"
	$sum = Get-FileHash -LiteralPath $wxfile -Algorithm "SHA256"
	if ($sum.Hash -ne $wxsum) {
		echo "Current file hash doesn't match"
		Remove-Item $wxfile
	}
}
if (-Not (Test-Path $wxfile)) {
	echo "Fetching zip"
	(new-object net.webclient).DownloadFile($wxurl, $wxfile)
}

$sum = Get-FileHash -LiteralPath $wxfile -Algorithm "SHA256"
if ($sum.Hash -ne $wxsum) {
	echo "ERROR: $($wxfile) is corrupt"
	echo "Orig: $($wxsum)"
	echo "Calc: $($sum.Hash)"
	exit 3
}

$unzip_args = "x", $wxfile, "-y", "-oC:/"

& "C:\Program Files\7-zip\7z.exe" $unzip_args

cd $wxdir

# Include is not needed and can be removed http://stackoverflow.com/a/17144445
(Get-Content src/msw/window.cpp) -replace '#include <pbt.h>', '' | Set-Content src/msw/window.cpp

cd build\msw

if ($ENV:Configuration -eq "Debug") {
	$buildtype = "BUILD=debug"
} else {
	$buildtype = "BUILD=release"
}
echo "Building UNICODE=1 $($buildtype)"
$build_args = "UNICODE=1", $buildtype
& "$($start_dir.Path)\ci\appveyor\build_wxwidgets.cmd" $build_args

