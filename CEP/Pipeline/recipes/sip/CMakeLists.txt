# $Id$

include(PythonInstall)

python_install(
  master/bbs.py
  master/cimager.py
  master/count_timesteps.py
  master/datamapper.py
  master/flag_baseline.py
  master/make_flaggable.py
  master/new_dppp.py
  master/new_vdsmaker.py
  master/parmdb.py
  master/rficonsole.py
  master/skymodel.py
  master/sourcedb.py
  master/storagemapper.py
  master/vdsreader.py
  nodes/bbs.py
  nodes/cimager.py
  nodes/count_timesteps.py
  nodes/dppp.py
  nodes/flag_baseline.py
  nodes/make_flaggable.py
  nodes/parmdb.py
  nodes/rficonsole.py
  nodes/sourcedb.py
  nodes/vdsmaker.py
  DESTINATION lofarpipe/recipes)