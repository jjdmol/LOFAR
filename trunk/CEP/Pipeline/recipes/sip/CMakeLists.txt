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
  master/dppp.py
  master/flag_baseline.py
  master/make_flaggable.py
  master/new_bbs.py
  master/parmdb.py
  master/parmexportcal.py
  master/rficonsole.py
  master/skymodel.py
  master/sourcedb.py
  master/storagemapper.py
  master/vdsmaker.py
  master/vdsreader.py
  master/__init__.py
  nodes/bbs.py
  nodes/cimager.py
  nodes/count_timesteps.py
  nodes/demixing.py
  nodes/dppp.py
  nodes/flag_baseline.py
  nodes/make_flaggable.py
  nodes/new_bbs.py
  nodes/parmdb.py
  nodes/parmexportcal.py
  nodes/rficonsole.py
  nodes/sourcedb.py
  nodes/vdsmaker.py
  nodes/demix/demixing.py
  nodes/demix/find_a_team.py
  nodes/demix/median_filter.py
  nodes/demix/shiftphasecenter.py
  nodes/demix/smoothdemix.py
  nodes/demix/subtract_from_averaged.py
  nodes/imager_create_dbs.py
  nodes/__init__.py
  DESTINATION lofarpipe/recipes)

install(PROGRAMS
  bin/msss_calibrator_pipeline.py
  bin/msss_target_pipeline.py
  bin/startPython.sh
  DESTINATION bin)

install(FILES
  pipeline.cfg
  tasks.cfg
  DESTINATION share/pipeline)

install(FILES
  demixing/bbs_CasA.parset
  demixing/bbs_CasA_smoothcal.parset
  demixing/bbs_CygA.parset
  demixing/bbs_CygA_smoothcal.parset
  demixing/bbs_HerA.parset
  demixing/bbs_HerA_smoothcal.parset
  demixing/bbs_HydA.parset
  demixing/bbs_HydA_smoothcal.parset
  demixing/bbs_TauA.parset
  demixing/bbs_TauA_smoothcal.parset
  demixing/bbs_VirA.parset
  demixing/bbs_VirA_smoothcal.parset
  DESTINATION share/pipeline/demixing)

install(FILES
  skymodels/3C196.skymodel
  skymodels/3C48.skymodel
  skymodels/Ateam_LBA_CC.skymodel
  DESTINATION share/pipeline/skymodels)

