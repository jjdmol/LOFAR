//////////////////////////////////////////////////////////////////////////
//
//  Readme.txt: File describing the way to create your own application
//              "YourFirstApp" from base application EmptyAppl
//
//  $Id$
//
//////////////////////////////////////////////////////////////////////////

Actions necessary to create your own application "YourFirstApp" using 
EmptyAppl:

* Copy EmptyAppl directory and rename to "YourFirstApp". (Don't forget to
  remove the CVS directories!)

* Rename files src/EmptyAppl.h and src/EmptyAppl.cc to src/YourFirstApp.h
  and src/YourFirstApp.cc.

* Replace all occurences of 'EmptyAppl' (in all files in all directories) 
  with 'YourFirstApp'.

* File: bootstrap ->Make sure the path in this file points to the bootstrap
                    script in the autoconf_share directory in the LOFAR tree.

* File: src/YourFirstApp.cc ->Define your own application (create WorkHolders
                              and steps, add these to simul, make connections
                              etc.)

* File: src/Makefile.am ->Declare any extra files (e.g. your own WorkHolder
                          files) in this makefile
