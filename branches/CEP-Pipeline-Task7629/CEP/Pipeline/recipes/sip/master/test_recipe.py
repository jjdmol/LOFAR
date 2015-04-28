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

class test_recipe(BaseRecipe, RemoteCommandRecipeMixIn):
    """
 
    """
    inputs = {
        'executable': ingredient.StringField(
            '--executable',
            help = "A parameter"
        ),
        'output_map_path': ingredient.StringField(
            '--output-map-path',
            help = "The path for the output mapfile"
        )
    }

    outputs = {
        'mapfile': ingredient.StringField(),
    }

    def go(self):
        """
        This member contains all the functionality of the imager_awimager.
        Functionality is all located at the node side of the script.
        """
        super(test_recipe, self).go()
        self.logger.info("Starting test_recipe run")

        # *********************************************************************
        # 1. collect the inputs and validate
        input_map = DataMap.load(self.inputs['args'][0])

        if not validate_data_maps(input_map):
            self.logger.error(
                        "the supplied input_ms mapfile is corrupt")
            self.logger.error(repr(input_map))
            return 1

        # *********************************************************************
        # 2. Start the node side of the awimager recipe
        # Compile the command to be executed on the remote machine
        node_command = "python %s" % (self.__file__.replace("master", "nodes"))
        jobs = []

        output_map = copy.deepcopy(input_map)        
        for w, x in zip(input_map, output_map):
            w.skip = x.skip = (
                w.skip or x.skip 
            )

        input_map.iterator = output_map.iterator = \
            DataMap.SkipIterator

        for measurement_item in input_map:
            if measurement_item.skip:
                jobs.append(None)
                continue
            # both the sourcedb and the measurement are in a map
            # unpack both
            host , measurement_path = measurement_item.host, measurement_item.file

            # construct and save the output name
            arguments = [[measurement_path],
                         self.inputs['executable'],
                         self.environment

                         ]

            jobs.append(ComputeJob(host, node_command, arguments))
        self._schedule_jobs(jobs)

        # *********************************************************************
        # 3. Check output of the node scripts

        for job, output_item in  zip(jobs, output_map):
            # job ==  None on skipped job
            if not "output" in job.results:
                output_item.file = "failed"
                output_item.skip = True

            else:
                output_item.file = job.results["output"]
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
            self.logger.warn("Failed awimager node run detected. continue with"
                              "successful tasks.")

        self._store_data_map(self.inputs['output_map_path'], output_map,
                             "mapfile containing produces awimages")

        self.outputs["mapfile"] = self.inputs['output_map_path']
        return 0


if __name__ == "__main__":
    sys.exit(test_recipe().main())

