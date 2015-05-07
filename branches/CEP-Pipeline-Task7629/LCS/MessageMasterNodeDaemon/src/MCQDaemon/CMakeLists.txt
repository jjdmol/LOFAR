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
  CQCommon.py
  DESTINATION lofar/messagebus)
  
