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
from lofarpipe.support.data_map import DataMap
from lofarpipe.recipes.helpers import metadata
from lofar.parameterset import parameterset
from lofarpipe.support.utilities import create_directory
class get_metadata(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Get the metadata from the given data products and return them as a LOFAR
    parameterset.
    
    1. Parse and validate inputs
    2. Load mapfiles
    3. call node side of the recipe
    4. validate performance
    5. Create the parset and return it.
    
    **Command line arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'product_type': ingredient.StringField(
            '--product-type',
            help="Data product type",
        ),
        'parset_prefix': ingredient.StringField(
            '--parset-prefix',
            help="Prefix for each key in the output parset file",
            default=''
        ),
        'metadata_file': ingredient.StringField(
            '--metadata-file',
            help="filename of parset to put obtained metadata in"
        )
    }

    outputs = {
    }
    
    # List of valid data product types.
    valid_product_types = ["Correlated", "InstrumentModel", "SkyImage"]


    def go(self):
        super(get_metadata, self).go()
        # ********************************************************************
        # 1. Parse and validate inputs
        args = self.inputs['args']
        product_type = self.inputs['product_type']
        global_prefix = self.inputs['parset_prefix']
        # Add a trailing dot (.) if not present in the prefix.
        if global_prefix and not global_prefix.endswith('.'):
            global_prefix += '.'

        if not product_type in self.valid_product_types:
            self.logger.warn(
                "Unknown product type: %s\n\tValid product types are: %s" %
                (product_type, ', '.join(self.valid_product_types))
        )

        # ********************************************************************
        # 2. Load mapfiles
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        data = DataMap.load(args[0])

        # ********************************************************************
        # 3. call node side of the recipe
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        data.iterator = DataMap.SkipIterator
        jobs = []
        for inp in data:
            jobs.append(
                ComputeJob(
                    inp.host, command,
                    arguments=[
                        inp.file,
                        self.inputs['product_type']
                    ]
                )
            )
        self._schedule_jobs(jobs)
        for job, inp in zip(jobs, data):
            if job.results['returncode'] != 0:
                inp.skip = True

        # ********************************************************************
        # 4. validate performance
        # 4. Check job results, and create output data map file
        if self.error.isSet():
            # Abort if all jobs failed
            if all(job.results['returncode'] != 0 for job in jobs):
                self.logger.error("All jobs failed. Bailing out!")
                return 1
            else:
                self.logger.warn(
                    "Some jobs failed, continuing with succeeded runs"
                )
        self.logger.debug("Updating data map file: %s" % args[0])
        data.save(args[0])

        # ********************************************************************
        # 5. Create the parset-file and return it to the caller
        parset = parameterset()
        prefix = "Output_%s_" % product_type  #Underscore is needed because
                             # Mom / LTA cannot differentiate input and output
        parset.replace('%snrOf%s' % (global_prefix, prefix), str(len(jobs)))

        prefix = global_prefix + prefix
        for idx, job in enumerate(jobs):
            self.logger.debug("job[%d].results = %s" % (idx, job.results))

            # the Master/node communication adds a monitor_stats entry,
            # this must be remove manually here 
            meta_data_parset = metadata.to_parset(job.results)
            try:
                meta_data_parset.remove("monitor_stats")
            except:
                pass

            parset.adoptCollection(meta_data_parset,
                                   '%s[%d].' % (prefix, idx))

        # Return result to caller
        parset.writeFile(self.inputs["metadata_file"])
        return 0


if __name__ == '__main__':
    sys.exit(get_metadata().main())
