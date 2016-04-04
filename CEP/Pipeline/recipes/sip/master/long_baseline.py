# LOFAR IMAGING PIPELINE
# long basseline master
# 
# 1. Create input files for individual nodes based on the  input mapfile
# 2. Perform basic input parsing and input validation
# 3. Call the node scripts with correct input
# 4. validate performance
#
# Wouter Klijn 
# 2014
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

class long_baseline(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Long baseline master:

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
        'nproc': ingredient.IntField(
            '--nproc',
            default=1,   # More then one might cause issues when ndppp shares 
                         # temp files between runs
            help="Maximum number of simultaneous processes per output node"
        ),
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
        'target_mapfile': ingredient.StringField(
            '--target-mapfile',
            help="Contains the node and path to target files, defines"
               " the number of nodes the script will start on."
        ),
        'subbandgroups_per_ms': ingredient.IntField(
            '--slices-per-image',
            help="The number of (time) slices for each output image"
        ),
        'subbands_per_subbandgroup': ingredient.IntField(
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
        ),
        'output_ms_mapfile': ingredient.StringField(
            '--output-ms-mapfile',
            help="Path to mapfile which contains the the final output locations"
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
            help="Path to mapfile containing the ms for each produced"
                "image")
    }

    def go(self):
        """
        Entry point for recipe: Called by the pipeline framework
        """
        super(long_baseline, self).go()
        self.logger.info("Starting long_baseline run")

        # *********************************************************************
        # input data     
        input_map = DataMap.load(self.inputs['args'][0])
        output_map = DataMap.load(self.inputs['target_mapfile'])
        subbandgroups_per_ms = self.inputs['subbandgroups_per_ms']
        subbands_per_subbandgroup = self.inputs['subbands_per_subbandgroup']
        final_output_map = DataMap.load(self.inputs['output_ms_mapfile'])
        # Validate input
        if not self._validate_input_map(input_map, output_map, subbandgroups_per_ms,
                            subbands_per_subbandgroup):
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
        n_subband_groups = len(output_map)

        globalfs = self.config.has_option("remote", "globalfs") and self.config.getboolean("remote", "globalfs")

        output_map.iterator = final_output_map.iterator = DataMap.SkipIterator
        for idx_sb_group, (output_item, final_item) in enumerate(zip(output_map, 
                                                            final_output_map)):
            #create the input files for this node
            self.logger.debug("Creating input data subset for processing"
                              "on: {0}".format(output_item.host))
            inputs_for_image_map = \
                self._create_input_map_for_sbgroup(
                                subbandgroups_per_ms, n_subband_groups,
                                subbands_per_subbandgroup, idx_sb_group, input_map)

            # Save the mapfile
            job_directory = self.config.get(
                            "layout", "job_directory")
            inputs_for_image_mapfile_path = os.path.join(
               job_directory, "mapfiles",
               "ms_per_image_{0}".format(idx_sb_group))
            self._store_data_map(inputs_for_image_mapfile_path,
                                inputs_for_image_map, "inputmap for location")

            #save the (input) ms, as a list of  mapfiles
            paths_to_image_mapfiles.append(
                tuple([output_item.host, inputs_for_image_mapfile_path, False]))

            # use a unique working directory per job, to prevent interference between jobs on a global fs
            working_dir = os.path.join(self.inputs['working_directory'], "ms_per_image_{0}".format(idx_sb_group))

            arguments = [self.environment,
                         self.inputs['parset'],
                         working_dir,
                         self.inputs['processed_ms_dir'],
                         self.inputs['ndppp_exec'],
                         output_item.file,
                         subbandgroups_per_ms,
                         subbands_per_subbandgroup,
                         inputs_for_image_mapfile_path,
                         self.inputs['asciistat_executable'],
                         self.inputs['statplot_executable'],
                         self.inputs['msselect_executable'],
                         self.inputs['rficonsole_executable'],
                         self.inputs['add_beam_tables'],
                         globalfs,
                         final_item.file]

            jobs.append(ComputeJob(output_item.host, node_command, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs, max_per_node=self.inputs['nproc'])

        # *********************************************************************
        # validate the output, cleanup, return output
        if self.error.isSet():   #if one of the nodes failed
            self.logger.warn("Failed prepare_imager run detected: Generating "
                             "new output_ms_mapfile_path without failed runs:"
                             " {0}".format(output_ms_mapfile_path))

        concat_ms = copy.deepcopy(output_map)
        slices = []
        finished_runs = 0
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
                "mapfile containing input ms per image:")

        # Set the return values
        self.outputs['mapfile'] = output_ms_mapfile_path
        self.outputs['slices_mapfile'] = self.inputs['slices_mapfile']
        self.outputs['ms_per_image_mapfile'] = \
            self.inputs["ms_per_image_mapfile"]

        return 0

    def _create_input_map_for_sbgroup(self, subbandgroups_per_ms,
            n_subband_groups, subbands_per_subbandgroup, idx_sb_group, input_mapfile):
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
        for idx_slice in range(subbandgroups_per_ms):
            # calculate the first line for current time slice and subband group
            line_idx_start = idx_slice * \
                (n_subband_groups * subbands_per_subbandgroup) + \
                (idx_sb_group * subbands_per_subbandgroup)
            line_idx_end = line_idx_start + subbands_per_subbandgroup

            #extend inputs with the files for the current time slice
            inputs_for_image.extend(input_mapfile[line_idx_start: line_idx_end])

        return DataMap(inputs_for_image)


    def _validate_input_map(self, input_map, output_map, subbandgroups_per_ms,
                            subbands_per_subbandgroup):
        """
        Return False if the inputs supplied are incorrect:
        the number if inputs and  output does not match. 
        Return True if correct.              
        The number of inputs is correct iff.
        len(input_map) == 
        len(output_map) * subbandgroups_per_ms * subbands_per_subbandgroup
        """
        # The output_map contains a number of path/node pairs. The final data 
        # dataproduct of the prepare phase: The 'input' for each of these pairs
        # is a number of measurement sets: The number of time slices times
        # the number of subbands collected into each of these time slices.
        # The total length of the input map should match this.
        if len(input_map) != len(output_map) * \
                                   (subbandgroups_per_ms * subbands_per_subbandgroup):
            self.logger.error(
                "Incorrect number of input ms for supplied parameters:\n\t"
                "len(input_map) = {0}\n\t"
                "len(output_map) * subbandgroups_per_ms * subbands_per_subbandgroup = "
                "{1} * {2} * {3} = {4}".format(
                    len(input_map), len(output_map),
                    subbandgroups_per_ms, subbands_per_subbandgroup,
                    len(output_map) * subbandgroups_per_ms * subbands_per_subbandgroup
                )
            )
            return False

        return True


if __name__ == "__main__":
    sys.exit(long_baseline().main())
