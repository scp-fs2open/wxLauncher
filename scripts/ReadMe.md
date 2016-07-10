Simple scripts to help with the build of wxLauncher.

version.cpp.maker.py
====================
Generates version.cpp. Only requires that the git source code
management tool (git) is in system path.

onlinehelpmaker.py
==================
Generates the "online" (linked in with the wxLauncher) help for
wxLauncher.  Has several dependencies:

 - [Markdown][markdown] for Python
 - [Six][six] a Python 2 and 3 compatibility library

[markdown]: https://pypi.python.org/pypi/Markdown
[six]: https://pypi.python.org/pypi/six
