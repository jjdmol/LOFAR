# $Id$

include(PythonInstall)

python_install(
  master/bbs.py
  master/cimager.py
  master/cep2_datamapper.py
  master/compression_pipeline.py
  master/count_timesteps.py
  master/datamapper.py
  master/demixing.py
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
  nodes/demixing.py
  nodes/dppp.py
  nodes/flag_baseline.py
  nodes/make_flaggable.py
  nodes/parmdb.py
  nodes/rficonsole.py
  nodes/sourcedb.py
  nodes/vdsmaker.py
  nodes/demix/demixing.py
  nodes/demix/find_a_team.py
  nodes/demix/median_filter.py
  nodes/demix/shiftphasecenter.py
  nodes/demix/smoothdemix.py
  nodes/demix/subtract_from_averaged.py
  DESTINATION lofarpipe/recipes)

install(FILES
  pipeline.cfg
  tasks.cfg
  DESTINATION share)

