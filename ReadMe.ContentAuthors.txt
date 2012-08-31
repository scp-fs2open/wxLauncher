** This document is indented for content authors and contains
instructions about how wxLauncher can help them. If you just
want to play FS2 Open see the main ReadMe.txt. **

Introduction
============
wxLauncher aims to give one unified answer to members of the
FreeSpace 2 Open modding community looking for an easy way
to give your users an easy to use launcher (and installer,
once that happens), as well as providing you with some useful
tools for making your life easier by reducing your cross platform
install issues. wxLauncher can also make your life easier
for working with FSO and jumping back and forth between different
content that you have being developed or completed.

Features
========
- Add a profile to wxLauncher from a file.
- Select the profile that wxLauncher will use on next startup.

A note about wxLaunchers commandline
====================================
There are three types of commandline options to wxLauncher.
A switch:
    Enables specific functionality just from its presence.
A operator:
    Causes wxLauncher to act (normally without showing the GUI)
    upon its internal data structures. Used with /OPERANDS/ to
    provide the targets or sources for operations.
An operand:
    Used with operators to specify what to act upon.

Adding profiles programmatically
============================
This is done with the --add-profile commandline operator. Requires
two Operands:
--profile - Contains name of the profile to add
--file    - Contains the fully qualified path to a profile
    to import.

This function will not overwrite an existing profile of the same
name.

wxLauncher developer note: this function behaves as if the
user asked wxLauncher to clone the profile that is contained
in the file specified.

Changing the default profile programmatically
=========================================
This is done with the --select-profile commandline operator.
It requires one Operand
--profile - The name of the profile to switch to

This function will cause wxLauncher to change the default profile
to the specified profile.


