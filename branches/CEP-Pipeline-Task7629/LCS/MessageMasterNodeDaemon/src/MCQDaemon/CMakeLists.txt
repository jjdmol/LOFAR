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
  CQCOmmon.py
  DESTINATION lofar/messagebus)
