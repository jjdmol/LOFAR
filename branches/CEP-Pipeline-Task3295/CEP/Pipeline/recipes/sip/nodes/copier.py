#                                                         LOFAR IMAGING PIPELINE
#
#                                                        Copy from to nodes
#                                                             Wouter Klijn, 2012
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import os
import shutil

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import create_directory


class Copier(LOFARnodeTCP):

    def run(self, infile, outfile, executable, initscript):
        # Time execution of this job
        with log_time(self.logger):
            pass

        # Create output directory (if it doesn't already exist)
        create_directory(os.path.dirname(outfile))

        # Initialize environment
        env = read_initscript(self.logger, initscript)

        try:
            pass # the work
#        except Exception, excp:
#            self.logger.error(str(excp))
#            return 1
        finally:
            pass

        return 0


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(Copier(jobid, jobhost, jobport).run_with_stored_arguments())
