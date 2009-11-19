Read me for building wxLauncher in VS2008.

== Adding support for OpenAL ==
OpenAL is supported in the code by define the preprocessor symbol USE_OPENAL=1, if not defined or defined as USE_OPENAL=0, OpenAL support is not compiled into the launcher at all.

For VS2008 the provided solution will build with USE_OPENAL=1 and as such requires that the include directory be defined. For the VS2008 solution, it requires that the system has the environment variable OPENALPATH defined to be the path without spaces but including the drive letter to the root location that OpenAL SDK is installed (the path should not end with 'include' as the project will add that itself).

1. Download the OpenAL SDK from <http://www.openal.org>. The last checked direct link to the required SDK is <http://connect.creativelabs.com/openal/Downloads/OpenAL11CoreSDK.zip>.
2. Extract the SDK to a location that does not have any spaces in the path.
3. Navigate Control Panel > System.
3a. On Windows Vista and 7 you then need to click "Advanced System Settings" in the left hand tasks menu. Allow if the UAC prompts you.
4. Go to the Advanced Tab.  Click the "Environment Variables" button at the bottom of the Advanced tab.
5. In "System Variables" click New....
6. Set the "Variable name" to OPENALPATH, and set "Variable Value" to the path to the root of the directory that you the OpenAL SDK in.  Remeber there should not be any spaces in the path.
7. Click OK. Click OK. Click OK. You should now be out of the Systems Settings window.
8. If Visual Studio is already running close all instances of it now and restart them.  You should be able to build the project now.  If you continue to have problems then you will have to restart your computer.

NOTE: wxLauncher does not need the link library as the OpenAL32.dll is loaded dynamicly, this allows the launcher to run on systems that do not have OpenAL installed and still work correctly.  

== Adding Python to your PATH ==
These instructions assume that you have not installed Python before. So download Python 2.6 (which at the time of writing is version 2.6.4).  Python 2.6 by default installs into C:\Python26, if this is different that where you installed Python then use that directory instead.
1. Download and install Python. (Remeber what directory you installed it into).
2. Navigate Control Panel > System.
2a. On Windows Vista and 7 you then need to click "Advanced System Settings" in the left hand tasks menu. Allow if the UAC prompts you.
3. Go to the Advanced Tab.  Click the "Environment Variables" button at the bottom of the Advanced tab.
4. In "System Variables" find variable "Path", select it and click edit. Do not delete anything from the box.
5. You should be at the end of the Variable Value edit box. 
5a. Type: ;C:\Python26
5b. Make sure that you do not have anything selected and you enter the text at the end of the line.  Replace C:\Python26 with the directory that you installed python in if it is different.
6. Click OK. Click OK. Click OK. You should now be out of the Systems Settings window.
7. If Visual Studio is already running close all instances of it now and restart them.  You should be able to build the project now.  If you continue to have problems then you will have to restart your computer.

== FAQ ==
Q: I can't launch the project from Visual Studio's Start Debugging action.
 wxLauncher complains that some assertion fails about an image being ok.
 
A: If you can get it to work by just running the wxLauncher.exe directly from 
the build directory and you do not get the error, this is caused by your debug
working directory being set incorrectly (or not at all). 
To correct this:
 * Right-click on the wxLauncher project and select properties.
 * Congiuation properties > Debugging.
 * Set "Working Directory" to "$(TargetDir)" (without quotes).
You should now beable to run the launcher from VS.

