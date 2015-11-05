#                                                         LOFAR IMAGING PIPELINE
#
#                                                    get_metadata: master script
#                                                             Marcel Loose: 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------
import sys
import os

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map
from lofarpipe.recipes.helpers import metadata
from lofar.parameterset import parameterset
from lofarpipe.support.utilities import create_directory
class get_metadata(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Get the metadata from the given data products and return them as a LOFAR
    parameterset.
    
    **Arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'product_type': ingredient.StringField(
            '--product-type',
            help = "Data product type",
#            optional=True,
#            default=None
        ),
        'parset_file': ingredient.StringField(
            '--parset-file',
            help = "Path to the output parset file"
        ),
        'parset_prefix': ingredient.StringField(
            '--parset-prefix',
            help = "Prefix for each key in the output parset file",
            default = ''
        )
    }

    # List of valid data product types.
    valid_product_types = ["Correlated", "InstrumentModel", "SkyImage"]


    def go(self):
        super(get_metadata, self).go()

        args = self.inputs['args']
        product_type = self.inputs['product_type']
        global_prefix = self.inputs['parset_prefix']
        # Add a trailing dot (.) if not present in the prefix.
        if global_prefix and not global_prefix.endswith('.'):
            global_prefix += '.'

        if not product_type in self.valid_product_types:
            self.logger.error(
                "Unknown product type: %s\n\tValid product types are: %s" %
                (product_type, ', '.join(self.valid_product_types))
        )

        #                           Load file <-> compute node mapping from disk
        # ----------------------------------------------------------------------
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        data = load_data_map(args[0])

        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        jobs = []
        for host, infile in data:
            jobs.append(
                ComputeJob(
                    host, command,
                    arguments = [
                        infile,
                        self.inputs['product_type']
                    ]
                )
            )
        self._schedule_jobs(jobs)

        if self.error.isSet():
            self.logger.warn("Failed get_metadata process detected")
            return 1

        # Create the parset-file and write it to disk.        
        parset = parameterset()
        prefix = "Output_%s_" % product_type
        parset.replace('%snrOf%s' % (global_prefix, prefix), str(len(jobs)))
        prefix = global_prefix + prefix
        for idx, job in enumerate(jobs):
            self.logger.debug("job[%d].results = %s" % (idx, job.results))
            parset.adoptCollection(
                metadata.to_parset(job.results), '%s[%d].' % (prefix, idx)
            )
        dir_path = os.path.dirname(self.inputs['parset_file'])
        #assure existence of containing directory
        create_directory(dir_path)

        parset.writeFile(self.inputs['parset_file'])


if __name__ == '__main__':
    sys.exit(get_metadata().main())
