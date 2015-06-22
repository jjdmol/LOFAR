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
import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

class test_recipe(LOFARnodeTCP):
    """
    """
    def run(self, argument):
        """
        """
        self.logger.critical("#####We are in the test recipe and we are going good#####")


        #self.logger.error(argument)
        ##self.environment.update(environment)

        self.outputs["output"] = "OUPUT FROM TEST RECIPE"
        self.outputs["status"] = True
        ## Time execution of this job
        return 0


if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(test_recipe(jobid, jobhost, jobport).run_with_stored_arguments())


    

