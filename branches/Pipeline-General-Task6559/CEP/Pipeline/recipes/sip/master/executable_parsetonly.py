#                                                         LOFAR IMAGING PIPELINE
#
#                                         New DPPP recipe: fixed node allocation
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import copy
import sys
import os

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, validate_data_maps


class executable_parsetonly(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Runs an executable on a number of MeasurementSets with parset as only argument.
    TODO:mapfiles go where?

    1. Load input data files
    3. Call the node side of the recipe
    4. Create mapfile with successful noderecipe runs

    **Command line arguments**

    1. A mapfile describing the data to be processed.
    2. Optionally, a mapfile with target output locations.

    """
    inputs = {
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="The full path to a configuration parset. The ``msin`` "
                 "and ``msout`` keys will be added by this recipe"
        ),
        'executable': ingredient.ExecField(
            '--executable',
            help="The full path to the relevant executable"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Name of the output mapfile containing the names of the "
                 "MS-files produced by the recipe"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="The full path to a mapfile describing the processed data"
        )
    }

    def go(self):
        self.logger.info("Starting %s run" % self.inputs['executable'])
        super(executable_parsetonly, self).go()

        # *********************************************************************
        # 1. load input data file, validate output vs the input location if
        #    output locations are provided
        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        indata = DataMap.load(args[0])
        if len(args) > 1:
            self.logger.debug("Loading output-data mapfile: %s" % args[1])
            outdata = DataMap.load(args[1])
            # Update the skip fields of the two maps. If 'skip' is True in any of
            # these maps, then 'skip' must be set to True in all maps.
            # TODO:this is functionality concerning datamaps and should be put as generic method in the datamaps class
            for w, x in zip(indata, outdata):
                w.skip = x.skip = (
                    w.skip or x.skip
                )
        else:
            outdata = copy.deepcopy(indata)
            for item in outdata:
                item.file = os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    os.path.basename(item.file) + '.' + os.path.split(str(self.inputs['executable']))[1]
                )

        # ********************************************************************
        # 2.
        # Validate all the data maps.
        if not validate_data_maps(indata, outdata):
            self.logger.error(
                "Validation of data mapfiles failed!"
            )
            return 1

        # ********************************************************************
        # 3. Call the node side of the recipe
        # Create and schedule the compute jobs
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        indata.iterator = outdata.iterator = DataMap.SkipIterator
        jobs = []
        for inp, outp in zip(
            indata, outdata
        ):
            jobs.append(
                ComputeJob(
                    inp.host, command,
                    arguments=[
                        inp.file,
                        outp.file,
                        self.inputs['parset'],
                        self.inputs['executable'],
                        self.environment
                    ]
                )
            )
        self._schedule_jobs(jobs)
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
        self.logger.debug("Writing data map file: %s" % self.inputs['mapfile'])
        outdata.save(self.inputs['mapfile'])
        self.outputs['mapfile'] = self.inputs['mapfile']
        return 0

if __name__ == '__main__':
    sys.exit(executable_parsetonly().main())
