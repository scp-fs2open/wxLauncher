wxLauncher - A multi-platform dual-purpose application
	http://code.google.com/p/wxlauncher/

Introduction
============
wxLauncher aims to give one unified answer to members of 
the Freespace 2 Open community looking for an easy way 
to control fs2_open on various platforms and to those 
looking for an easy way to find and get updates to their
Freespace 2 MODs and TCs.

Compatibility
=============
wxLauncher is built for the 3.6.9+ versions of fs2_open.
while older versions might function, we don't officially
support them. Use them at your own risk.

Download binaries
=============
Precompiled binaries can be found on the project downloads
page:
	http://code.google.com/p/wxlauncher/downloads/list

Just find the build for your system and you're set.

Building from source
============
The wxLauncher requires the CMake build system to for building.
CMake is a Cross-platform meta-build system (it makes the files
that allow a platform native build system to build the 
launcher).  CMake allows us to support your favourite complier
from VS2005 and newer, XCode, KDevelop, and of course autotools.

Requirements shortlist:
All platforms:
 - CMake 2.8
 - wxWidgets
 - Python
 - Markdown for Python
Windows
 - Platform SDK
Linux
 - SDL 1.2
OS X
 - SDL 1.2

Optional components
All platforms:
 - OpenAL

wxLauncher is built using CMake.  Only version 2.8 has been 
tested (and the CMake file enforces this).  CMake can be 
downloaded in binary form, from the [CMake Home Page] or from
your distro's package repository.
  [Cmake Home Page]: http://cmake.org

wxLauncher is a wxWidgets based application, thus can 
only be built with wxWidgets version 2.8.10 or higher 
installed on your system.
wxWidgets homepage:
	http://www.wxwidgets.org/
	
Python 2.5 or higher is required to build this project.
This project also assumes that the python execuatble is 
in your PATH.  See the compiler specific instructions
on how to add Python to your PATH.
Python homepage:
	http://www.python.org/
	
Markdown for Python is required in order to build the 
integrated help system. 
Python Markdown homepage:
	http://www.freewisdom.org/projects/python-markdown/
On debian-based systems:
	apt-get install python-markdown

The OpenAL Software Development Kit is an optional component
needed to build this program.  OpenAL support requires the 
preprocessor symbol USE_OPENAL=1.  This symbol is set to 1 by
default by CMake.  You can pass -DUSE_OPENAL=0 to CMake to
disable building of OpenAL support.  See the compiler specific
instructions for getting your compiler ready to build
with OpenAL.
OpenAL homepage:
	http://connect.creativelabs.com/openal/default.aspx

The Microsoft Windows SDK (formorly the Platform SDK) is
required to build both this application and wxWidgets when
 on Windows.  Note that only the Windows SDK for Windows
Vista and Windows 7 have been tested with this application.

wxLauncher has only been built with Microsoft's Visual Studio
2008 Express edition.  Other VS2008 versions should work, as
well as any version of VS2005 should work, but have not been
tested.  Visual Studio 2008 C++ Express can be downloaded
from [Microsoft] for free.
[Microsoft]: http://www.microsoft.com/express/download/

wxLauncher's source can be explored from the project's
source page:
	http://code.google.com/p/wxlauncher/source/checkout
  
To get the source, you'll need Mercurial:
	http://mercurial.selenic.com/
or the TortiseHG frontend (which includes Mercurial):
	http://tortoisehg.bitbucket.org/

Once Mercurial is installed, you can get a copy of the source
by runningthe following command (in a folder of your choice):
	hg clone https://wxlauncher.googlecode.com/hg/ wxlauncher  
  
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
- Set PYTHON_EXECUTABLE to the python that you want to use (it may not show up. If it doesn't show up, don't worry about it, it means that cmake found it)
- Check DEVELOPMENT_MODE if you are going to be debugging wxLauncher.
 Make sure that it is unchecked if you plan on distributing the
code as with this checked the launcher will only run on the dev 
machine. This option changes where the launcher looks for files to display as the default interface.
- Check USE_JOYSTICK, USE_OPENAL, and/or USE_SPEECH if you want those options complied in.
- If USE_OPENAL is check, OPENAL_INCLUDE_DIR will appear, this 
should be set the include folder in the OpenAL SDK folder.

Building - Linux (Ubuntu)
=========================
Commandline
-----------
- Download the source
- sudo apt-get install build-essential libopenal-dev libwxgtk2.8-dev libwxgtk2.8-dbg python-markdown
- Download and install the cmake 2.8 .debs for your platform from: <https://launchpad.net/ubuntu/+source/cmake>.  You will need cmake-data, and cmake-2.8.0* for your platform, plus cmake-curses-gui or cmake-qt-gui
- cd <source directory>
- mkdir build
- cd build
- cmake -DUSE_OPENAL=1 ../

Building - OS X 10.5 (Leopard)
==============================
Commandline
-----------
- Download the source
- Download and install XCode 3.1 if not already installed, you will need a  Free Apple Developers Center login to download if Xcode in not on your OSX Install Disk.
- Download and install Python 2.6
- Download and install CMake 2.8
- Download and install SDL 1.2 (follow instructions in the SDL ReadMe.txt)
- (Optional) Download and install Mercurial (make sure you have the correct download for your version of OS X, if you get an error about python not being installed but you already have, you downloaded the version of Mercurial for your version of OS X)
- Run CMake ( either using cmake or ccmake in the source directory or use Spotlight to find the CMake GUI)
- Choose XCode as your generator.

License
=======
See License.txt.
