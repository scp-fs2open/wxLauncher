The coding style for wxLauncher is split
into three parts, one for the C/C++
portion of the code, one for the python 
code, and one for the online help
manual(Markdown) portion of code.

All code
========
All code for wxLauncher should have 
a line feed (LF) (ASCII 0x0A)
as the line endings in all files. This
line ending style is also known as the
Unix style line ending.  This means
all linux and OSX coders should have:
    [hooks]
    pretxncommit.crlf = python:hgext.win32text.forbidcrlf
    [patch]
    eol = lf
as in their .hgrc files. The setting 
can be set in either the ~/.hgrc or
in the <wxlauncher dir>/.hg/.hgrc.
Either location will work and will cause
mercurial to automaticilly convert line
endings.

All windows users should set:
    [hooks]
    # make sure that crlf's do not get committed.
    pretxncommit.crlf = python:hgext.win32text.forbidcrlf
    [patch]
    eol = lf
in their Mercurial.ini file (located
in the user profile) or in the 
<wxlauncher dir>/.hg/.hgrc file.
This setting allows mercurial to make 
sure that any incoming files with the
wrong line endings will be corrected
quickly.

Tabs are to be set two spaces and to
be auto converted to spaces.  This
is espcially important when editing
the Python code.

Help Manual
===========
For the coding style and language style
for the help manual see the ReadMe.txt
in the onlinehelp folder.

Python Code
===========
Python coding style is to follow PEP 8
found on the python website at:
<http://www.python.org/dev/peps/pep-0008/>.

C/C++ Code
==========
The C/C++ coding style similar to the
Python and wxWidgets coding style.

-Class names should be CamelCased with
the first letter also capitalized.

-Public functions of classes should also
be CamelCased and start with a capital
letter.

-Event handling functions should be named
On<verb><noun>.

-Other functions should also be
<verb><noun> where possible.

-Private class variables should be
camelCased starting with a lowercase
letter and always referenced via this.
    this->memberVariable = 9;

-Macros must be named with all capital
letters with words separated by
underscores.
