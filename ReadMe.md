# wxLauncher: Multi-platform launcher for Freespace 2

Introduction
============
[wxLauncher] aims to give one unified answer to members of 
the Freespace 2 Open community looking for an easy way 
to control `fs2_open` on various platforms and to those 
looking for an easy way to find and get updates to their
Freespace 2 MODs and TCs.

[wxLauncher]: https://github.com/wxLauncher/wxlauncher

Compatibility
=============
wxLauncher is built for the 3.6.9+ versions of `fs2_open`.
While older versions might function, we don't officially
support them. Use them at your own risk.

Obtaining wxLauncher
====================
Precompiled binaries can be found on the project
[releases page](https://github.com/wxLauncher/wxlauncher/releases)

Find the build for your system and you're set.

Building from source
====================
The wxLauncher requires the CMake build system for building.
CMake is a cross-platform meta-build system; it generates the
files that allow a platform's native build system to build the
launcher.  CMake supports VS2005 and newer, as well as
Xcode, KDevelop, and of course autotools.

### Requirements shortlist

  * All platforms:
      * CMake 2.8
      * wxWidgets 2.8.10+ or 3.0.2
      * Python 2.6+ (but not 3.x)
      * Markdown in Python
  * Windows
      * Windows SDK or Platform SDK
      * Nullsoft Scriptable Install System (NSIS)
  * Linux
      * SDL 1.2
  * OS X
      * SDL 1.2

### Optional components
  * All platforms:
      * OpenAL or OpenALSoft

### Detailed requirements

wxLauncher is built using CMake.  Only version 2.8 or later has
been tested (and the CMake file enforces this).  CMake can be 
downloaded in binary form, from the [CMake Home Page] or if you
run Linux, from your distro's package repository.
[Cmake Home Page]: http://cmake.org

wxLauncher is a [wxWidgets]-based application. It can 
only be built with wxWidgets version 2.8.10 or higher.
Version 0.10.0 has been tested with:
  - 2.8.12 (no STL)
  - 2.8.12 (STL)
  - 3.0.2 (no STL)
  - 3.0.2 (STL)
[wxWidgets]: http://www.wxwidgets.org/
	
[Python] 2.6 or higher is required to build this project.
This project also assumes that the python executable is 
in your PATH.  Check your operating system's documentation
for information on how to add Python to your PATH.
[Python]: http://www.python.org/
	
Markdown in Python is required in order to build the 
integrated help system.
  * [Markdown in Python homepage](http://freewisdom.org/projects/python-markdown/)
  * On `pip` enabled installs, try: `pip install markdown`
  * On Debian-based systems, try: `apt-get install python-markdown`

The OpenAL Software Development Kit is an optional component
needed to build this program.  It can be downloaded from the
[OpenAL homepage]. OpenAL support requires the 
preprocessor symbol `USE_OPENAL=1`.  This symbol is set to 1 by
default by CMake.  You can pass `-DUSE_OPENAL=0` to CMake to
disable building with OpenAL support.  See the compiler specific
instructions for getting your compiler ready to build
with OpenAL. Note that Mac OS X ships with OpenAL pre-installed.
wxLauncher should also work with the [OpenALSoft] library.
[OpenAL homepage]: http://connect.creativelabs.com/openal/default.aspx
[OpenALSoft]: http://kcat.strangesoft.net/openal.html

The Microsoft Windows SDK (formerly the Platform SDK) is
required to build both this application and wxWidgets when
 on Windows.  Note that only the Windows SDK for Windows
Vista and Windows 7 have been tested with this application.

The Nullsoft Scriptable Install System (NSIS) is required on
Windows to build the installer. The latest version at the time
of writing is 2.46. Be sure to either install NSIS before 
running CMake or re-run CMake after installing NSIS. NSIS can
be found on the [NSIS homepage](http://nsis.sourceforge.net/).

The offical wxLauncher releases for Windows are built with
Microsoft's Visual Studio 2008 Express edition. Other
Visual Studio versions should work, but are not routinely
tested. [Visual Studio Express] editions can be downloaded
and used for developing wxLauncher for free.
[Visual Studio Express]: http://www.microsoft.com/express/download/

wxLauncher's source can be explored from the project's
[source page](https://github.com/wxLauncher/wxlauncher/)
  
To get the source, you'll need Git:
  * [Command Line Git](http://git-scm.com/)
  * Graphical frontend: [TortioseGit](http://tortiosgit.org)
  * Distro package manager. Sometimes called `git-scm`
  * The Homebrew Project for OS X

Once Git is installed, you can get a copy of the source
by running the following command in a folder of your choice:
	`git clone https://code.google.com/p/wxlauncher/`
  
Building - Windows
==================
Run CMake in your favourite way (GUI, or on the commandline 
ccmake (uses curses) or cmake).

CMake QT Gui
------------
Assuming the GUI, select the CMakeLists.txt in the main
wxLauncher source directory and set your output directory
to where you want the native build tool to be placed,
somewhere without spaces. Click configure until the Generate
button Enables.  The lines that are highligted red are new 
variables that CMake has found. 
- Set wxWdigets_ROOT_DIR to the root directory of your wxWidgets 
source directory if it remains NOTFOUND.
- Set PYTHON_EXECUTABLE to the python that you want to use. It may not show
up. If it doesn't show up, it means that cmake found python automatically,
but if cmake selects the wrong version of python, please see "CMake selects the
wrong python".
- Check DEVELOPMENT_MODE if you are going to be debugging wxLauncher. Make sure
that it is unchecked if you plan on distributing the code as with this checked,
the launcher will only run on the dev machine. This option changes where the
launcher looks for files to display such as the default interface.
- Check USE_JOYSTICK, USE_OPENAL, and/or USE_SPEECH if you want those options compiled in.
- If USE_OPENAL is checked, OPENAL_INCLUDE_DIR will appear. Set it to the include
folder in the OpenAL SDK folder.

Building - Linux (has been tested on Ubuntu)
=========================
Command line
-----------
- Download the wxLauncher source
- sudo apt-get install build-essential libopenal-dev libwxgtk2.8-dev
libwxgtk2.8-dbg python-markdown
- Download and install the cmake 2.8 .debs for your platform from:
<https://launchpad.net/ubuntu/+source/cmake>.  You will need cmake-data, and
cmake-2.8.0* for your platform, plus cmake-curses-gui or cmake-qt-gui
- cd <source directory>
- mkdir build
- cd build
To prevent error messages like "The launcher is expecting (/home/foo/bar) to
contain the resource images." from occurring, either type:
- cmake -DUSE_OPENAL=1 -DDEVELOPMENT_MODE=1 -DCMAKE_INSTALL_PREFIX=/usr/local ../
- make
Or instead type:
- cmake -DUSE_OPENAL=1 -DCMAKE_INSTALL_PREFIX=/usr/local ../
- make
- make install (with root privileges)

CMake selects the wrong python
------------------------------
If CMake selects the wrong version of python (should be version 2), you
can set the version yourself by setting PYTHON_EXECUTABLE when you run CMake by
adding "-DPYTHON_EXECUTABLE=</path/to/python2>" to your command line, for
example "-DPYTHON_EXECUTABLE=/usr/bin/python2".

If you run into this problem, please post in this ticket
<http://code.google.com/p/wxlauncher/issues/detail?id=99> with your distro,
OS version, and whether you're running 32- or 64-bit.

Building - OS X 10.6 (Snow Leopard), although should also work on 10.5 (Leopard)
==============================
- Download and install the most recent version of Xcode 3. You will need
a free Apple Developer Center account to download Xcode 3. If you have a
different version of Xcode 3, such as a copy provided on your OS X install disc,
that might also work, but it can't be guaranteed.
- Download and install the most recent version of Python 2 if you don't
already have at least Python 2.6; Python 3 will not work
- Download and install Markdown in Python (link is provided above)
- Download and install Git, making sure that you select the version of
  Git for your version of OS X
- Download the wxLauncher source
- Download and install CMake 2.8
- Download and build wxWidgets 2.8.12 (use the wxMac version). Once you've
downloaded and extracted the wxMac-2.8.12 tarball, do these things:
 * cd wxMac-2.8.12/
 * type either "mkdir build-debug" or "mkdir build-release" (your choice)
 * cd <TheBuildDirYouJustMade>
 * Type the following to configure, adjusting according to the notes that follow:
 * arch_flags="-arch i386"
 * ../configure CFLAGS="$arch_flags" CXXFLAGS="$arch_flags"
 CPPFLAGS="$arch_flags" LDFLAGS="$arch_flags" OBJCFLAGS="$arch_flags"
 OBJCXXFLAGS="$arch_flags" --enable-unicode --enable-debug --disable-shared
 --with-macosx-sdk=/Developer/SDKs/MacOSX10.5.sdk --with-macosx-version-min=10.5
   > If you're building on Leopard, leave out the 'arch_flags="-arch i386"' and
   the 'CFLAGS="$arch_flags" CXXFLAGS="$arch_flags" CPPFLAGS="$arch_flags"
   LDFLAGS="$arch_flags" OBJCFLAGS="$arch_flags" OBJCXXFLAGS="$arch_flags"' parts
   > If you're not interested in compatibility with Leopard, leave out the
   '--with-macosx-sdk=/Developer/SDKs/MacOSX10.5.sdk --with-macosx-version-min=10.5' part
   > If you want a release build rather than a debug build, leave out the '--enable-debug'
 * make
- Install SDL: you can either (1) download, build, and install it yourself (such
as with MacPorts) or (2) simply download the Frameworks.tgz tarball from the
FreeSpace Open source tree
<http://svn.icculus.org/*checkout*/fs2open/trunk/fs2_open/projects/Xcode/Frameworks.tgz>
and extract the SDL.framework, then copy the SDL.framework folder to your
/Library/Frameworks folder
- Run CMake *twice* either by using cmake at the command line or by using the
CMake GUI (which is most likely in /Applications, although you can find it using
Spotlight), selecting Xcode as your generator. For currently unknown reasons,
CMake must be run twice for CPack to correctly generate drag-and-drop .apps.
- A few notes on configuring the CMake variables:
 * Set wxWidgets_CONFIG_EXECUTABLE and wxWidgets_wxrc_EXECUTABLE to point
 to the version of wxWidgets you built, not the pre-installed version in
 /usr/local. wxWidgets_CONFIG_EXECUTABLE is located at
 /yourWxWidgetsBuildDir/wx-config and wxWidgets_wxrc_EXECUTABLE is
 located at /yourWxWidgetsBuildDir/utils/wxrc/wxrc
 * If you are not using SDL.framework, uncheck USING_SDL_FRAMEWORK (or on
 command line, add -DUSING_SDL_FRAMEWORK=0)
 * If you are using SDL.framework in /Library/Frameworks, make sure that
 SDL_LIBRARY is set to /Library/Frameworks/SDL.framework, since CMake might
 automatically add extra stuff to SDL_LIBRARY, such as ";-framework Cocoa"
 * If you're building on Snow Leopard or later with Leopard compatibility, make
 sure that CMAKE_OSX_SYSROOT is set to /Developer/SDKs/MacOSX10.5.sdk
- Example command-line usage of CMake if you're not building for 10.5 and are
using the SDL.framework:
 * cd <wxLauncherSourceDir>
 * mkdir build
 * cd build
 * cmake -G Xcode -DwxWidgets_CONFIG_EXECUTABLE=/<wxWidgetsBuildDir>/wx-config
   -DwxWidgets_wxrc_EXECUTABLE=<wxWidgetsBuildDir>/utils/wxrc/wxrc
   -DSDL_LIBRARY=/Library/Frameworks/SDL.framework -DUSE_OPENAL=1 ../
- Once you have your Xcode project set up, build the "ALL_BUILD" target to build
wxlauncher.app in /yourWxLauncherBuildDir/YourSelectedBuildConfig/ , or type
"xcodebuild -configuration <YourSelectedBuildConfig>" at the shell prompt in
your wxLauncher build folder. Make sure that the build configuration you choose
(Debug or Release) matches the build configuration you used when you built wxWidgets.

Important known issues on OS X:
 - After startup or after a FS2 Open binary is (re-)selected, checkboxes on
the advanced settings page may not appear until after a moment or after the
user interacts with the advanced settings page, such as by clicking on the
flag list. 

License
=======
See License.txt.
