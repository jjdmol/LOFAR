# $Id$
include(PythonInstall)


# install(PROGRAMS
  # MCQDaemon.py
# #  NCQDaemon.py
# #  createbus.sh
  # DESTINATION bin)


python_install(
  MCQDaemon.py
  CQDaemon.py
  CQExceptions.py
  CQCommon.py
#  MCQLib.py
#  NCQLib.py
#  CQConfig.py
#  CQCommon.py
  DESTINATION lofar/messagebus)
  
