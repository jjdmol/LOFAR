#                                                          LOFAR PIPELINE SCRIPT
#
#                                           running an executable with arguments
#                                                         Stefan Froehlich, 2014
#                                                      s.froehlich@fz-juelich.de
# ------------------------------------------------------------------------------

from __future__ import with_statement
from subprocess import CalledProcessError
import os
import shutil
import sys

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import LOFARnodeTCP


class executable_args(LOFARnodeTCP):
    """
    Basic script for running an executable with arguments.
    """

    def run(self, infile, outfile,
            executable, arguments, work_dir, environment):
        """
        This function contains all the needed functionality
        """
        # Debugging info
        self.logger.debug("infile          = %s" % infile)
        self.logger.debug("outfile         = %s" % outfile)
        self.logger.debug("executable      = %s" % executable)
        self.logger.debug("working directory = %s" % work_dir)
        self.logger.debug("arguments       = %s" % arguments)
        self.logger.debug("environment     = %s" % environment)

        self.environment.update(environment)

        # Time execution of this job
        with log_time(self.logger):
            if os.path.exists(infile):
                self.logger.info("Processing %s" % (infile))
            else:
                self.logger.error("Dataset %s does not exist" % (infile))
                return 1

            # Check if executable is present
            if not os.access(executable, os.X_OK):
                self.logger.error("Executable %s not found" % executable)
                return 1

            # *****************************************************************
            # Perform house keeping, test if work is already done
            # If input and output files are different, and if output file
            # already exists, then we're done.
            if os.path.exists(outfile):
                self.logger.info(
                    "Output file %s already exists. We're done." % outfile
                )
                self.outputs['ok'] = True
                return 0

            try:
            # ****************************************************************
            # Run
                cmd = [executable] + arguments
                with CatchLog4CPlus(
                    work_dir,
                    self.logger.name + "." + os.path.basename(infile),
                    os.path.basename(executable),
                ) as logger:
                    # Catch segfaults and retry
                    catch_segfaults(
                        cmd, work_dir, self.environment, logger
                    )
            except CalledProcessError, err:
                # CalledProcessError isn't properly propagated by IPython
                self.logger.error(str(err))
                return 1
            except Exception, err:
                self.logger.error(str(err))
                return 1

        # We need some signal to the master script that the script ran ok.
        self.outputs['ok'] = True
        return 0

if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(executable_args(jobid, jobhost, jobport).run_with_stored_arguments())
