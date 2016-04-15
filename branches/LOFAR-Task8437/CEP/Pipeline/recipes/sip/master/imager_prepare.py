# LOFAR IMAGING PIPELINE
# Prepare phase master
# 
# 1. Create input files for individual nodes based on the  input mapfile
# 2. Perform basic input parsing and input validation
# 3. Call the node scripts with correct input
# 4. validate performance
#
# Wouter Klijn 
# 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import os
import sys
import copy
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.data_map import DataMap, MultiDataMap

class imager_prepare(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Prepare phase master:

    1. Validate input
    2. Create mapfiles with input for work to be perform on the individual nodes
       based on the structured input mapfile. The input mapfile contains a list 
       of measurement sets. 
       Each node computes a single subband group but needs this for all
       timeslices. 
    3. Call the node scripts with correct input
    4. validate performance
       Only output the measurement nodes that finished succesfull

    **Command Line arguments:**

    The only command line argument is the a to a mapfile containing "all"
    the measurement sets needed for creating the sky images. First ordered on 
    timeslice then on subband group and finaly on index in the frequency
    range.

    **Arguments:**
    """

    inputs = {
        'ndppp_exec': ingredient.ExecField(
            '--ndppp-exec',
            help="The full path to the ndppp executable"
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="The full path to a prepare parset"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used by the nodes: local data"
        ),
        'nthreads': ingredient.IntField(
            '--nthreads',
            default=8,
            help="Number of threads per process"
        ),
        'target_mapfile': ingredient.StringField(
            '--target-mapfile',
            help="Contains the node and path to target files, defines"
               " the number of nodes the script will start on."
        ),
        'slices_per_image': ingredient.IntField(
            '--slices-per-image',
            help="The number of (time) slices for each output image"
        ),
        'subbands_per_image': ingredient.IntField(
            '--subbands-per-image',
            help="The number of subbands to be collected in each output image"
        ),
        'asciistat_executable': ingredient.ExecField(
            '--asciistat-executable',
            help="full path to the ascii stat executable"
        ),
        'statplot_executable': ingredient.ExecField(
            '--statplot-executable',
            help="The full path to the statplot executable"
        ),
        'msselect_executable': ingredient.ExecField(
            '--msselect-executable',
            help="The full path to the msselect executable "
        ),
        'rficonsole_executable': ingredient.ExecField(
            '--rficonsole-executable',
            help="The full path to the rficonsole executable "
        ),
        'do_rficonsole': ingredient.BoolField(
            '--do_rficonsole',
            default=True,
            help="toggle the rficonsole step in preprocessing (default True)"
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help="Full path of mapfile; contains a list of the "
                 "successfully generated and concatenated sub-band groups"
        ),
        'slices_mapfile': ingredient.StringField(
            '--slices-mapfile',
            help="Path to mapfile containing the produced subband groups"
        ),
        'ms_per_image_mapfile': ingredient.StringField(
            '--ms-per-image-mapfile',
            help="Path to mapfile containing the ms for each produced"
                "image"
        ),
        'processed_ms_dir': ingredient.StringField(
            '--processed-ms-dir',
            help="Path to directory for processed measurment sets"
        ),
        'add_beam_tables': ingredient.BoolField(
            '--add_beam_tables',
            default=False,
            help="Developer option, adds beamtables to ms"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help="path to a mapfile Which contains a list of the"
                 "successfully generated and concatenated measurement set"
            ),
        'slices_mapfile': ingredient.FileField(
            help="Path to mapfile containing the produced subband groups"),

        'ms_per_image_mapfile': ingredient.FileField(
            help="Path to mapfile containing the used ms for each produced"
                "image")
    }

    def go(self):
        """
        Entry point for recipe: Called by the pipeline framework
        """
        super(imager_prepare, self).go()
        self.logger.info("Starting imager_prepare run")
        job_directory = self.config.get("layout", "job_directory")
        # *********************************************************************
        # input data     
        input_map = DataMap.load(self.inputs['args'][0])
        output_map = DataMap.load(self.inputs['target_mapfile'])
        slices_per_image = self.inputs['slices_per_image']
        subbands_per_image = self.inputs['subbands_per_image']
        # Validate input
        if not self._validate_input_map(input_map, output_map, slices_per_image,
                            subbands_per_image):
            return 1

        # outputs
        output_ms_mapfile_path = self.inputs['mapfile']

        # *********************************************************************
        # schedule the actual work
        # TODO: Refactor this function into: load data, perform work, 
        # create output
        node_command = " python %s" % (self.__file__.replace("master", "nodes"))

        jobs = []
        paths_to_image_mapfiles = []
        n_subband_groups = len(output_map)  # needed for subsets in sb list

        globalfs = self.config.has_option("remote", "globalfs") and self.config.getboolean("remote", "globalfs")

        for idx_sb_group, item in enumerate(output_map):
            #create the input files for this node
            self.logger.debug("Creating input data subset for processing"
                              "on: {0}".format(item.host))
            inputs_for_image_map = \
                self._create_input_map_for_sbgroup(
                                slices_per_image, n_subband_groups,
                                subbands_per_image, idx_sb_group, input_map)

            # Save the mapfile
            inputs_for_image_mapfile_path = os.path.join(
               job_directory, "mapfiles",
               "ms_per_image_{0}.map".format(idx_sb_group))

            self._store_data_map(inputs_for_image_mapfile_path,
                                inputs_for_image_map, "inputmap for location")

            # skip the current step if skip is set, cannot use skip due to 
            # the enumerate: dependency on the index in the map
            if item.skip == True:
                # assure that the mapfile is correct
                paths_to_image_mapfiles.append(
                    tuple([item.host, [], True]))
                continue

            #save the (input) ms, as a list of  mapfiles
            paths_to_image_mapfiles.append(
                tuple([item.host, inputs_for_image_mapfile_path, False]))

            # use unique working directories per job, to prevent interference between jobs on a global fs
            working_dir = os.path.join(self.inputs['working_directory'], "imager_prepare_{0}".format(idx_sb_group))

            arguments = [self.environment,
                         self.inputs['parset'],
                         working_dir,
                         self.inputs['processed_ms_dir'],
                         self.inputs['ndppp_exec'],
                         item.file,
                         slices_per_image,
                         subbands_per_image,
                         inputs_for_image_mapfile_path,
                         self.inputs['asciistat_executable'],
                         self.inputs['statplot_executable'],
                         self.inputs['msselect_executable'],
                         self.inputs['rficonsole_executable'],
                         self.inputs['do_rficonsole'],
                         self.inputs['add_beam_tables'],
                         globalfs]

            jobs.append(ComputeJob(item.host, node_command, arguments,
                    resources={
                        "cores": self.inputs['nthreads']
                    }))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)

        # *********************************************************************
        # validate the output, cleanup, return output
        if self.error.isSet():   #if one of the nodes failed
            self.logger.warn("Failed prepare_imager run detected: Generating "
                             "new output_ms_mapfile_path without failed runs:"
                             " {0}".format(output_ms_mapfile_path))

        concat_ms = copy.deepcopy(output_map)
        slices = []
        finished_runs = 0
        #scan the return dict for completed key
        # loop over the potential jobs including the skipped
        # If we have a skipped item, add the item to the slices with skip set
        jobs_idx = 0
        for item in concat_ms: 
            # If this is an item that is skipped via the skip parameter in 
            # the parset, append a skipped             
            if item.skip:    
                slices.append(tuple([item.host, [], True]))
                continue

            # we cannot use the skip iterator so we need to manually get the
            # current job from the list
            job = jobs[jobs_idx]

            # only save the slices if the node has completed succesfull
            if job.results["returncode"] == 0:
                finished_runs += 1
                slices.append(tuple([item.host,
                                 job.results["time_slices"], False]))
            else:
                # Set the dataproduct to skipped!!
                item.skip = True
                slices.append(tuple([item.host, [], True]))
                msg = "Failed run on {0}. NOT Created: {1} ".format(
                    item.host, item.file)
                self.logger.warn(msg)

            # we have a non skipped workitem, increase the job idx
            jobs_idx += 1

        if finished_runs == 0:
            self.logger.error("None of the started compute node finished:"
                "The current recipe produced no output, aborting")
            return 1

        # Write the output mapfiles:
        # concat.ms paths:
        self._store_data_map(output_ms_mapfile_path, concat_ms,
                    "mapfile with concat.ms")

        # timeslices
        MultiDataMap(slices).save(self.inputs['slices_mapfile'])
        self.logger.info(
            "Wrote MultiMapfile with produces timeslice: {0}".format(
                self.inputs['slices_mapfile']))

        #map with actual input mss.
        self._store_data_map(self.inputs["ms_per_image_mapfile"],
            DataMap(paths_to_image_mapfiles),
                "mapfile containing (used) input ms per image:")

        # Set the return values
        self.outputs['mapfile'] = output_ms_mapfile_path
        self.outputs['slices_mapfile'] = self.inputs['slices_mapfile']
        self.outputs['ms_per_image_mapfile'] = \
            self.inputs["ms_per_image_mapfile"]
        return 0

    def _create_input_map_for_sbgroup(self, slices_per_image,
            n_subband_groups, subbands_per_image, idx_sb_group, input_mapfile):
        """
        Creates an input mapfile:
        This is a subset of the complete input_mapfile based on the subband 
        details suplied: The input_mapfile is structured: First all subbands for
        a complete timeslice and the the next timeslice. The result value 
        contains all the information needed for a single subbandgroup to be
        computed on a single compute node
        """
        inputs_for_image = []
        # collect the inputs: first step over the time slices
        for idx_slice in range(slices_per_image):
            # calculate the first line for current time slice and subband group
            line_idx_start = idx_slice * \
                (n_subband_groups * subbands_per_image) + \
                (idx_sb_group * subbands_per_image)
            line_idx_end = line_idx_start + subbands_per_image

            #extend inputs with the files for the current time slice
            inputs_for_image.extend(input_mapfile[line_idx_start: line_idx_end])

        return DataMap(inputs_for_image)


    def _validate_input_map(self, input_map, output_map, slices_per_image,
                            subbands_per_image):
        """
        Return False if the inputs supplied are incorrect:
        the number if inputs and  output does not match. 
        Return True if correct.              
        The number of inputs is correct iff.
        len(input_map) == 
        len(output_map) * slices_per_image * subbands_per_image
        """
        # The output_map contains a number of path/node pairs. The final data 
        # dataproduct of the prepare phase: The 'input' for each of these pairs
        # is a number of measurement sets: The number of time slices times
        # the number of subbands collected into each of these time slices.
        # The total length of the input map should match this.
        if len(input_map) != len(output_map) * \
                                   (slices_per_image * subbands_per_image):
            self.logger.error(
                "Incorrect number of input ms for supplied parameters:\n\t"
                "len(input_map) = {0}\n\t"
                "len(output_map) * slices_per_image * subbands_per_image = "
                "{1} * {2} * {3} = {4}".format(
                    len(input_map), len(output_map),
                    slices_per_image, subbands_per_image,
                    len(output_map) * slices_per_image * subbands_per_image
                )
            )
            return False

        return True


if __name__ == "__main__":
    sys.exit(imager_prepare().main())
