#                                                         LOFAR IMAGING PIPELINE
#
#                                                  BBS (BlackBoard Selfcal) node
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from __future__ import with_statement
from subprocess import Popen, CalledProcessError, PIPE, STDOUT
from tempfile import mkstemp, mkdtemp
import os
import sys
import shutil

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import get_mountpoint
from lofarpipe.support.utilities import log_time
from lofarpipe.support.pipelinelogging import log_process_output

from lofar.parameterset import parameterset


class new_bbs(LOFARnodeTCP):
    #                      Handles running a single BBS kernel on a compute node
    # --------------------------------------------------------------------------
    def run(self, executable, infiles, db_key, db_name, db_user, db_host):
        # executable : path to KernelControl executable
        # infiles    : tuple of MS, instrument- and sky-model files
        # db_*       : database connection parameters
        # ----------------------------------------------------------------------
        self.logger.debug("executable = %s" % executable)
        self.logger.debug("infiles = %s" % str(infiles))
        self.logger.debug("db_key = %s" % db_key)
        self.logger.debug("db_name = %s" % db_name)
        self.logger.debug("db_user = %s" % db_user)
        self.logger.debug("db_host = %s" % db_host)

        (ms, parmdb_instrument, parmdb_sky) = infiles

        with log_time(self.logger):
            if os.path.exists(ms):
                self.logger.info("Processing %s" % (ms))
            else:
                self.logger.error("Dataset %s does not exist" % (ms))
                return 1

            #        Build a configuration parset specifying database parameters
            #                                                     for the kernel
            # ------------------------------------------------------------------
            self.logger.debug("Setting up BBSKernel parset")
            # Getting the filesystem must be done differently, using the
            # DataProduct keys in the parset provided by the scheduler.
            filesystem = "%s:%s" % (os.uname()[1], get_mountpoint(ms))
            fd, parset_file = mkstemp()
            kernel_parset = parameterset()
            for key, value in {
                "ObservationPart.Filesystem": filesystem,
                "ObservationPart.Path": ms,
                "BBDB.Key": db_key,
                "BBDB.Name": db_name,
                "BBDB.User": db_user,
                "BBDB.Host": db_host,
                "ParmDB.Sky": parmdb_sky,
                "ParmDB.Instrument": parmdb_instrument
            }.iteritems():
                kernel_parset.add(key, value)
            kernel_parset.writeFile(parset_file)
            os.close(fd)
            self.logger.debug("BBSKernel parset written to %s" % parset_file)

            #                                                     Run the kernel
            #               Catch & log output from the kernel logger and stdout
            # ------------------------------------------------------------------
            working_dir = mkdtemp()
            try:
                self.logger.info("******** {0}".format(open(parset_file).read()))
                cmd = [executable, parset_file, "0"]
                self.logger.debug("Executing BBS kernel")
                with CatchLog4CPlus(
                    working_dir,
                    self.logger.name + "." + os.path.basename(ms),
                    os.path.basename(executable),
                ):
                    bbs_kernel_process = Popen(
                        cmd, stdout = PIPE, stderr = PIPE, cwd = working_dir
                    )
                    sout, serr = bbs_kernel_process.communicate()
                log_process_output("BBS kernel", sout, serr, self.logger)
                if bbs_kernel_process.returncode != 0:
                    raise CalledProcessError(
                        bbs_kernel_process.returncode, executable
                    )
            except CalledProcessError, e:
                self.logger.error(str(e))
                return 1
            finally:
                os.unlink(parset_file)
                shutil.rmtree(working_dir)
            return 0

if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(new_bbs(jobid, jobhost, jobport).run_with_stored_arguments())
