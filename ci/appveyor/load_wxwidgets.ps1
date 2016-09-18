if ($ENV:WXVER -eq "2.8") {
	$wxurl = "https://github.com/wxWidgets/wxWidgets/releases/download/v2.8.12/wxMSW-2.8.12.zip"
	$wxdir = "C:\wxMSW-2.8.12"
	$wxfile = "wxMSW-2.8.12.zip"
	$sha2sum = "307D713D8AFFBED69A89418D9C9073193AADFEF4B16DA3D8EF68558A9F57AE88"
	$7zipoutdir = "C:\"
} else {
	$wxurl = "https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.0/wxWidgets-3.1.0.7z"
	$wxdir = "C:\wxWidgets-3.1.0"
	$wxfile = "wxWidgets-3.1.0.7z"
	$sha2sum = "FDB970D6D3278B5ABD999E6F87295C2162E64184826D07B1AA26AB1E3E46BA27"
	$7zipoutdir = $wxdir
}

$start_dir = $pwd

if (Test-Path $wxdir) {
	echo "$($wxdir) already exists"
	exit 0
}
echo "$($wxdir) is missing. Getting wx"

if (Test-Path $wxfile) {
	echo "Zip exists"
	$sum = Get-FileHash -LiteralPath $wxfile -Algorithm "SHA256"
	if ($sum.Hash -ne $sha2sum) {
		echo "Current file hash doesn't match"
		Remove-Item $wxfile
	}
}
if (-Not (Test-Path $wxfile)) {
	echo "Fetching zip"
	Invoke-WebRequest $wxurl -outfile $wxfile
}

$sum = Get-FileHash -LiteralPath $wxfile -Algorithm "SHA256"
if ($sum.Hash -ne $sha2sum) {
	echo "ERROR: $($wxfile) is corrupt"
	echo "Orig: $($sha2sum)"
	echo "Calc: $($sum.Hash)"
	exit 3
}

$unzip_args = "x", $wxfile, "-o$($7zipoutdir)"

& "C:\Program Files\7-zip\7z.exe" $unzip_args

cd $wxdir

if ($stl) {}

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

