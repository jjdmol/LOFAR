#                                                         LOFAR IMAGING PIPELINE
#
#                                    Example recipe with simple job distribution
#                                                          Wouter Klijn, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------
import sys
import copy
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, validate_data_maps

class imager_awimager(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Master script for the awimager. Collects arguments from command line and
    pipeline inputs.
    
    1. Load mapfiles and validate these
    2. Run the awimage node scripts
    3. Retrieve output. Construct output map file succesfull runs
    
    Details regarding the implementation of the imaging step can be found in 
    the node recipe 
    **CommandLine Arguments**
    
    A mapfile containing (node, datafile) pairs. The measurements set use as
    input for awimager executable  
 
    """
    inputs = {
        'executable': ingredient.ExecField(
            '--executable',
            help="The full path to the  awimager executable"
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="The full path to a awimager configuration parset."
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. Results location"
        ),
        'output_image': ingredient.StringField(
            '--output-image',
            help="Path of the image to be create by the awimager"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path for output mapfile. A list of the"
                 "successfully generated images will be written here"
        ),
        'sourcedb_path': ingredient.StringField(
            '--sourcedb-path',
            help="Full path of sourcedb used to create a mask for known sources"
        ),
        'mask_patch_size': ingredient.FloatField(
            '--mask-patch-size',
            help="Scale factor for patches in the awimager mask"
        ),
        'autogenerate_parameters': ingredient.BoolField(
            '--autogenerate-parameters',
            help="Turns on the autogeneration of: cellsize, image-size. MSSS like"
            "functionality"
        ),
    }

    outputs = {
        'mapfile': ingredient.StringField()
    }

    def go(self):
        """
        This member contains all the functionality of the imager_awimager.
        Functionality is all located at the node side of the script.
        """
        super(imager_awimager, self).go()
        self.logger.info("Starting imager_awimager run")

        # *********************************************************************
        # 1. collect the inputs and validate        
        input_map = DataMap.load(self.inputs['args'][0])
        sourcedb_map = DataMap.load(self.inputs['sourcedb_path'])

        if not validate_data_maps(input_map, sourcedb_map):
            self.logger.error(
                        "the supplied input_ms mapfile and sourcedb mapfile"
                        "are incorrect. Aborting")
            self.logger.error(repr(input_map))
            self.logger.error(repr(sourcedb_map))
            return 1

        # *********************************************************************
        # 2. Start the node side of the awimager recipe
        # Compile the command to be executed on the remote machine
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))
        jobs = []

        output_map = copy.deepcopy(input_map)
        for w, x, y in zip(input_map, output_map, sourcedb_map):
            w.skip = x.skip = y.skip = (
                w.skip or x.skip or y.skip
            )
        sourcedb_map.iterator = input_map.iterator = output_map.iterator = \
            DataMap.SkipIterator

        for measurement_item, source_item in zip(input_map, sourcedb_map):
            if measurement_item.skip or source_item.skip:
                jobs.append(None)
                continue
            # both the sourcedb and the measurement are in a map
            # unpack both
            host , measurement_path = measurement_item.host, measurement_item.file
            host2 , sourcedb_path = source_item.host, source_item.file

            #construct and save the output name
            arguments = [self.inputs['executable'],
                         self.environment,
                         self.inputs['parset'],
                         self.inputs['working_directory'],
                         self.inputs['output_image'],
                         measurement_path,
                         sourcedb_path,
                         self.inputs['mask_patch_size'],
                         self.inputs['autogenerate_parameters']
                         ]

            jobs.append(ComputeJob(host, node_command, arguments))
        self._schedule_jobs(jobs)

        # *********************************************************************
        # 3. Check output of the node scripts

        for job, output_item in  zip(jobs, output_map):
            # job ==  None on skipped job
            if not "image" in job.results:
                output_item.file = "failed"
                output_item.skip = True

            else:
                output_item.file = job.results["image"]
                output_item.skip = False

        # Check if there are finished runs
        succesfull_runs = None
        for item in output_map:
            if item.skip == False:
                succesfull_runs = True
                break

        if not succesfull_runs:
            self.logger.error(
                    "None of the starter awimager run finished correct")
            self.logger.error(
                    "No work left to be done: exiting with error status")
            return 1

        # If partial succes
        if self.error.isSet():
            self.logger.error("Failed awimager node run detected. continue with"
                              "successful tasks.")

        self._store_data_map(self.inputs['mapfile'], output_map,
                             "mapfile containing produces awimages")

        self.outputs["mapfile"] = self.inputs['mapfile']
        return 0


if __name__ == "__main__":
    sys.exit(imager_awimager().main())
