# $Id: CMakeLists.txt 21998 2012-09-12 07:40:19Z rafferty $

include(PythonInstall)

python_install(
  class_obsPreprocessing.py
  class_mergesubbands.py
  class_obsparMergedData.py
  class_obspar.py
  class_selfcalparam.py
  class_selfcalrun.py
  class_dataInit.py
  class_selfcalPeeling.py 
  old_class_obsparMergedData.py
  old_class_selfcalparam.py
  old_class_selfcalrun.py
  LSM_module
  DESTINATION lofar/selfcal)

install(PROGRAMS
  selfcal.py
  mergeSB.py 
  old_selfcal.py
  DESTINATION bin)
