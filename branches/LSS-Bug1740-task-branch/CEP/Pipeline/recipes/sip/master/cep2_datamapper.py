#                                                         LOFAR IMAGING PIPELINE
#
#                        Generate a mapfile for processing data on storage nodes
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os.path
import sys

from lofar.mstools import findFiles

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.group_data import store_data_map
from lofarpipe.support.utilities import create_directory
import lofarpipe.support.lofaringredient as ingredient

class cep2_datamapper(BaseRecipe):
    """
    Search for a set of MS files on all the CEP-II cluster nodes and generate
    a mapfile suitable for further processing.

    **Arguments**

    observation_dir: full path to the directory to search for MS files.
    mapfile: name of the mapfile to produce.
    """
    inputs = {
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path (including filename) of mapfile to produce "
                 "(clobbered if exists)"
        ),
        'observation_dir': ingredient.StringField(
            '--observation-dir',
            help="Full path to the directory to search for MS files"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="Full path (including filename) of generated mapfile"
        )
    }

    def go(self):
        self.logger.info("Starting CEP-II datamapper run")
        super(cep2_datamapper, self).go()

        # Search for the data-files
        data = findFiles(os.path.join(self.inputs['observation_dir'],
                                      '*.{dppp,MS,dp3}'),
                         '-1d')
        datamap = zip(data[0], data[1])

        self.logger.info("Found %i datasets to process." % len(datamap))
        self.logger.debug("datamap = %s" % datamap)

        # Write datamap-file
        create_directory(os.path.dirname(self.inputs['mapfile']))
        store_data_map(self.inputs['mapfile'], datamap)
        self.logger.debug("Wrote mapfile %s" % self.inputs['mapfile'])

        self.outputs['mapfile'] = self.inputs['mapfile']
        return 0

if __name__ == '__main__':
    sys.exit(cep2_datamapper().main())
