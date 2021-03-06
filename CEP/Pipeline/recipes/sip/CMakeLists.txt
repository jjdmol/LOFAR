# $Id$

include(PythonInstall)

python_install(
  helpers/__init__.py
  helpers/metadata.py
  helpers/WritableParmDB.py
  helpers/ComplexArray.py
  helpers/MultipartPostHandler.py
  helpers/data_quality.py
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
  master/long_baseline.py
  master/new_bbs.py
  master/rficonsole.py
  master/setupparmdb.py
  master/setupsourcedb.py
  master/vdsmaker.py
  master/vdsreader.py
  master/selfcal_awimager.py
  master/selfcal_bbs.py
  master/selfcal_finalize.py
  master/executable_args.py
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
  nodes/long_baseline.py
  nodes/new_bbs.py
  nodes/rficonsole.py
  nodes/setupparmdb.py
  nodes/setupsourcedb.py
  nodes/vdsmaker.py
  nodes/selfcal_awimager.py
  nodes/selfcal_bbs.py
  nodes/selfcal_finalize.py
  nodes/executable_args.py
  nodes/executable_casa.py
  nodes/python_plugin.py
  nodes/calibrate-stand-alone.py
  plugins/PipelineStep_createMapfile.py
  plugins/PipelineStep_addMapfile.py
  plugins/PipelineStep_changeMapfile.py
  plugins/PipelineStep_combineParsets.py
  DESTINATION lofarpipe/recipes)

lofar_add_bin_scripts(
  bin/calibration_pipeline.py
  bin/msss_calibrator_pipeline.py
  bin/msss_imager_pipeline.py
  bin/msss_target_pipeline.py
  bin/preprocessing_pipeline.py
  bin/imaging_pipeline.py
  bin/pulsar_pipeline.py
  bin/long_baseline_pipeline.py
  bin/selfcal_imager_pipeline.py
  bin/pipelineAborted.sh
  bin/runPipeline.sh
  bin/startPython.sh
  bin/startPythonVersion.sh
  bin/stopPython.sh
  bin/genericpipeline.py
  bin/loader.py
  external/bad_station_detection/asciistats.py
  external/bad_station_detection/statsplot.py)

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
  skymodels/CygA.skymodel
  DESTINATION share/pipeline/skymodels)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/pipeline.cfg
  ${CMAKE_CURRENT_BINARY_DIR}/pipeline.cfg.CEP4
  ${CMAKE_CURRENT_BINARY_DIR}/tasks.cfg
  ${CMAKE_CURRENT_BINARY_DIR}/tasks.cfg.CEP4
  DESTINATION share/pipeline)

# PIPELINE_FEEDBACK_METHOD is used in pipeline.cfg.in to enable/disable qpid feedback
if(HAVE_QPID)
  set(PIPELINE_FEEDBACK_METHOD "messagebus")
else(HAVE_QPID)
  set(PIPELINE_FEEDBACK_METHOD "none")
endif(HAVE_QPID)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/pipeline.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/pipeline.cfg)

# Convert configuration files through docker-template
foreach(_file pipeline.cfg.CEP4)
  # _src -> _dst
  set(_src ${CMAKE_CURRENT_SOURCE_DIR}/${_file}.tmpl)
  set(_dst ${CMAKE_CURRENT_BINARY_DIR}/${_file})

  # add generating command, and (any) target to force the generation
  # when "all" is build.
  add_custom_command(
    OUTPUT ${_dst}
    COMMAND ${CMAKE_SOURCE_DIR}/Docker/docker-template -v ${CMAKE_BINARY_DIR}/Docker/versiondocker < ${_src} > ${_dst}
    DEPENDS ${CMAKE_SOURCE_DIR}/Docker/docker-template ${_src} ${CMAKE_BINARY_DIR}/Docker/versiondocker
  )
  add_custom_target(${_file}_target ALL DEPENDS ${_dst})

  # install resulting file
  install(FILES
    ${_dst}
    DESTINATION share/pipeline
    RENAME Dockerfile
  )
endforeach()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tasks.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/tasks.cfg)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/tasks.cfg.CEP4.in
  ${CMAKE_CURRENT_BINARY_DIR}/tasks.cfg.CEP4)

