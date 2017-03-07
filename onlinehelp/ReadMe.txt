What is this?
=============
This folder contains the Online help for wxLauncher.  This online help is for the users and FSO modders.  The documentation of the launcher's technical details can be found in the code itself, the /docs folder, the wxLauncher wiki or the doxygen generated help.

Technical details
=================
The contents of this folder are "source" files that are used to generate the online help for the launcher.  The help generated from this "source" is also transformed into a format suitable for the wxLauncher wiki.

Each folder is the name of a subsection in the table of contents, or optionally the contents of the 'title' tags from the index.help files will be used as the name of the subsection.

Every .help file will be a page in the help document.  index.help will be the default page for each book and subsection of the book.  The contents of html tag "title" will be the name of the page in the table of contents.  The .help files are assumed to be [1][markdown markup].
  [1]: http://daringfireball.net/projects/markdown/syntax

Note that any links in the html files should be relative such that if you were to open the .help file as an .html document in a browser the link would work. Though because they can be in markdown, the browser may not do anything sensible with them in this case.

Both html and markdown image links are allowed in the .help source.  If the image will be outside of the online help archive, the file is copied to a random name within the archive and the image tag changed so that it will work correctly.  For the wiki output the all tags are changed so that they will load correctly on the wiki.

The online help compiler also recongizes meta tags that have a key "name" with a value of "control".  These tags are recongized as anchors from the file for to the control for the context help system.  The tag is also required to have key called "content" with a value that is the name of the ID of a control in the launcher proper.  When a tag is encountered the help system will generate a link between the control that has an ID by that name.

NOTE: The correctness of the ID name given is not checked by the complier of the online help. The names given are only checked by the c++ compiler when the interface file is compiled into the launcher itself.
NOTE: The list of valid names is found in ids.h, in the enum WindowIDS.
