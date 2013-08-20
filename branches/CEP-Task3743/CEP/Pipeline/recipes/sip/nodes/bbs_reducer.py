#                                                         LOFAR IMAGING PIPELINE
#
#                                   BBS reducer (BlackBoard Selfcal) node recipe
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import shutil
import sys
from subprocess import CalledProcessError
from tempfile import mkdtemp

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import log_time
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.utilities import catch_segfaults

class bbs_reducer(LOFARnodeTCP):
    """
    Handle running the bbs-reducer executable.
    """
    def run(self, files, executable, parset, environment):
        """
        Run the bbs-reducer executable. 
        *Arguments*
        - `files`: argument is a tuple of (MS-file, parmdb-file, sourcedb-file)
        - `executable`: full path to the bbs-reducer executable
        - `parset`: full path to the parset-file
        - `environment`: environment variables to use
        """
        self.logger.debug("files       = %s" % str(files))
        self.logger.debug("executable  = %s" % executable)
        self.logger.debug("parset      = %s" % parset)
        self.logger.debug("environment = %s" % environment)
        
        self.environment.update(environment)
        ms, parmdb, sourcedb = files
        
        # Time execution of this job
        with log_time(self.logger):
            if os.path.exists(ms):
                self.logger.info("Processing %s" % ms)
            else:
                self.logger.error("Measurement Set %s does not exist" % ms)
                return 1

            # Run bbs-reducer. Catch log output from bbs-reducer and stdout.
            scratch_dir = mkdtemp()
            try:
                cmd = [executable,
                       "--parmdb=%s" % parmdb, 
                       "--sourcedb=%s" % sourcedb,
                       ms, parset
                      ]
                with CatchLog4CPlus(
                    scratch_dir,
                    self.logger.name + "." + os.path.basename(ms),
                    os.path.basename(executable),
                ) as logger:
                    catch_segfaults(cmd, scratch_dir, self.environment, logger)
            except CalledProcessError, err:
                self.logger.error(str(err))
                return 1
            finally:
                shutil.rmtree(scratch_dir)

        return 0


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(bbs_reducer(jobid, jobhost, jobport).run_with_stored_arguments())

