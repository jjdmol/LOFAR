# $Id$

include(PythonInstall)

python_install(
  helpers/__init__.py
  helpers/metadata.py
  helpers/WritableParmDB.py
  helpers/ComplexArray.py
  helpers/MultipartPostHandler.py
  master/__init__.py
  master/bbs_reducer.py
  master/copier.py
  master/dppp.py
  master/gainoutliercorrection.py
  master/get_metadata.py
  master/imager_awimager.py
  master/imager_bbs.py
  master/imager_create_dbs.py
  master/imager_finalize.py
  master/imager_prepare.py
  master/imager_source_finding.py
  master/new_bbs.py
  master/rficonsole.py
  master/setupparmdb.py
  master/setupsourcedb.py
  master/vdsmaker.py
  master/vdsreader.py
  nodes/__init__.py
  nodes/bbs_reducer.py
  nodes/copier.py
  nodes/demix/find_a_team.py
  nodes/dppp.py
  nodes/gainoutliercorrection.py
  nodes/get_metadata.py
  nodes/imager_awimager.py
  nodes/imager_bbs.py
  nodes/imager_create_dbs.py
  nodes/imager_finalize.py
  nodes/imager_prepare.py
  nodes/imager_source_finding.py
  nodes/new_bbs.py
  nodes/rficonsole.py
  nodes/setupparmdb.py
  nodes/setupsourcedb.py
  nodes/vdsmaker.py
  DESTINATION lofarpipe/recipes)

install(PROGRAMS
  bin/calibration_pipeline.py
  bin/msss_calibrator_pipeline.py
  bin/msss_imager_pipeline.py
  bin/msss_target_pipeline.py
  bin/preprocessing_pipeline.py
  bin/startPython.sh
  external/bad_station_detection/asciistats.py
  external/bad_station_detection/statsplot.py
  DESTINATION bin)

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
  skymodels/3C147.skymodel
  skymodels/3C196.skymodel
  skymodels/3C286.skymodel
  skymodels/3C287.skymodel
  skymodels/3C295.skymodel
  skymodels/3C380.skymodel
  skymodels/3C48.skymodel
  skymodels/Ateam_LBA_CC.skymodel
  DESTINATION share/pipeline/skymodels)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/pipeline.cfg
  ${CMAKE_CURRENT_BINARY_DIR}/tasks.cfg
  DESTINATION share/pipeline)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pipeline.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/pipeline.cfg)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tasks.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/tasks.cfg)

