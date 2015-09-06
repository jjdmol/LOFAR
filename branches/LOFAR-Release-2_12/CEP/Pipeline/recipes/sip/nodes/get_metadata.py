#                                                         LOFAR IMAGING PIPELINE
#
#                                                      get_metadata: node script
#                                                             Marcel Loose: 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import log_time
from lofarpipe.recipes.helpers import metadata

import os
import sys

class get_metadata(LOFARnodeTCP):
    """
    Get the metadata from the given data product and return it to the master
    using self.outputs.
    """
    def run(self, infile, product_type):
        with log_time(self.logger):
            if os.path.exists(infile):
                self.logger.info("Processing %s" % (infile))
            else:
                self.logger.error("Dataset %s does not exist" % (infile))
                return 1

            self.logger.debug("Product type: %s" % product_type)
            if product_type == "Correlated":
                self.outputs = metadata.Correlated(self.logger, infile).data()
            elif product_type == "InstrumentModel":
                self.outputs = metadata.InstrumentModel(self.logger, 
                                                        infile).data()
            elif product_type == "SkyImage":
                self.outputs = metadata.SkyImage(self.logger, infile).data()
            else:
                self.logger.error("Unknown product type: %s" % product_type)
                return 1

            return 0

if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(get_metadata(jobid, jobhost, jobport).run_with_stored_arguments())
