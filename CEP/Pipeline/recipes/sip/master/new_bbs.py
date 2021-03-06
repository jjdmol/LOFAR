#                                                        LOFAR IMAGING PIPELINE
#
#                                               BBS (BlackBoard Selfcal) recipe
#                                                        John Swinbank, 2009-10
#                                                     swinbank@transientskp.org
#                                                    Wouter Klijn, 2012
#                                                            klijn@astron.nl
# -----------------------------------------------------------------------------

from __future__ import with_statement
import subprocess
import sys
import os
import threading
import tempfile
import shutil
import time
import signal

from lofar.parameterset import parameterset

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_process_output
from lofarpipe.support.remotecommand import run_remote_command
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.jobserver import job_server
import lofarpipe.support.utilities as utilities
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.utilities import create_directory


class new_bbs(BaseRecipe):
    """
    **This bbs recipe still uses the oldstyle bbs with global control**
    **New versions will have stand alone capability**

    The bbs recipe coordinates running BBS on a group of MeasurementSets. It
    runs both GlobalControl and KernelControl; as yet, SolverControl has not
    been integrated.

    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'control_exec': ingredient.ExecField(
            '--control-exec',
            dest="control_exec",
            help="BBS Control executable"
        ),
        'kernel_exec': ingredient.ExecField(
            '--kernel-exec',
            dest="kernel_exec",
            help="BBS Kernel executable"
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            dest="parset",
            help="BBS configuration parset"
        ),
        'db_key': ingredient.StringField(
            '--db-key',
            dest="db_key",
            help="Key to identify BBS session"
        ),
        'db_host': ingredient.StringField(
            '--db-host',
            dest="db_host",
            help="Database host with optional port (e.g. ldb001:5432)"
        ),
        'db_user': ingredient.StringField(
            '--db-user',
            dest="db_user",
            help="Database user"
        ),
        'db_name': ingredient.StringField(
            '--db-name',
            dest="db_name",
            help="Database name"
        ),
        'instrument_mapfile': ingredient.FileField(
            '--instrument-mapfile',
            help="Full path to the mapfile containing the names of the "
                 "instrument model files generated by the `parmdb` recipe"
        ),
        'sky_mapfile': ingredient.FileField(
            '--sky-mapfile',
            help="Full path to the mapfile containing the names of the "
                 "sky model files generated by the `sourcedb` recipe"
        ),
        'data_mapfile': ingredient.StringField(
            '--data-mapfile',
            help="Full path to the mapfile containing the names of the "
                 "data files that were processed by BBS (clobbered if exists)"
        ),
        'gvds': ingredient.StringField(
            '-g', '--gvds',
            help="Path for output GVDS file"
        )
    }
    outputs = {
        'mapfile': ingredient.FileField(
            help="Full path to a mapfile describing the processed data"
        )
    }

    def __init__(self):
        super(new_bbs, self).__init__()
        self.bbs_map = list()
        self.parset = parameterset()
        self.killswitch = threading.Event()

    def _set_input(self, in_key, ps_key):
        """
        Set the input-key `in_key` to the value of `ps_key` in the parset, if
        that is defined.
        """
        try:
            self.inputs[in_key] = self.parset.getString(ps_key)
        except RuntimeError, exceptionobject:
            self.logger.warn(str(exceptionobject))

    def _make_bbs_map(self):
        """
        This method bundles the contents of three different map-files.
        All three map-files contain a list of tuples of hostname and filename.
        The contents of these files are related by index in the list. They
        form triplets of MS-file, its associated instrument model and its
        associated sky model.

        The data structure `self.bbs_map` is a list of tuples, where each
        tuple is a pair of hostname and the aforementioned triplet.

        For example:
        bbs_map[0] = ('locus001',
            ('/data/L29697/L29697_SAP000_SB000_uv.MS',
            '/data/scratch/loose/L29697/L29697_SAP000_SB000_uv.MS.instrument',
            '/data/scratch/loose/L29697/L29697_SAP000_SB000_uv.MS.sky')
        )

        Returns `False` if validation of the three map-files fails, otherwise
        returns `True`.
        """
        self.logger.debug("Creating BBS map-file using: %s, %s, %s" %
                          (self.inputs['args'][0],
                           self.inputs['instrument_mapfile'],
                           self.inputs['sky_mapfile']))
        data_map = load_data_map(self.inputs['args'][0])
        instrument_map = load_data_map(self.inputs['instrument_mapfile'])
        sky_map = load_data_map(self.inputs['sky_mapfile'])

        if not validate_data_maps(data_map, instrument_map, sky_map):
            self.logger.error("Validation of input data mapfiles failed")
            return False

        # Store data mapfile containing list of files to be processed by BBS.
        store_data_map(self.inputs['data_mapfile'], data_map)

        self.bbs_map = [
            (dat[0], (dat[1], ins[1], sky[1]))
            for dat, ins, sky in zip(data_map, instrument_map, sky_map)
        ]

        return True

    def go(self):
        self.logger.info("Starting BBS run")
        super(new_bbs, self).go()

        #                Check for relevant input parameters in the parset-file
        # ---------------------------------------------------------------------
        self.logger.debug("Reading parset from %s" % self.inputs['parset'])
        self.parset = parameterset(self.inputs['parset'])

        self._set_input('db_host', 'BBDB.Host')
        self._set_input('db_user', 'BBDB.User')
        self._set_input('db_name', 'BBDB.Name')
        self._set_input('db_key', 'BBDB.Key')

        #self.logger.debug("self.inputs = %s" % self.inputs)

        #                                         Clean the blackboard database
        # ---------------------------------------------------------------------
        self.logger.info(
            "Cleaning BBS database for key '%s'" % (self.inputs['db_key'])
        )
        command = ["psql",
                   "-h", self.inputs['db_host'],
                   "-U", self.inputs['db_user'],
                   "-d", self.inputs['db_name'],
                   "-c", "DELETE FROM blackboard.session WHERE key='%s';" %
                         self.inputs['db_key']
                  ]
        self.logger.debug(command)
        if subprocess.call(command) != 0:
            self.logger.warning(
                "Failed to clean BBS database for key '%s'" %
                self.inputs['db_key']
            )

        #                  Create a bbs_map describing the file mapping on disk
        # ---------------------------------------------------------------------
        if not self._make_bbs_map():
            return 1

        # Produce a GVDS file, describing the data that must be processed.
        gvds_file = self.run_task(
            "vdsmaker",
            self.inputs['data_mapfile'],
            gvds=self.inputs['gvds']
        )['gvds']

        #      Construct a parset for BBS GlobalControl by patching the GVDS
        #           file and database information into the supplied template
        # ------------------------------------------------------------------
        self.logger.debug("Building parset for BBS control")
        # Create a location for parsets
        job_directory = self.config.get(
                            "layout", "job_directory")
        parset_directory = os.path.join(job_directory, "parsets")
        create_directory(parset_directory)

        # patch the parset and copy result to target location remove tempfile
        try:
            bbs_parset = utilities.patch_parset(
                self.parset,
                {
                    'Observation': gvds_file,
                    'BBDB.Key': self.inputs['db_key'],
                    'BBDB.Name': self.inputs['db_name'],
                    'BBDB.User': self.inputs['db_user'],
                    'BBDB.Host': self.inputs['db_host'],
                    #'BBDB.Port': self.inputs['db_name'],
                }
            )
            bbs_parset_path = os.path.join(parset_directory,
                                           "bbs_control.parset")
            shutil.copyfile(bbs_parset, bbs_parset_path)
            self.logger.debug("BBS control parset is %s" % (bbs_parset_path,))

        finally:
            # Always remove the file in the tempdir
            os.remove(bbs_parset)

        try:
            #        When one of our processes fails, we set the killswitch.
            #      Everything else will then come crashing down, rather than
            #                                         hanging about forever.
            # --------------------------------------------------------------
            self.killswitch = threading.Event()
            self.killswitch.clear()
            signal.signal(signal.SIGTERM, self.killswitch.set)

            #                           GlobalControl runs in its own thread
            # --------------------------------------------------------------
            run_flag = threading.Event()
            run_flag.clear()
            bbs_control = threading.Thread(
                target=self._run_bbs_control,
                args=(bbs_parset, run_flag)
            )
            bbs_control.start()
            run_flag.wait()    # Wait for control to start before proceeding

            #      We run BBS KernelControl on each compute node by directly
            #                             invoking the node script using SSH
            #      Note that we use a job_server to send out job details and
            #           collect logging information, so we define a bunch of
            #    ComputeJobs. However, we need more control than the generic
            #     ComputeJob.dispatch method supplies, so we'll control them
            #                                          with our own threads.
            # --------------------------------------------------------------
            command = "python %s" % (self.__file__.replace('master', 'nodes'))
            jobpool = {}
            bbs_kernels = []
            with job_server(self.logger, jobpool, self.error) as(jobhost,
                                                                   jobport):
                self.logger.debug("Job server at %s:%d" % (jobhost, jobport))
                for job_id, details in enumerate(self.bbs_map):
                    host, files = details
                    jobpool[job_id] = ComputeJob(
                        host, command,
                        arguments=[
                            self.inputs['kernel_exec'],
                            files,
                            self.inputs['db_key'],
                            self.inputs['db_name'],
                            self.inputs['db_user'],
                            self.inputs['db_host']
                        ]
                    )
                    bbs_kernels.append(
                        threading.Thread(
                            target=self._run_bbs_kernel,
                            args=(host, command, job_id, jobhost, str(jobport))
                        )
                    )
                self.logger.info("Starting %d threads" % len(bbs_kernels))
                for thread in bbs_kernels:
                    thread.start()
                self.logger.debug("Waiting for all kernels to complete")
                for thread in bbs_kernels:
                    thread.join()

            #         When GlobalControl finishes, our work here is done
            # ----------------------------------------------------------
            self.logger.info("Waiting for GlobalControl thread")
            bbs_control.join()
        finally:
            os.unlink(bbs_parset)

        if self.killswitch.isSet():
            #  If killswitch is set, then one of our processes failed so
            #                                   the whole run is invalid
            # ----------------------------------------------------------
            return 1

        self.outputs['mapfile'] = self.inputs['data_mapfile']
        return 0

    def _run_bbs_kernel(self, host, command, *arguments):
        """
        Run command with arguments on the specified host using ssh. Return its
        return code.

        The resultant process is monitored for failure; see
        _monitor_process() for details.
        """
        try:
            bbs_kernel_process = run_remote_command(
                self.config,
                self.logger,
                host,
                command,
                self.environment,
                arguments=arguments
            )
        except OSError:
            self.logger.exception("BBS Kernel failed to start")
            self.killswitch.set()
            return 1
        result = self._monitor_process(bbs_kernel_process,
                                       "BBS Kernel on %s" % host)
        sout, serr = bbs_kernel_process.communicate()
        serr = serr.replace("Connection to %s closed.\r\n" % host, "")
        log_process_output("SSH session (BBS kernel)", sout, serr, self.logger)
        return result

    def _run_bbs_control(self, bbs_parset, run_flag):
        """
        Run BBS Global Control and wait for it to finish. Return its return
        code.
        """
        self.logger.info("Running BBS GlobalControl")
        working_dir = tempfile.mkdtemp(suffix=".%s" % (os.path.basename(__file__),))
        with CatchLog4CPlus(
            working_dir,
            self.logger.name + ".GlobalControl",
            os.path.basename(self.inputs['control_exec'])
        ):
            with utilities.log_time(self.logger):
                try:
                    bbs_control_process = utilities.spawn_process(
                        [
                            self.inputs['control_exec'],
                            bbs_parset,
                            "0"
                        ],
                        self.logger,
                        cwd=working_dir,
                        env=self.environment
                    )
                    # _monitor_process() needs a convenient kill() method.
                    bbs_control_process.kill = lambda : os.kill(
                                    bbs_control_process.pid, signal.SIGKILL)
                except OSError, e:
                    self.logger.error(
                            "Failed to spawn BBS Control (%s)" % str(e))
                    self.killswitch.set()
                    return 1
                finally:
                    run_flag.set()

            returncode = self._monitor_process(
                bbs_control_process, "BBS Control"
            )
            sout, serr = bbs_control_process.communicate()
        shutil.rmtree(working_dir)
        log_process_output(
            self.inputs['control_exec'], sout, serr, self.logger
        )
        return returncode

    def _monitor_process(self, process, name="Monitored process"):
        """
        Monitor a process for successful exit. If it fails, set the kill
        switch, so everything else gets killed too. If the kill switch is set,
        then kill this process off.

        Name is an optional parameter used only for identification in logs.
        """
        while True:
            try:
                returncode = process.poll()
                # Process still running
                if returncode == None:
                    time.sleep(1)

                # Process broke!
                elif returncode != 0:
                    self.logger.warn(
                        "%s returned code %d; aborting run" % (name,
                                                            returncode)
                    )
                    self.killswitch.set()
                    break

                # Process exited cleanly
                else:
                    self.logger.info("%s clean shutdown" % (name))
                    break

                # Other process failed; abort
                if self.killswitch.isSet():
                    self.logger.warn("Killing %s" % (name))
                    process.kill()
                    returncode = process.wait()
                    break

            # Catch All exceptions: we need to take down all processes whatever
            # is throw
            except:
                # An exception here is likely a ctrl-c or similar. Whatever it
                # is, we bail out.
                self.killswitch.set()
        return returncode

if __name__ == '__main__':
    sys.exit(new_bbs().main())
