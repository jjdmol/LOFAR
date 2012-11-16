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

class dppp(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Runs ``NDPPP`` on a number of MeasurementSets. This is used for averaging,
    and/or flagging, and/or demixing of data.

    1. Load input data files
    2. Load parmdb and sourcedb
    3. Call the node side of the recipe
    4. Create mapfile with successful noderecipe runs

    **Command line arguments**

    1. A mapfile describing the data to be processed.
    2. Optionally, a mapfile with target output locations.

    """
    inputs = {
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="The full path to a DPPP configuration parset. The ``msin`` "
                 "and ``msout`` keys will be added by this recipe"
        ),
        'executable': ingredient.ExecField(
            '--executable',
            help="The full path to the relevant DPPP executable"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            default=".dppp",
            help="Added to the input filename to generate the output filename"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. Results will be "
                 "written here"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Name of the output mapfile containing the names of the "
                 "MS-files produced by the DPPP recipe"
        ),
        'parmdb_mapfile': ingredient.StringField(
            '--parmdb-mapfile',
            optional=True,
            help="Path to mapfile containing the parmdb files "
                 "(used by demixing step only)"
        ),
        'sourcedb_mapfile': ingredient.StringField(
            '--sourcedb-mapfile',
            optional=True,
            help="Path to mapfile containing the sourcedb files "
                 "(used by demixing step only)"
        ),
        'demix_always': ingredient.ListField(
            '--demix-always',
            help="List of sources that must always be demixed "
                 "(used by demixing step only)",
            default=[]
        ),
        'demix_if_needed': ingredient.ListField(
            '--demix-if-needed',
            help="List of sources that will only be demixed if needed, "
                 "based on some heuristics (used by demixing step only)",
            default=[]
        ),
        # NB times are read from vds file as string
        'data_start_time': ingredient.StringField(
            '--data-start-time',
            default="",
            help="Start time to be passed to DPPP; used to pad data"
        ),
        'data_end_time': ingredient.StringField(
            '--data-end-time',
            default="",
            help="End time to be passed to DPPP; used to pad data"
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            default=8,
            help="Maximum number of simultaneous processes per output node"
        ),
        'nthreads': ingredient.IntField(
            '--nthreads',
            default=2,
            help="Number of threads per (N)DPPP process"
        ),
        'clobber': ingredient.BoolField(
            '--clobber',
            default=False,
            help="If ``True``, pre-existing output files will be removed "
                 "before processing starts. If ``False``, the pipeline will "
                 "abort if files already exist with the appropriate output "
                 "filenames"
        )
# Keys that are present in the original demixing recipe. 
# Don't know yet if we still need them.
#        'timestep': ingredient.IntField(
#            '--timestep',
#            help="Time step for averaging",
#            default=10
#        ),
#        'freqstep': ingredient.IntField(
#            '--freqstep',
#            help="Frequency step for averaging",
#            default=60
#        ),
#        'half_window': ingredient.IntField(
#            '--half-window',
#            help="Window size of median filter",
#            default=20
#        ),
#        'threshold': ingredient.FloatField(
#            '--threshold',
#            help="Solutions above/below threshold*rms are smoothed",
#            default=2.5
#        ),
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="The full path to a mapfile describing the processed data"
#        ),
#        'fullyflagged': ingredient.ListField(
#            help="A list of all baselines which were completely flagged in any "
#                 "of the input MeasurementSets"
        )
    }


    def go(self):
        self.logger.info("Starting DPPP run")
        super(dppp, self).go()

#        #                Keep track of "Total flagged" messages in the DPPP logs
#        # ----------------------------------------------------------------------
#        self.logger.searchpatterns["fullyflagged"] = "Fully flagged baselines"

        # *********************************************************************
        # 1. load input data file, validate output vs the input location if
        #    output locations are provided
        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        indata = DataMap.load(args[0])
        if len(args) > 1:
            self.logger.debug("Loading output-data mapfile: %s" % args[1])
            outdata = DataMap.load(args[1])
        else:
            outdata = copy.deepcopy(indata)
            for item in outdata:
                item.file = os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    os.path.basename(item.file) + self.inputs['suffix']
                )

        # ********************************************************************
        # 2. Load parmdb and sourcedb
        # Load parmdb-mapfile, if one was given.         
        if self.inputs.has_key('parmdb_mapfile'):
            self.logger.debug(
                "Loading parmdb mapfile: %s" % self.inputs['parmdb_mapfile']
            )
            parmdbdata = DataMap.load(self.inputs['parmdb_mapfile'])
        else:
            parmdbdata = copy.deepcopy(indata)
            for item in parmdbdata:
                item.file = ''

        # Load sourcedb-mapfile, if one was given.         
        if self.inputs.has_key('sourcedb_mapfile'):
            self.logger.debug(
                "Loading sourcedb mapfile: %s" % self.inputs['sourcedb_mapfile']
            )
            sourcedbdata = DataMap.load(self.inputs['sourcedb_mapfile'])
        else:
            sourcedbdata = copy.deepcopy(indata)
            for item in sourcedbdata:
                item.file = ''

        # Validate all the data maps.
        if not validate_data_maps(indata, outdata, parmdbdata, sourcedbdata):
            self.logger.error(
                "Validation of data mapfiles failed!"
            )
            return 1

        # Update the skip fields of the four maps. If 'skip' is True in any of
        # these maps, then 'skip' must be set to True in all maps.
        for w, x, y, z in zip(indata, outdata, parmdbdata, sourcedbdata):
            w.skip = x.skip = y.skip = z.skip = (
                w.skip or x.skip or y.skip or z.skip
            )

        # ********************************************************************
        # 3. Call the node side of the recipe
        # Create and schedule the compute jobs
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        indata.iterator = outdata.iterator = DataMap.SkipIterator
        parmdbdata.iterator = sourcedbdata.iterator = DataMap.SkipIterator
        jobs = []
        for inp, outp, pdb, sdb in zip(
            indata, outdata, parmdbdata, sourcedbdata
        ):
            jobs.append(
                ComputeJob(
                    inp.host, command,
                    arguments=[
                        inp.file,
                        outp.file,
                        pdb.file,
                        sdb.file,
                        self.inputs['parset'],
                        self.inputs['executable'],
                        self.environment,
                        self.inputs['demix_always'],
                        self.inputs['demix_if_needed'],
                        self.inputs['data_start_time'],
                        self.inputs['data_end_time'],
                        self.inputs['nthreads'],
                        self.inputs['clobber']
                    ]
                )
            )
        self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])
        for job, outp in zip(jobs, outdata):
            if job.results['returncode'] != 0:
                outp.skip = True

#        # *********************************************************************
#        # 4. parse logfile for fully flagged baselines
#        matches = self.logger.searchpatterns["fullyflagged"].results
#        self.logger.searchpatterns.clear() # finished searching
#        stripchars = "".join(set("Fully flagged baselines: "))
#        baselinecounter = defaultdict(lambda: 0)
#        for match in matches:
#            for pair in (
#                pair.strip(stripchars) for pair in match.getMessage().split(";")
#            ):
#                baselinecounter[pair] += 1
#        self.outputs['fullyflagged'] = baselinecounter.keys()

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
    sys.exit(dppp().main())
