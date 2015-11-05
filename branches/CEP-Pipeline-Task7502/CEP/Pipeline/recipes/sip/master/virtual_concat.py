#                                                         LOFAR IMAGING PIPELINE
#
#                                                    Virtual concat master recipe
#                                                            Wouter Klijn 2015
#                                                                klijn@astron.nl
# ------------------------------------------------------------------------------

import sys
import lofarpipe.support.lofaringredient as ingredient

from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.data_map import DataMap, DataProduct, validate_data_maps, \
        align_data_maps, MultiDataMap, MultiDataProduct


from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn

class virtual_concat(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    New style recipe pair.
    The new style recipe is data centric: It always has the same
    number of input mapfiles as output mapfiles. This allows perfect information
    regarding possible failed work items during processing.
    This means that a recipe wich processes a data set should
    provide both a set of mapfiles for the input location and the output location.
    And it should provide a set of mapfile location where the results of
    the processing step can be stored. This set will be written by the 
    recipe and will contain an updates set of skip parameters.

    A second idea incorporated in the newstyle recipe pair is the formalization
    of a number of boilerplate code patterns repeated in the old master recipes:

    1. mapfiles always need to be loaded, validated and the skipfield need to be
    aligned. 
    2. there is always a set of jobs to start with some parameters.
    3. The results of the jobs should be applied to the mapfiles used to 
       start the jobs
    4. The updates mapfiles need to be written to file
    5. Exit with the correct exit value (zero if there are succesfull jobs
      one if there were no completed jobs)

    Todo: 

    Perform a virtual concat of the supplied measurements sets
    A virtual measurement set will be created: It references 
    the input ms with a singular measurement set interface
    This is a many to one reduce step 
    
    **Arguments**
    
    A mapfile describing the data to be processed.
    """
    inputs = {
        'input_ms': ingredient.MultiDataMapField(
            '--input_multimapfile',
            help="Full path to the mapfile containing the input measurement "
                 "sets to be combined into a single one"
        ),
        'output_ms': ingredient.DataMapField(
            '--output_mapfile',
            help="Full path to the mapfile containing the names of the "
                 "virtual measurementset to create"
        ),
    }
    outputs = {
        'input_multimapfile': ingredient.StringField(
            help="Full path to a mapfile describing the processed data"
                 " it contains the original input mapfile with skipfiles"
                 " set where needed (on error etc.)"
        ),
        'output_mapfile': ingredient.StringField(
            help="Full path to a mapfile describing the processed data"
                 " it contains the original input mapfile with skipfiles"
                 " set where needed (on error etc.)"
        )
    }
    

    def __init__(self):
        """
        Initialize our data members.
        """
        super(virtual_concat, self).__init__()


    def _load_and_align_mapfiles(self):
        """
        Load all data map files in the input, validate that they
        represent a valid data combination and align the skip fields
        """
        self.logger.debug("Loading map files:")

        data_maps = {}
        for (key, value) in self.inputs.items:
            # Grab all mapfiles from the input
            if value is ingredient.DataMapField:
                data_maps[key] = DataMap.load(mapfile)
            elif value is ingredient.MultiDataMapField:
                data_maps[key] = MultiDataMap.load(mapfile)
            else:
                continue

            self.logger.debug(mapfile)


        if not validate_data_maps(data_maps.values):
            self.logger.error("Validation of input data mapfiles failed")
            return False, []

        # Update the skip fields of the three maps. If 'skip' is True in any of
        # these maps, then 'skip' must be set to True in all maps.
        align_data_maps(data_maps.values)
        
        return True, data_maps
    

    def _create_argument_list(arguments_dict):
        """
        Template function that creates, based on the mapfiles and
        the script argument, a dict of arguments that will be supplied to 
        the node recipe.
        
        """
        # this would be the place to add additional 'global'
        # arguments
        arguments=[self.environment]

        # This entails a tight coupling between the node and recipe
        arguments.append(arguments_dict['input_ms'])
        arguments.append(arguments_dict['output_ms'])

        # add aditional items
        return arguments

    def _run_jobs(self, data_maps):
        """
        Create and schedule the compute jobs
        """
        command = "python %s" % (self.__file__.replace('master', 'nodes'))

        # only run on the non skipped entries
        for map in data_maps.values():
            if map is DataMap:
                map.iterator = DataMap.SkipIterator
            else: 
                map.iterator = MultiDataMap.SkipIterator

        # create a set of jobs 
        jobs = []
        # create mapfile entries, use unpack on the list of maps return
        # by the dict 
        for map_entries in zip(*(data_maps.values())):
            # zip the mapfile entries with the names of the mapfiles
            # and create a dict out of the pairs
            arguments = self._create_argument_list(
                dict(zip(data_maps.keys(), map_entries)))
            jobs.append(
                ComputeJob(
                    data.host, command, 
                    arguments
                )
            )

        # wait for all the jobs to complete and return 
        self._schedule_jobs(jobs)

        return jobs


    def _update_mapfiles(self, jobs, data_maps):
        """
        Update the mapfiles taking in account any failed runs
        The supplied DataMaps should have the skipIterator set
        """
        # create a big list of the jobs and the data_maps
        # It expects a set of mapfiles with the skip parameter set
        for job_and_maps_entrie in zip([self.jobs].extend(data_maps)):
            if job_and_maps_entrie[0].results['returncode'] != 0:
                for map_entrie in job_and_maps_entrie[1:]:
                    map_entrie.skip = True

    
    def _save_mapfiles(self, input_maps, output_mapfiles):
        """

        """
        for map, path in zip(input_maps, output_mapfiles):
            map.save(path)



    def _handle_errors(self):
        """
        Handle errors from the node scripts. If all jobs returned a (fatal)
        error, then the recipe should abort; return 1.
        Otherwise it should report that some jobs failed and continue with the
        remaining, successfully processed Measurement Set files; return 0.
        """
        if self.error.isSet(jobs):
            # Abort if all jobs failed
            if all(job.results['returncode'] != 0 for job in jobs):
                self.logger.error("All virtual_contat jobs failed. Bailing out!")
                return 1
            else:
                self.logger.warn(
                    "Some virtual_contat jobs failed, "
                    "continuing with succeeded runs"
            )
        return 0


    def go(self):
        """
        This it the actual workhorse. It is called by the framework. 
        """
        self.logger.info("Starting virtual_concat run")
        super(virtual_concat, self).go()

        # Load the required map-files.
        load_succesfull, input_maps =  \
            self._load_and_align_mapfiles(input_mapfiles,
                                          output_mapfiles)
        if not load_succesfull:
            return 1

        # Create and schedule the compute jobs
        jobs = self._run_jobs(input_maps)

        # Update the mapfiles taking failed runs into account.
        self._update_mapfiles(jobs, input_maps)

        # Save the updated mapfiles to a new output mapfile
        self._save_mapfiles(input_maps, output_mapfiles)

        # Handle errors, if any.
        return self._handle_errors(jobs)


if __name__ == '__main__':
    sys.exit(virtual_concat().main())

