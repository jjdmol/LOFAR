#                                                         LOFAR IMAGING PIPELINE
#
#                                    Example recipe with simple job distribution
#                                                          Wouter Klijn, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------
# python imager_awimager.py ~/build/preparation/output.map --job imager_awimager --config ~/build/preparation/pipeline.cfg --initscript /opt/cep/LofIm/daily/lofar/lofarinit.sh --parset ~/build/preparation/parset.par --working-directory "/data/scratch/klijn" --executable /opt/cep/LofIm/daily/lofar/bin/awimager -d
# the measurement set with input should be located in the working directory

import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map

class imager_awimager(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Run the imager_awimager on the nodes and the data files suplied in the mapfile
    **Arguments**
    A mapfile containing node->datafile pairs  
 
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help = "The full path to the relevant awimager executable"
        ),
        'initscript': ingredient.FileField(
            '--initscript',
            help = '''The full path to an (Bourne) shell script which will\
             intialise the environment (ie, ``lofarinit.sh``)'''
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help = "The full path to a awimager configuration parset."
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help = "Working directory used on outpuconfigt nodes. Results location"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            default = ".awimager",
            help = "Added to the input filename to generate the output filename"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help = "Full path of mapfile; contains a list of the"
                 "successfully generated images"
        ),
    }

    outputs = {
        'mapfile': ingredient.StringField()
    }

    def go(self):
        super(imager_awimager, self).go()
        self.logger.info("Starting imager_awimager run")

        #collect the inputs        
        # TODO:  input_map to be merged with change marcel
        input_map = eval(open(self.inputs['args'][0]).read())
        executable = self.inputs['executable']
        init_script = self.inputs['initscript']
        parset = self.inputs['parset']
        working_dir = self.inputs['working_directory']
#        job_name = self.inputs['job_name']
        suffix = self.inputs['suffix']

        # Compile the command to be executed on the remote machine
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))
        # Create the jobs
        jobs = []
        outnames = collections.defaultdict(list)
        for host, measurement_set in input_map:
            #construct and save the output name
            outnames[host].append(measurement_set + suffix)
            arguments = [executable, init_script, parset, working_dir,
                       measurement_set]

            jobs.append(ComputeJob(host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        created_awimages = []
        for job in  jobs:
            if job.results.has_key("image"):
                created_awimages.append((job.host, job.results["image"]))
            #TODO else: aw imager failed 

        if self.error.isSet():
            self.logger.warn("Failed awimager run detected")
            return 1

        store_data_map(self.inputs['mapfile'], created_awimages)
        self.outputs["mapfile"] = self.inputs['mapfile']
        return 0


if __name__ == "__main__":
    sys.exit(imager_awimager().main())
