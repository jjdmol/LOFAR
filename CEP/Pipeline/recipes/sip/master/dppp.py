#                                                         LOFAR IMAGING PIPELINE
#
#                                         New DPPP recipe: fixed node allocation
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

from collections import defaultdict

import sys
import os

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps

class dppp(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Runs DPPP (either ``NDPPP`` or -- in the unlikely event it's required --
    ``IDPPP``) on a number of MeasurementSets. This is used for compressing
    and/or flagging data

    **Arguments**

    A mapfile describing the data to be processed.
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
        ),
        'fullyflagged': ingredient.ListField(
            help="A list of all baselines which were completely flagged in any "
                 "of the input MeasurementSets"
        )
    }


    def go(self):
        self.logger.info("Starting DPPP run")
        super(dppp, self).go()

        #                Keep track of "Total flagged" messages in the DPPP logs
        # ----------------------------------------------------------------------
        self.logger.searchpatterns["fullyflagged"] = "Fully flagged baselines"

        #                            Load file <-> output node mapping from disk
        # ----------------------------------------------------------------------
        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        indata = load_data_map(args[0])
        if len(args) > 1:
            self.logger.debug("Loading output-data mapfile: %s" % args[1])
            outdata = load_data_map(args[1])
            if not validate_data_maps(indata, outdata):
                self.logger.error(
                    "Validation of input/output data mapfiles failed"
                )
                return 1
        else:
            outdata = [
                (host,
                 os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    os.path.basename(infile) + self.inputs['suffix'])
                ) for host, infile in indata
            ]

        # Load parmdb-mapfile, if one was given.         
        if self.inputs.has_key('parmdb_mapfile'):
            self.logger.debug(
                "Loading parmdb mapfile: %s" % self.inputs['parmdb_mapfile']
            )
            parmdbdata = load_data_map(self.inputs['parmdb_mapfile'])
        else:
            parmdbdata = [(None, None)] * len(indata)
            
        # Load sourcedb-mapfile, if one was given.         
        if self.inputs.has_key('sourcedb_mapfile'):
            self.logger.debug(
                "Loading sourcedb mapfile: %s" % self.inputs['sourcedb_mapfile']
            )
            sourcedbdata = load_data_map(self.inputs['sourcedb_mapfile'])
        else:
            sourcedbdata = [(None, None)] * len(indata)

        # Create and schedule the compute jobs
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        jobs = []
        for host, infile, outfile, parmdb, sourcedb in (w + (x[1], y[1], z[1]) 
            for w, x, y, z in zip(indata, outdata, parmdbdata, sourcedbdata)):
            jobs.append(
                ComputeJob(
                    host, command,
                    arguments=[
                        infile,
                        outfile,
                        parmdb,
                        sourcedb,
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

        #                                  Log number of fully flagged baselines
        # ----------------------------------------------------------------------
        matches = self.logger.searchpatterns["fullyflagged"].results
        self.logger.searchpatterns.clear() # finished searching
        stripchars = "".join(set("Fully flagged baselines: "))
        baselinecounter = defaultdict(lambda: 0)
        for match in matches:
            for pair in (
                pair.strip(stripchars) for pair in match.getMessage().split(";")
            ):
                baselinecounter[pair] += 1
        self.outputs['fullyflagged'] = baselinecounter.keys()

        if self.error.isSet():
            # dppp needs to continue on partial succes.
            # Get the status of the jobs
            node_status = {}
            ok_counter = 0
            for job in jobs:
                if job.results.has_key("ok"):
                    node_status[job.host] = True
                    ok_counter += 1
                else:
                    node_status[job.host] = False

            # if all nodes failed abort
            if ok_counter == 0:
                self.logger.error("None of the dppp runs finished with an ok status")
                self.logger.error("Exiting recipe with fail status")
                return 1

            # Create new mapfile with only the successful runs
            new_outdata = []
            for host, path in outdata:
                if node_status[host]:
                    new_outdata.append((host, path))
                # else do not put in the outdata list
            #swap the outputfiles
            outdata = new_outdata

            self.logger.warn("Failed DPPP process detected,"
                             "continue with succeeded runs")

        # Write output data return ok status
        self.logger.debug("Writing data map file: %s" %
                          self.inputs['mapfile'])
        store_data_map(self.inputs['mapfile'], outdata)
        self.outputs['mapfile'] = self.inputs['mapfile']
        return 0

if __name__ == '__main__':
    sys.exit(dppp().main())
