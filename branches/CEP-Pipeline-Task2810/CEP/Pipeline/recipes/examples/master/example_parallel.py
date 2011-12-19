#                                                         LOFAR IMAGING PIPELINE
#
#                                    Example recipe with simple job distribution
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
#
#                                                Fixed example script
#                                                             Wouter Klijn, 2012
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------

import sys
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob

class example_parallel(BaseRecipe, RemoteCommandRecipeMixIn):
    def go(self):
        self.inputs["job_name"] = "example_parallel"              
        super(example_parallel, self).go()
        node_name = "locus040"
        print("\n\n\n\n***********************************************************")
        print("Run script with option -v (verbose) or -d (debug) to validate functioning of the node script")
        print("***********************************************************\n\n\n\n")
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))

        job = ComputeJob(node_name, node_command, arguments=["CommandLineArgument"])
        self._schedule_jobs([job])
        if self.error.isSet():
            return 1
        else:
            return 0

if __name__ == "__main__":
    sys.exit(example_parallel().main())
