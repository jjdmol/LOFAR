#                                                        LOFAR IMAGING PIPELINE
#
#                                                       test_recipe node recipe
#                                                      Wouter Klijn 2015
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------

import os
import shutil
import sys
import time

from lofarpipe.support.lofarnode import LOFARnodeTCP

# Includes for QPID framework, might not be available. Set status flag 
_QPID_ENABLED = False
try:
    import lofar.messagebus.NCQLib as NCQLib
    import lofar.messagebus.NCQLib.QPIDNode as  LOFARnodeTCP
    _QPID_ENABLED = True
except:
    pass
# End QPID include 

class test_recipe(LOFARnodeTCP):
    """
    """
    def run(self, argument):
        """
        """
        self.logger.critical("##########################We are in the test recipe and we are going good#####################")


        #self.logger.error(argument)
        ##self.environment.update(environment)

        self.outputs["output"] = "*********************************This is output created on the local node ***************************"
        self.outputs["status"] = True
        time.sleep(6)
        ## Time execution of this job
        return 0


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------



    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(test_recipe(jobid, jobhost, jobport).run_with_stored_arguments())

