#                                                         LOFAR IMAGING PIPELINE
#
#                                    Example recipe with simple job distribution
#                                                          Wouter Klijn, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------
import sys
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, validate_data_maps

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
        input_map = load_data_map(self.inputs['args'][0])
        sourcedb_map = load_data_map(self.inputs['sourcedb_path'])

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
        for measurement_set, source in zip(input_map, sourcedb_map):
            # both the sourcedb and the measurement are in a map
            # unpack both
            host , measurement_set = measurement_set
            host2 , sourcedb_path = source

            #construct and save the output name
            arguments = [self.inputs['executable'],
                         self.environment,
                         self.inputs['parset'],
                          self.inputs['working_directory'],
                         self.inputs['output_image'],
                         measurement_set,
                         sourcedb_path,
                         self.inputs['mask_patch_size']]

            jobs.append(ComputeJob(host, node_command, arguments))
        self._schedule_jobs(jobs)

        # *********************************************************************
        # 3. Check output of the node scripts
        created_awimages = []
        for job in  jobs:
            if job.results.has_key("image"):
                created_awimages.append((job.host, job.results["image"]))

        # If not succesfull runs abort
        if len(created_awimages) == 0:
            self.logger.error(
                    "None of the starter awimager run finished correct")
            self.logger.error(
                    "No work left to be done: exiting with error status")
            return 1

        # If partial succes
        if self.error.isSet():
            self.logger.error("Failed awimager node run detected. continue with"
                              "successful tasks.")

        self._store_data_map(self.inputs['mapfile'], created_awimages,
                             "mapfile containing produces awimages")

        self.outputs["mapfile"] = self.inputs['mapfile']
        return 0


if __name__ == "__main__":
    sys.exit(imager_awimager().main())
