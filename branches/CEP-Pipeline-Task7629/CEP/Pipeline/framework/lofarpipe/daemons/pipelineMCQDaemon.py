#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
import lofarpipe.daemons.pipelineMCQDaemonImp as pipelineMCQDaemon
"""
TODO: Config parser:

    def _read_config(self):
        # If a config file hasn't been specified, use the default
        if not self.inputs.has_key("config"):
            # Possible config files, in order of preference:
            conf_locations = (
                os.path.join(sys.path[0], 'pipeline.cfg'),
                os.path.join(os.path.expanduser('~'), '.pipeline.cfg')
            )
            for path in conf_locations:
                if os.access(path, os.R_OK):
                    self.inputs["config"] = path
                    break
            if not self.inputs.has_key("config"):
                raise PipelineException("Configuration file not found")

        config = ConfigParser({
            "job_name": self.inputs["job_name"],
            "start_time": self.inputs["start_time"],
            "cwd": os.getcwd()
        })
        print >> sys.stderr, "Reading configuration file: %s" % \
                              self.inputs["config"]
        config.read(self.inputs["config"])
        return config

https://docs.python.org/2/library/configparser.html

[DEFAULT]
lofarroot = @CMAKE_INSTALL_PREFIX@
casaroot = @CASACORE_ROOT_DIR@
pyraproot = @PYRAP_ROOT_DIR@
hdf5root = $ENV{HDF5_ROOT}
wcsroot = @WCSLIB_ROOT_DIR@
pythonpath = @PYTHON_INSTALL_DIR@
runtime_directory = %(lofarroot)s/var/run/pipeline
recipe_directories = [%(pythonpath)s/lofarpipe/recipes]
working_directory = /data/scratch/$ENV{USER}
task_files = [%(lofarroot)s/share/pipeline/tasks.cfg]

[layout]
job_directory = %(runtime_directory)s/%(job_name)s

[cluster]
clusterdesc = %(lofarroot)s/share/cep2.clusterdesc

[deploy]
engine_ppath = %(pythonpath)s:%(pyraproot)s/lib:/opt/cep/pythonlibs/lib/python/site-packages
engine_lpath = %(lofarroot)s/lib:%(casaroot)s/lib:%(pyraproot)s/lib:%(hdf5root)s/lib:%(wcsroot)s/lib

[logging]
log_file = %(runtime_directory)s/%(job_name)s/logs/%(start_time)s/pipeline.log
xml_stat_file = %(runtime_directory)s/%(job_name)s/logs/%(start_time)s/statistics.xml

[feedback]
# Method of providing feedback to LOFAR.
# Valid options:
#    messagebus    Send feedback and status using LCS/MessageBus
#    none          Do NOT send feedback and status
method = @PIPELINE_FEEDBACK_METHOD@

"""
if __name__ == "__main__":
    # TODO: Read these parameters from a config file
    # TODO: Daemon parameter in the init should be removed
    broker = "locus102"
    busname = "testmcqdaemon"
    masterCommandQueueName = busname + "/" + "pipelineMasterCommandQueue"
    deadLetterQueueName = "testmcqdaemon.deadletter"
    deadletter_log_location = \
    "/home/klijn/build/7629/gnu_debug/installed/var/log/pipelineMCQDaemonDeadletter.log"



    daemon = pipelineMCQDaemon.PipelineMCQDaemon(broker, busname, masterCommandQueueName,
                               deadLetterQueueName, deadletter_log_location, 1, True)

    daemon.run()


