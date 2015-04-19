# $Id$

include(PythonInstall)

python_install(
  __init__.py
  taskfeedbackdataproducts.py
  taskfeedbackprocessing.py
  taskfeedbackstate.py
  DESTINATION lofar/messagebus/protocols)
