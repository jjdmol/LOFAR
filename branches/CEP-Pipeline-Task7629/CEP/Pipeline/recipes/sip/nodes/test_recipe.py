#                                                        LOFAR IMAGING PIPELINE
#
#                                                       test_recipe node recipe
#                                                      Wouter Klijn 2015
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------

import os
import shutil
import sys

from lofarpipe.support.lofarnode import LOFARnodeTCP

class test_recipe(LOFARnodeTCP):
    """
    """
    def run(self, files, executable, parset, environment):
        """
        """

        self.environment.update(environment)
        # Time execution of this job
        return 0


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(test_recipe(jobid, jobhost, jobport).run_with_stored_arguments())

