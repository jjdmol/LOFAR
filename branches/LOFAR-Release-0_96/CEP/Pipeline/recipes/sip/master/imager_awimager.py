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
from lofarpipe.support.group_data import load_data_map, store_data_map, validate_data_maps

class imager_awimager(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Run the imager_awimager on the nodes and the data files suplied in the mapfile
    **Arguments**
    A mapfile containing node->datafile pairs  
 
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help = "The full path to the  awimager executable"
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
            help = "Working directory used on output nodes. Results location"
        ),
        'output_image': ingredient.StringField(
            '--output-image',
            help = "Path of the image to be create by the awimager"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help = "Full path of mapfile; contains a list of the"
                 "successfully generated images"
        ),
        'sourcedb_path': ingredient.StringField(
            '--sourcedb-path',
            help = "Full path of sourcedb used to create a mask for known sources"
        ),
        'mask_patch_size': ingredient.FloatField(
            '--mask-patch-size',
            help = "Scale factor for patches in the awimager mask"
        ),
    }

    outputs = {
        'mapfile': ingredient.StringField()
    }

    def go(self):
        super(imager_awimager, self).go()
        self.logger.info("Starting imager_awimager run")

        #collect the inputs        
        input_map = load_data_map(self.inputs['args'][0])

        executable = self.inputs['executable']
        init_script = self.inputs['initscript']
        parset = self.inputs['parset']
        output_image = self.inputs['output_image']
        working_directory = self.inputs['working_directory']
        sourcedb_path = self.inputs['sourcedb_path']
        mask_patch_size = self.inputs['mask_patch_size']
        # Compile the command to be executed on the remote machine
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))
        # Create the jobs
        jobs = []
        sourcedb_map = load_data_map(sourcedb_path)
        outnames = collections.defaultdict(list)

        if not validate_data_maps(input_map, sourcedb_map):
            self.logger.error("the supplied input_ms mapfile and sourcedb mapfile"
                              "are incorrect. Aborting")
            self.logger.error(repr(input_map))
            self.logger.error(repr(sourcedb_map))

        for ms, source in zip(input_map, sourcedb_map):
            # both the sourcedb and the measurement are in a map
            # unpack both
            host , measurement_set = ms
            host2 , sourcedb_path = source

            #construct and save the output name
            outnames[host].append(measurement_set)
            arguments = [executable, init_script, parset, working_directory, output_image,
                       measurement_set, sourcedb_path, mask_patch_size]

            jobs.append(ComputeJob(host, node_command, arguments))
        self._schedule_jobs(jobs)

        created_awimages = []
        for job in  jobs:
            if job.results.has_key("image"):
                created_awimages.append((job.host, job.results["image"]))
            #TODO else: aw imager failed. Currently partial runs cannot be
            # restarted: for the next lofar version the framework needs to 
            # be expanded with a partial rerun capability 

        if self.error.isSet():
            self.logger.warn("Failed awimager node run detected")
            return 1

        store_data_map(self.inputs['mapfile'], created_awimages)
        self.logger.debug("Wrote mapfile containing produces awimages: {0}".format(
                           self.inputs['mapfile']))
        self.outputs["mapfile"] = self.inputs['mapfile']
        return 0


if __name__ == "__main__":
    sys.exit(imager_awimager().main())
