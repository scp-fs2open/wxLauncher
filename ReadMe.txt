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
For more info about the builds on specific system see
the readme.txt that is in the build systems folder.  All
building code is in the build directory.

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

The OpenAL Software Development Kit is an optional component
needed to build this program.  OpenAL support is not
compiled in by default and requires the preprocessor
symbol USE_OPENAL=1.  See the compiler specific
instructions for getting your compiler ready to build
with OpenAL.
OpenAL homepage:
	http://connect.creativelabs.com/openal/default.aspx

The Microsoft Windows SDK (formorly the Platform SDK) is
required to build both this application and wxWidgets when
 on windows.  Note that only the Windows SDK for Windows
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
or TortiseHG:
	http://tortoisehg.bitbucket.org/

Once Mercurial is installed, you can get a copy of the source
by runningthe following command (in a folder of your choice):
	hg clone https://wxlauncher.googlecode.com/hg/ wxlauncher  
  
Building
========
Define USE_SPEECH=1 to have the launcher use the Windows 
Speech API.
Define USE_JOYSTICK=1 to have the launcher setup and configure
Joysticks using (currently) the window MMSystem API.
Define USE_OPENAL=1 to have the launcher allow setup and
testing of the OpenAL api and installation.

License
=======
wxLauncher is Free Software, released under the 
GNU General Public License, version 2.
You should have one copy of the license downloaded with the
source (as this readme file). If it's missing, you can find
a copy of the license at:
	http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
	
Please add it back to the source package ;)