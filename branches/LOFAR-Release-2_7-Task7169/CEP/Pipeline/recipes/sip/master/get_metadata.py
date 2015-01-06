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
    5. Create the parset-file and write it to disk.  
    
    **Command line arguments**

    A mapfile describing the data to be processed.
    """
    inputs = {
        'product_type': ingredient.StringField(
            '--product-type',
            help="Data product type",
        ),
        'parset_file': ingredient.StringField(
            '--parset-file',
            help="Path to the output parset file"
        ),
        'parset_prefix': ingredient.StringField(
            '--parset-prefix',
            help="Prefix for each key in the output parset file",
            default=''
        ),
        'toplevel_meta_data_path': ingredient.StringField(
            '--toplevel-meta-data',
            help="Path to parset with toplevel meta information, default = ''",
            default=''
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
            self.logger.error(
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

        # ********************************************************************
        # 5. Create the parset-file and write it to disk.        
        parset = parameterset()
        prefix = "Output_%s_" % product_type
        parset.replace('%snrOf%s' % (global_prefix, prefix), str(len(jobs)))

        # If there is meta data to add from the toplevel script
        pipeline_meta_parset_path = self.inputs['toplevel_meta_data_path']
        if pipeline_meta_parset_path != "":
            pipeline_meta_parset = parameterset(pipeline_meta_parset_path)
            parset.adoptCollection(pipeline_meta_parset)

        prefix = global_prefix + prefix
        for idx, job in enumerate(jobs):
            self.logger.debug("job[%d].results = %s" % (idx, job.results))

            # the Master/node communication adds a monitor_stats entry,
            # this must be remove manually here 
            meta_data_parset = metadata.to_parset(job.results)
            meta_data_parset.remove("monitor_stats")

            parset.adoptCollection(meta_data_parset,
                                   '%s[%d].' % (prefix, idx))

        try:

            create_directory(os.path.dirname(self.inputs['parset_file']))
            parset.writeFile(self.inputs['parset_file'])
            self.logger.info("Wrote meta data to: " + 
                             self.inputs['parset_file'])
        except RuntimeError, err:
            self.logger.error("Failed to write meta-data: %s" % str(err))
            return 1

        return 0


if __name__ == '__main__':
    sys.exit(get_metadata().main())
