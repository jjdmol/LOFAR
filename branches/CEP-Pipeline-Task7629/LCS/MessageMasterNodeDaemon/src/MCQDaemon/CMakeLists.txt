# $Id$
include(PythonInstall)


install(PROGRAMS
  MCQDaemon.py
  NCQDaemon.py
  DESTINATION bin)


python_install(
  MCQLib.py
  NCQLib.py
  CQConfig.py
  DESTINATION lofar/messagebus)
