#                                                          LOFAR PIPELINE SCRIPT
#
#                                           running an executable with arguments
#                                                         Stefan Froehlich, 2014
#                                                      s.froehlich@fz-juelich.de
# ------------------------------------------------------------------------------

import copy
import sys
import os

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, validate_data_maps, align_data_maps
from lofarpipe.support.parset import Parset


class executable_args(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Basic script for running an executable with arguments.
    Passing a mapfile along so the executable can process MS.
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="The full path to the relevant executable"
        ),
        'arguments': ingredient.ListField(
            '-a', '--arguments',
            help="List of arguments for the executable. Will be added as ./exe arg0 arg1...",
            default='',
            optional=True
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="Path to the arguments for this executable. Will be converted to --key=value",
            default='',
            optional=True
        ),
        'inputkey': ingredient.StringField(
            '-i', '--inputkey',
            help="Parset key that the executable will recognize as key for inputfile",
            default='',
            optional=True
        ),
        'outputkey': ingredient.StringField(
            '-0', '--outputkey',
            help="Parset key that the executable will recognize as key for outputfile",
            default='',
            optional=True
        ),
        'mapfile_in': ingredient.StringField(
            '--mapfile-in',
            help="Name of the input mapfile containing the names of the "
                 "MS-files to run the recipe",
            default=''#,
            #optional=True
        ),
        'mapfile_out': ingredient.StringField(
            '--mapfile-out',
            help="Name of the output mapfile containing the names of the "
                 "MS-files produced by the recipe",
            default='',
            optional=True
        ),
        'skip_infile': ingredient.BoolField(
            '--skip-infile',
            help="Dont give the input file to the executable.",
            default=False,
            optional=True
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="The full path to a mapfile describing the processed data"
        )
    }

    def go(self):
        self.logger.info("Starting %s run" % self.inputs['executable'])
        super(executable_args, self).go()

        # *********************************************************************
        # try loading input/output data file, validate output vs the input location if
        #    output locations are provided
        try:
            indata = DataMap.load(self.inputs['mapfile_in'])
        except Exception:
            self.logger.error('Could not load input Mapfile %s' % self.inputs['mapfile_in'])
            return 1
        if self.inputs['mapfile_out'] is not '':
            try:
                outdata = DataMap.load(self.inputs['mapfile_out'])
            except Exception:
                self.logger.error('Could not load output Mapfile %s' % self.inputs['mapfile_out'])
                return 1
            # sync skip fields in the mapfiles
            align_data_maps(indata, outdata)
        else:
            # ouput will be directed in the working directory if no output mapfile is specified
            outdata = copy.deepcopy(indata)
            for item in outdata:
                item.file = os.path.join(
                    self.inputs['working_directory'],
                    self.inputs['job_name'],
                    os.path.basename(item.file) + '.' + os.path.split(str(self.inputs['executable']))[1]
                )
            self.inputs['mapfile_out'] = os.path.join(os.path.dirname(self.inputs['mapfile_in']), os.path.basename(self.inputs['executable']) + '.' + 'mapfile')

        if not validate_data_maps(indata, outdata):
            self.logger.error(
                "Validation of data mapfiles failed!"
            )
            return 1

        # prepare arguments
        arglist = self.inputs['arguments']
        parset = Parset()
        parset.adoptFile(self.inputs['parset'])
        for k in parset.keys:
            arglist.append('--' + k + '=' + parset.getString(k))

        # ********************************************************************
        # Call the node side of the recipe
        # Create and schedule the compute jobs
        command = "python %s" % (self.__file__.replace('master', 'nodes'))
        indata.iterator = outdata.iterator = DataMap.SkipIterator
        jobs = []
        for inp, outp in zip(
            indata, outdata
        ):
            if self.inputs['inputkey'] and not self.inputs['skip_infile']:
                arglist.append('--' + self.inputs['inputkey'] + '=' + inp.file)
            if self.inputs['outputkey'] and not self.inputs['skip_infile']:
                arglist.append('--' + self.inputs['outputkey'] + '=' + outp.file)
            if not self.inputs['inputkey'] and not self.inputs['skip_infile']:
                arglist.insert(0, inp.file)
            jobs.append(
                ComputeJob(
                    inp.host, command,
                    arguments=[
                        inp.file,
                        outp.file,
                        self.inputs['executable'],
                        arglist,
                        self.inputs['working_directory'],
                        self.environment
                    ]
                )
            )
        self._schedule_jobs(jobs)
        for job, outp in zip(jobs, outdata):
            if job.results['returncode'] != 0:
                outp.skip = True

        # *********************************************************************
        # Check job results, and create output data map file
        if self.error.isSet():
            # Abort if all jobs failed
            if all(job.results['returncode'] != 0 for job in jobs):
                self.logger.error("All jobs failed. Bailing out!")
                return 1
            else:
                self.logger.warn(
                    "Some jobs failed, continuing with succeeded runs"
                )
        self.logger.debug("Writing data map file: %s" % self.inputs['mapfile_out'])
        outdata.save(self.inputs['mapfile_out'])
        self.outputs['mapfile'] = self.inputs['mapfile_out']
        return 0

if __name__ == '__main__':
    sys.exit(executable_args().main())
