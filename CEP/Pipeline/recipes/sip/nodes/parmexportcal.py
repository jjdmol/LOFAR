#                                                         LOFAR IMAGING PIPELINE
#
#                                    Node recipe to export calibration solutions
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import os
import shutil
import sys
import tempfile

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import read_initscript, create_directory
from lofarpipe.support.utilities import catch_segfaults


class ParmExportCal(LOFARnodeTCP):
    
    def run(self, infile, outfile, executable, initscript):
        # Time execution of this job
        with log_time(self.logger):
            if os.path.exists(infile):
                self.logger.info("Processing %s" % infile)
            else:
                self.logger.error(
                    "Instrument model file %s does not exist" % infile
                )
                return 1

        # Check if executable exists and is executable.
        if not os.access(executable, os.X_OK):
            self.logger.error("Executable %s not found" % executable)
            return 1

        # Create output directory (if it doesn't already exist)
        create_directory(os.path.dirname(outfile))
        
        # Initialize environment
        env = read_initscript(self.logger, initscript)

        try:
            temp_dir = tempfile.mkdtemp()
            with CatchLog4CPlus(
                temp_dir,
                self.logger.name + '.' + os.path.basename(infile),
                os.path.basename(executable)
            ) as logger:
                catch_segfaults(
                    [executable, '-in', infile, '-out', outfile],
                    temp_dir,
                    env,
                    logger
                )
        except Exception, excp:
            self.logger.error(str(excp))
            return 1
        finally:
            shutil.rmtree(temp_dir)

        return 0


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(ParmExportCal(jobid, jobhost, jobport).run_with_stored_arguments())
