# $Id$
include(PythonInstall)


install(PROGRAMS
  MCQDaemon.py
  NCQDaemon.py
  DESTINATION bin)


python_install(
  MCQDaemon.py
  DESTINATION lofar/messagebus)
