These .py files are scripts that build help build the files required for wxLauncher.

== version.cpp.maker.py ==
Generates version.cpp. Only requires that mecurial (hg) is in system path.

== onlinehelpmaker.py ==
Generates the online (linked in with the wxLauncher) help for wxLauncher.  Has several dependencies:

 - http://pypi.python.org/pypi/setuptools
 - http://www.freewisdom.org/projects/python-markdown/Installation