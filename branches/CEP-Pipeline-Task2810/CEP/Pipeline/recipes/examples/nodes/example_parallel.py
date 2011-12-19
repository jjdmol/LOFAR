#                                                         LOFAR IMAGING PIPELINE
#
#                                                Example of a simple node script
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
#
#                                                Fixed example script
#                                                             Wouter Klijn, 2012
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------

import sys
from lofarpipe.support.lofarnode import LOFARnodeTCP

class example_parallel(LOFARnodeTCP):
    def run(self, *args):        
        log4CPlusName = "example_parallel" 
        command_line_argument = args[0]  
            
        self.logger.info(log4CPlusName)
        self.logger.info(command_line_argument)

        return 0

if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(example_parallel(jobid, jobhost, jobport).run_with_stored_arguments())
