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
        'executable': ingredient.ExecField(
            '--executable',
            help = "The full path to the relevant DPPP executable"
        ),
        'initscript': ingredient.FileField(
            '--initscript',
            help = "The full path to an (Bourne) shell script which will "
                 "intialise the environment (ie, ``lofarinit.sh``)"
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help = "The full path to a DPPP configuration parset. The ``msin`` "
                 "and ``msout`` keys will be added by this recipe"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            default = ".dppp",
            help = "Added to the input filename to generate the output filename"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help = "Working directory used on output nodes. Results will be "
                 "written here"
        ),
        # NB times are read from vds file as string
        'data_start_time': ingredient.StringField(
            '--data-start-time',
            default = "None",
            help = "Start time to be passed to DPPP; used to pad data"
        ),
        'data_end_time': ingredient.StringField(
            '--data-end-time',
            default = "None",
            help = "End time to be passed to DPPP; used to pad data"
        ),
        'nproc': ingredient.IntField(
            '--nproc',
            default = 8,
            help = "Maximum number of simultaneous processes per output node"
        ),
        'nthreads': ingredient.IntField(
            '--nthreads',
            default = 2,
            help = "Number of threads per (N)DPPP process"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help = "Filename into which a mapfile describing the output data "
                 "will be written"
        ),
        'clobber': ingredient.BoolField(
            '--clobber',
            default = False,
            help = "If ``True``, pre-existing output files will be removed "
                 "before processing starts. If ``False``, the pipeline will "
                 "abort if files already exist with the appropriate output "
                 "filenames"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help = "The full path to a mapfile describing the processed data"
        ),
        'fullyflagged': ingredient.ListField(
            help = "A list of all baselines which were completely flagged in any "
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

        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        jobs = []
        for host, infile, outfile in (x + (y[1],)
            for x, y in zip(indata, outdata)):
            jobs.append(
                ComputeJob(
                    host, command,
                    arguments = [
                        infile,
                        outfile,
                        self.inputs['parset'],
                        self.inputs['executable'],
                        self.inputs['initscript'],
                        self.inputs['data_start_time'],
                        self.inputs['data_end_time'],
                        self.inputs['nthreads'],
                        self.inputs['clobber']
                    ]
                )
            )
        self._schedule_jobs(jobs, max_per_node = self.inputs['nproc'])

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
