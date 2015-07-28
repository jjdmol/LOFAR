# $Id$

include(PythonInstall)

python_install(
  __init__.py
  cuisine/__init__.py
  cuisine/cook.py
  cuisine/files.py
  cuisine/ingredient.py
  cuisine/job_parser.py
  cuisine/message.py
  cuisine/parset.py
  cuisine/pipeline.py
  cuisine/pipeline_manager_config.py
  cuisine/WSRTrecipe.py
  support/__init__.py
  support/baserecipe.py
  support/clusterdesc.py
  support/control.py
  support/data_map.py
  support/group_data.py
  support/jobserver.py
  support/lofarexceptions.py
  support/lofaringredient.py
  support/lofarnode.py
  support/loggingdecorators.py
  support/mac.py
  support/parset.py
  support/pipelinelogging.py
  support/remotecommand.py
  support/stateful.py
  support/subprocessgroup.py
  support/utilities.py
  support/xmllogging.py
  support/usagestats.py
  daemons/__init__.py
  daemons/pipelineMCQDaemonImp.py
  daemons/pipelineSCQDaemonImp.py
  daemons/subprocessManager.py
  daemons/SCQLib.py
  daemons/MCQLib.py
  DESTINATION lofarpipe)

install(PROGRAMS
  daemons/startMCQDaemons.py
  daemons/pipelineMCQDaemon.py
  daemons/pipelineSCQDaemon.py
  DESTINATION bin)
  

