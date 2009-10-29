Read me for building wxLauncher in VS2008.

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

