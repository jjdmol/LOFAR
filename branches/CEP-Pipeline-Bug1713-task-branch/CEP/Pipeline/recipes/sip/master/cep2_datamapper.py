#                                                         LOFAR IMAGING PIPELINE
#
#                        Generate a mapfile for processing data on storage nodes
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os.path
import sys

from lofar.mstools import findFiles
from lofar.parameterset import parameterset

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
            help="Full path to the directory to search for MS files "
                 "(deprecated)",
            default=""
        ),
        'parset': ingredient.StringField(
            '--parset',
            help="Full path to the parset-file provided by MAC/SAS",
            default=""
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="Full path (including filename) of generated mapfile"
        )
    }


    def _read_files(self):
        """Read data file locations from parset-file"""
        self.logger.debug("Reading data file locations from parset-file: %s" %
                          self.inputs['parset'])
        parset = parameterset(self.inputs['parset'])
        filenames = parset.getStringVector(
            'ObsSW.Observation.DataProducts.Input_Correlated.filenames')
        locations = parset.getStringVector(
            'ObsSW.Observation.DataProducts.Input_Correlated.locations')
        return [''.join(x).split(':') for x in zip(locations, filenames)]


    def _search_files(self):
        """Search for the data-files"""
        ms_pattern = os.path.join(self.inputs['observation_dir'],
                                  '*.{dppp,MS,dp3}')
        self.logger.debug("Searching for data files: %s" % ms_pattern)
        data = findFiles(ms_pattern, '-1d')
        return zip(data[0], data[1])


    def go(self):
        self.logger.info("Starting CEP-II datamapper run")
        super(cep2_datamapper, self).go()

        if self.inputs['parset']:
            datamap = self._read_files()
        elif self.inputs['observation_dir']:
            datamap = self._search_files()
        else:
            self.logger.error("Either observation_dir or parset must be given")
            return 1

        self.logger.info("Found %i datasets to process." % len(datamap))
        self.logger.debug("datamap = %s" % datamap)

        # Write datamap-file
        create_directory(os.path.dirname(self.inputs['mapfile']))
        store_data_map(self.inputs['mapfile'], datamap)
        self.logger.debug("Wrote mapfile: %s" % self.inputs['mapfile'])

        self.outputs['mapfile'] = self.inputs['mapfile']
        return 0


if __name__ == '__main__':
    sys.exit(cep2_datamapper().main())
