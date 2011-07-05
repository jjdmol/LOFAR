#                                                         LOFAR IMAGING PIPELINE
#
#                        Generate a mapfile for processing data on storage nodes
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os.path
import sys
import subprocess
from collections import defaultdict

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.parset import Parset
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

        datamap = {}
        for node in ["locus%03i" % n for n in range(1,101)]:
            self.logger.debug("Searching on node %s ..." % node)
            pattern = ' '.join([os.path.join(self.inputs['observation_dir'],f)
                                for f in ['*.dppp', '*.MS', '*.dp3']])
            command = ["ssh", "-xT", "-o StrictHostKeyChecking=no",
                       "%s" % node, "ls -1d %s" % pattern]
            find = subprocess.Popen(command,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
            (output, error) = find.communicate()
            if find.returncode == 0:
                datamap[node] = output.split()
            elif find.returncode > 127:
                # Log errors not related to 'ls'
                self.logger.warn("%s" % error.strip())

        self.logger.info("Found %i datasets to process." %
                      sum(len(datamap[k]) for k in datamap))
        self.logger.debug("datamap = %s" % datamap)

        # Write datamap-file
        create_directory(os.path.dirname(self.inputs['mapfile']))
        file = open(self.inputs['mapfile'], 'w')
        for key in sorted(datamap):
            file.write('%s = %s\n' % (key, datamap[key]))
        file.close()
        self.logger.debug("Wrote mapfile %s" % self.inputs['mapfile'])

        self.outputs['mapfile'] = self.inputs['mapfile']
        return 0

if __name__ == '__main__':
    sys.exit(cep2_datamapper().main())
