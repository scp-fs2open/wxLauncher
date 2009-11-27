This folder contains the Online help for wxLauncher.  This online help is for the users.  The contents of this folder are "source" files that are used to generate the online help for the launcher.

Each folder is the name of a subsection in the table of contents.

Every .help file will be a page in the help document.  index.hlp will be the default page for each book and subsection of the book.  The contents of html tag "title" will be the name of the page in the table of contents.  The .help files are assumed to be markdown markup: <http://daringfireball.net/projects/markdown/syntax>.

Note that any links in the html files should be relative such that if you were to open the .help file as an .html document in a browser the link would work. Though because they can be in markdown, the browser may not do anything sensible with them in this case.