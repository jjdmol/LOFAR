#                                                                 LOFAR PIPELINE
#
#                                                         Stefan Froehlich, 2015
#                                                      s.froehlich@fz-juelich.de
# ------------------------------------------------------------------------------

import copy
import sys
import os

import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, validate_data_maps
from lofarpipe.support.parset import Parset

class executable_parsetonly(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    wrapping an executable with only a parset as argument
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="The full path to the  awimager executable"
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="The full path to a configuration parset. The ``msin`` "
                 "and ``msout`` keys will be added by this recipe"
        ),
        'inputkey': ingredient.StringField(
            '-I', '--inputkey',
            help="Parset key that the executable will recognize as key for inputfile",
            default=''
        ),
        'outputkey': ingredient.StringField(
            '-0', '--outputkey',
            help="Parset key that the executable will recognize as key for outputfile",
            default=''
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Name of the output mapfile containing the names of the "
                 "MS-files produced by the recipe",
            optional=True
        ),
        'outputsuffixes': ingredient.ListField(
            '--outputsuffixes',
            help="Suffixes for the outputfiles",
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="The full path to a mapfile describing the processed data"
        )
    }

    #print inputs['outputsuffixes']
    #for k in inputs['outputsuffixes']:
    #    outputs[k] = ''

    def go(self):
        self.logger.info("Starting %s run" % self.inputs['executable'])
        super(executable_parsetonly, self).go()

        # *********************************************************************
        # 1. load input data file, validate output vs the input location if
        #    output locations are provided
        args = self.inputs['args']
        self.logger.debug("Loading input-data mapfile: %s" % args[0])
        indata = DataMap.load(args[0])

        outputsuffix = self.inputs['outputsuffixes']
        outputmapfiles = []
        prefix = os.path.join(self.inputs['working_directory'], self.inputs['job_name'])
        for name in outputsuffix:
            outputmapfiles.append(copy.deepcopy(indata))
            for item in outputmapfiles[-1]:
                item.file = os.path.join(
                    prefix,
                     os.path.basename(item.file) + '.' + os.path.split(str(self.inputs['executable']))[1] + '.' + name
                )

        outdata = copy.deepcopy(indata)
        for item in outdata:
            item.file = os.path.join(
                self.inputs['working_directory'],
                self.inputs['job_name'],
                os.path.basename(item.file) + '.' + os.path.split(str(self.inputs['executable']))[1]
                #os.path.basename(item.file) + '.'
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
        # the following parset things are not nice...
        nodeparsetraw = Parset()
        nodeparsetraw.adoptFile(self.inputs['parset'])
        nodeparsetraw.add(self.inputs['inputkey'], 'placeholder')
        nodeparsetraw.add(self.inputs['outputkey'], 'placeholder')
        nodeparsetdict = {}
        for k in nodeparsetraw.keys:
            nodeparsetdict[k] = str(nodeparsetraw[k])

        noderecipe = (self.__file__.replace('master', 'nodes')).replace('awimager.py', 'executable_parsetonly.py')
        #noderecipe = (self.__file__.replace('master', 'nodes'))
        command = "python %s" % noderecipe
        indata.iterator = outdata.iterator = DataMap.SkipIterator
        jobs = []
        for inp, outp in zip(
            indata, outdata
        ):
            nodeparsetdict[self.inputs['inputkey']] = inp.file
            nodeparsetdict[self.inputs['outputkey']] = outp.file
            nodeparsetrawstring = outp.file + '.' + 'parset'
            jobs.append(
                ComputeJob(
                    inp.host, command,
                    arguments=[
                        inp.file,
                        self.inputs['executable'],
                        nodeparsetdict,
                        nodeparsetrawstring,
                        #self.inputs['working_directory'],
                        prefix,
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
        #self.logger.debug("Writing data map file: %s" % self.inputs['mapfile'])
        #outdata.save(self.inputs['mapfile'])
        #self.outputs['mapfile'] = self.inputs['mapfile']
        mapdict = {}
        for item, name in zip(outputmapfiles, outputsuffix):
            item.save(name + '.' + 'mapfile')
            mapdict[name] = name + '.' + 'mapfile'
            #self.outputs[name] = name + '.' + 'mapfile'
        if not outputsuffix:
            outdata.save(self.inputs['mapfile'])
            self.outputs['mapfile'] = self.inputs['mapfile']
        else:
            print outputsuffix[0]
            self.outputs.update(mapdict)
            self.outputs['mapfile'] = outputsuffix[0] + '.' + 'mapfile'
            #self.outputs['psfmap'] = outputsuffix[-1] + '.' + 'mapfile'
        return 0

if __name__ == '__main__':
    sys.exit(executable_parsetonly().main())
