#                                                         LOFAR IMAGING PIPELINE
#
#                                                    get_metadata: master script
#                                                             Marcel Loose: 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map
from lofar.parameterset import parameterset

import sys


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
            help="Data product type",
#            optional=True,
#            default=None
        ),
        'parset_file': ingredient.StringField(
            '--parset-file',
            help="Path to the output parset file"
        )
    }

    # List of valid data product types.
    valid_product_types = ["Correlated", "InstrumentModel", "SkyImage"]


    def go(self):
        super(get_metadata, self).go()

        args = self.inputs['args']
        product_type = self.inputs['product_type']
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
                    arguments=[
                        infile,
                        self.inputs['product_type']
                    ]
                )
            )
        self._schedule_jobs(jobs)

        # Create the parset-file and write it to disk.        
        parset = parameterset()
        prefix = "Output_%s_" % product_type
        parset.replace('nrOf%s' % prefix, str(len(jobs)))
        for idx, job in enumerate(jobs):
            print "job[%d].results = %s" % (idx, job.results)
            parset.replace('%s[%d]' % (prefix, idx), str(job.results))
        parset.writeFile(self.inputs['parset_file'])
        
        if self.error.isSet():
            self.logger.warn("Failed get_metadata process detected")
            return 1


if __name__ == '__main__':
    sys.exit(get_metadata().main())
