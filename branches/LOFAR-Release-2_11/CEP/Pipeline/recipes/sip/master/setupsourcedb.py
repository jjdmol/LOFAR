#                                                         LOFAR IMAGING PIPELINE
#
#                                                    setupsourcedb master recipe
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import copy
import os
import sys

import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.data_map import DataMap, validate_data_maps
from lofarpipe.support.remotecommand import ComputeJob

class setupsourcedb(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Create a distributed Sky Model database (SourceDB) for a distributed
    Measurement Set (MS).

    1. Load input and output mapfiles. Validate 
    2. Check if input skymodel file exists. If not, make filename empty.
    3. Call node side of recipe
    4. Validate performance and create output

    **Command line arguments**

    1. A mapfile describing the input data to be processed. 
    2. A mapfile with target location <if provided it will be validated against
       The input data>
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="Full path to makesourcedb executable",
        ),
        'skymodel': ingredient.FileField(
            '-s', '--skymodel',
            help="Input sky catalogue",
            optional=True
        ),
        'type': ingredient.StringField(
            '--type',
            help="Output type (casa or blob)",
            default="casa"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile to produce; it will contain "
                 "a list of the generated sky-model files"
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            help="Maximum number of simultaneous processes per compute node",
            default=8
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            help="Suffix of the table name of the sky model",
            default=".sky"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. "
                 "Results will be written here."
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="mapfile with created sourcedb paths"
        )
    }


    def go(self):
        self.logger.info("Starting setupsourcedb run")
        super(setupsourcedb, self).go()

        # *********************************************************************
        # 1. Load input and output mapfiles. Validate

        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        indata = DataMap.load(args[0])
        if len(args) > 1:
            self.logger.debug("Loading output-data mapfile: %s" % args[1])
            outdata = DataMap.load(args[1])
            if not validate_data_maps(indata, outdata):
                self.logger.error(
                    "Validation of input/output data mapfiles failed"
                )
                return 1
        else:
            outdata = copy.deepcopy(indata)
            for item in outdata:
                item.file = os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    os.path.basename(item.file) + self.inputs['suffix']
                )

        # *********************************************************************
        # 2. Check if input skymodel file exists. If not, make filename empty.
        try:
            skymodel = self.inputs['skymodel']
        except KeyError:
            skymodel = ""
            self.logger.info("No skymodel specified. Using an empty one")

        # ********************************************************************
        # 3. Call node side of script
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        outdata.iterator = DataMap.SkipIterator
        jobs = []
        for outp in outdata:
            jobs.append(
                ComputeJob(
                    outp.host,
                    command,
                    arguments=[
                        self.inputs['executable'],
                        skymodel,
                        outp.file,
                        self.inputs['type']
                    ]
                )
            )
        self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])
        for job, outp in zip(jobs, outdata):
            if job.results['returncode'] != 0:
                outp.skip = True

        # *********************************************************************
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
        self.logger.debug("Writing sky map file: %s" % self.inputs['mapfile'])
        outdata.save(self.inputs['mapfile'])
        self.outputs['mapfile'] = self.inputs['mapfile']
        return 0


if __name__ == '__main__':
    sys.exit(setupsourcedb().main())
