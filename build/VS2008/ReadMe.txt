Read me for building wxLauncher in VS2008.

== Adding Python to your PATH ==
These instructions assume that you have not installed Python before. So download Python 2.6 (which at the time of writing is version 2.6.4).  Python 2.6 by default installs into C:\Python26, if this is different that where you installed Python then use that directory instead.
1. Download and install Python. (Remeber what directory you installed it into).
2. Navigate Control Panel > System.
2a. On Windows Vista and 7 you then need to click "Advanced System Settings" in the left hand tasks menu. Allow if the UAC prompts you.
3. Go to the Advanced Tab.  Click the "Evvironment Variables" button at the bottom of the Advanced tab.
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

