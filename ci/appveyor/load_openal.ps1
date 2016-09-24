$start_dir = $pwd

$openaldir = $ENV:OPENAL
$sha2sum = "A341F8542F1F0B8C65241A17DA13D073F18EC06658E1A1606A8ECC8BBC2B3314"
$openalurl = "http://kcat.strangesoft.net/openal-releases/openal-soft-1.17.2.tar.bz2"
$openalfile = "openal-soft-1.17.2.tar.bz2"
$openaltar = "openal-soft-1.17.2.tar"
$7zipoutdir = "C:\"

if (Test-Path $openaldir) {
	echo "$($openaldir) already exists"
	exit 0
}
echo "$($openaldir) is missing. Getting openal"

if (Test-Path $openalfile) {
	echo "Zip exists"
	$sum = Get-FileHash -LiteralPath $openalfile -Algorithm "SHA256"
	if ($sum.Hash -ne $sha2sum) {
		echo "Current file hash doesn't match"
		Remove-Item $openalfile
	}
}
if (-Not (Test-Path $openalfile)) {
	echo "Fetching zip"
	(new-object net.webclient).DownloadFile($openalurl, $openalfile)
}

$sum = Get-FileHash -LiteralPath $openalfile -Algorithm "SHA256"
if ($sum.Hash -ne $sha2sum) {
	echo "ERROR: $($wxfile) is corrupt"
	echo "Orig: $($sha2sum)"
	echo "Calc: $($sum.Hash)"
	exit 3
}

$unzip_args = "x", $openalfile, "-y"
& "C:\Program Files\7-zip\7z.exe" $unzip_args
if ($exitcode -gt 0) {exit $exitcode}

$unzip_args = "x", $openaltar, "-y", "-o$($7zipoutdir)"
& "C:\Program Files\7-zip\7z.exe" $unzip_args
if ($exitcode -gt 0) {exit $exitcode}

cd $openaldir

cd build

& "$($start_dir.Path)\ci\appveyor\build_openal.cmd"
if ($exitcode -gt 0) {exit $exitcode}
