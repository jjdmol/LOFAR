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

#            # Get the product metadata. If data product type was not specified,
#            # derive it from the input filename's extension.
#            if not product_type:
#                ext = os.path.splitext(infile)[1]
#                if ext == ".MS": product_type = "Correlated"
#                elif ext == ".INST": product_type = "InstrumentModel"
#                elif ext == ".IM": product_type = "SkyImage"
#            if not product_type:
#                self.logger.error("File %s has unknown product type" % infile)
#                return 1

            self.logger.debug("Product type: %s" % product_type)
            try:
#                self.outputs[product_type] = (
                self.outputs = (
                    metadata.data_product[product_type](infile).data()
                )
            except KeyError:
                self.logger.error("Unknown product type: %s" % product_type)
                return 1

            return 0

if __name__ == "__main__":
    #   If invoked directly, parse command line arguments for logger information
    #                        and pass the rest to the run() method defined above
    # --------------------------------------------------------------------------
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(get_metadata(jobid, jobhost, jobport).run_with_stored_arguments())
