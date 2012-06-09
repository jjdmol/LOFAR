# LOFAR IMAGING PIPELINE
# Prepare phase master
# 
# 1. Create input files for individual nodes based on the structured input mapfile
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
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import store_data_map, load_data_map
from lofarpipe.support.utilities import create_directory

class imager_prepare(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Prepare phase master
 
    1. Create input files for individual nodes based on the structured input mapfile
    2. Perform basic input parsing and input validation
    3. Call the node scripts with correct input
    4. validate performance
    
    node functionality:
    
    1. Collect the Measurement Sets (MSs): copy to the  current node
    2. Start dppp: Combines the data from subgroups into single timeslice
    3. Add addImagingColumns to the casa images
    4. Concatenate the time slice measurement sets, to a virtual ms 
    
    **Arguments**

    """

    inputs = {
        'ndppp_exec': ingredient.ExecField(
            '--ndppp-exec',
            help = "The full path to the ndppp executable"
        ),
        'initscript': ingredient.FileField(
            '--initscript',
            help = '''The full path to an (Bourne) shell script which will\
             intialise the environment (ie, ``lofarinit.sh``)'''
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help = "The full path to a prepare parset (mainly ndppp)"
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help = "Working directory used by the nodes: local data"
        ),
        'target_mapfile': ingredient.StringField(
            '--target-mapfile',
            help = "Contains the node and path to target product files, defines the"
               " number of nodes the script will start on."
        ),
        'slices_per_image': ingredient.IntField(
            '--slices-per-image',
            help = "The number of (time) slices for each output image"
        ),
        'subbands_per_image': ingredient.IntField(
            '--subbands-per-image',
            help = "The number of subbands to be collected in each output image"
        ),
        'asciistat_executable': ingredient.ExecField(
            '--asciistat-executable',
            help = "full path to the ascii stat executable"
        ),
        'statplot_executable': ingredient.ExecField(
            '--statplot-executable',
            help = "full path to the statplot executable"
        ),
        'msselect_executable': ingredient.ExecField(
            '--msselect-executable',
            help = "full path to the msselect executable "
        ),
        'rficonsole_executable': ingredient.ExecField(
            '--rficonsole-executable',
            help = "full path to the rficonsole executable "
        ),
        'mapfile': ingredient.StringField(
            '--mapfile',
            help = "Full path of mapfile; contains a list of the"
                 "successfully generated and concatenated sub-band groups:"
        ),
        'slices_mapfile': ingredient.StringField(
            '--slices-mapfile',
            help = "Path to mapfile containing the produced subband groups"
        ),
        'raw_ms_per_image_mapfile': ingredient.StringField(
            '--raw-ms-per-image-mapfile',
            help = "Path to mapfile containing the raw ms for each produced"
                "image"
        ),
        'processed_ms_dir': ingredient.StringField(
            '--processed-ms-dir',
            help = "Path to directory for processed measurment sets"
        )
    }

    outputs = {
        'mapfile': ingredient.FileField(
            help = "path to a mapfile Which contains a list of the"
                 "successfully generated and concatenated measurement set"
            ),
        'slices_mapfile': ingredient.FileField(
            help = "Path to mapfile containing the produced subband groups"),

        'raw_ms_per_image_mapfile': ingredient.FileField(
            help = "Path to mapfile containing the raw ms for each produced"
                "image")
    }

    def go(self):
        """
        Main function for recipe: Called by the pipeline framework
        """
        super(imager_prepare, self).go()
        self.logger.info("Starting imager_prepare run")

        # *********************************************************************
        # input data     
        input_map = load_data_map(self.inputs['args'][0])
        output_map = load_data_map(self.inputs['target_mapfile'])
        slices_per_image = self.inputs['slices_per_image']
        subbands_per_image = self.inputs['subbands_per_image']
        # Validate input
        if self._validate_input_map(input_map, output_map, slices_per_image,
                            subbands_per_image):
            return 1

        # outputs
        output_ms_mapfile_path = self.inputs['mapfile']
        output_slices_mapfile_path = self.inputs['slices_mapfile']
        processed_ms_dir = self.inputs['processed_ms_dir']

        # Environment parameters
        init_script = self.inputs['initscript']
        parset = self.inputs['parset']
        working_directory = self.inputs['working_directory']
        ndppp_exec = self.inputs['ndppp_exec']
        asciistat_executable = self.inputs['asciistat_executable']
        statplot_executable = self.inputs['statplot_executable']
        msselect_executable = self.inputs['msselect_executable']
        rficonsole_executable = self.inputs['rficonsole_executable']


        # *********************************************************************            
        # schedule the actual work
        nodeCommand = " python %s" % (self.__file__.replace("master", "nodes"))

        jobs = []
        inputs_for_image_mapfile_path_list = []
        n_subband_groups = len(output_map)
        for idx_sb_group, (host, output_measurement_set) in enumerate(output_map):
            #create the input files for this node
            self.logger.debug("Creating input data subset for processing"
                              "on: {0}".format(host))
            inputs_for_image_mapfile_path = self._create_input_map_for_subband_group(
                                slices_per_image, n_subband_groups,
                                subbands_per_image, idx_sb_group, input_map)

            #save the (input) ms, as a list of  
            inputs_for_image_mapfile_path_list.append((host,
                                            inputs_for_image_mapfile_path))

            arguments = [init_script, parset, working_directory,
                        processed_ms_dir,
                        ndppp_exec, output_measurement_set,
                        slices_per_image, subbands_per_image,
                        inputs_for_image_mapfile_path, asciistat_executable,
                        statplot_executable, msselect_executable,
                        rficonsole_executable]

            jobs.append(ComputeJob(host, nodeCommand, arguments))

        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)


        # *********************************************************************
        # validate the output, cleanup, return output
        slices = []
        if self.error.isSet():   #if one of the nodes failed
            self.logger.warn("Failed prepare_imager run detected: Generating "
                             "new output_ms_mapfile_path without failed runs:"
                             " {0}".format(output_ms_mapfile_path))
            concatenated_timeslices = []
            #scan the return dict for completed key
            for ((host, output_measurement_set), job) in zip(output_map, jobs):
                if job.results.has_key("completed"):
                    concatenated_timeslices.append((host, output_measurement_set))

                    #only save the slices if the node has completed succesfull
                    if job.results.has_key("time_slices"):
                        slices.append((host, job.results["time_slices"]))
                else:
                    self.logger.warn("Failed run on {0}. NOT Created: {1} ".format(
                        host, output_measurement_set))
            if len(concatenated_timeslices) == 0:
                self.logger.error("None of the started compute node finished:"
                    "The current recipe produced no output, aborting")
                return 1

            store_data_map(output_ms_mapfile_path, concatenated_timeslices)
            self.logger.debug(
                "Wrote target mapfile: {0}".format(output_ms_mapfile_path))

        else: #Copy output map from input output_ms_mapfile_path and return           
            store_data_map(output_ms_mapfile_path, output_map)
            for ((host, output_measurement_set), job) in zip(output_map, jobs):
                if job.results.has_key("time_slices"):
                    slices.append((host, job.results["time_slices"]))

        store_data_map(output_slices_mapfile_path, slices)
        self.logger.debug(
                "Wrote Time_slice mapfile: {0}".format(output_ms_mapfile_path))
        store_data_map(self.inputs["raw_ms_per_image_mapfile"],
                       inputs_for_image_mapfile_path_list)
        self.logger.debug(
                "Wrote mapfile containing (raw) input ms: {0}".format(
                    self.inputs["raw_ms_per_image_mapfile"]))
        # Set the outputs
        self.outputs['mapfile'] = self.inputs["mapfile"]
        self.outputs['slices_mapfile'] = self.inputs["slices_mapfile"]
        self.outputs['raw_ms_per_image_mapfile'] = self.inputs["raw_ms_per_image_mapfile"]
        return 0


    def _create_input_map_for_subband_group(self, slices_per_image, n_subband_groups,
                        subbands_per_image, idx_sb_group, input_mapfile):
        """
        _create_input_map_for_subband_group() Creates an input mapfile representation:
        This is a subset of the complete input_mapfile based on the subband 
        details suplied: The input_mapfile is structured: First all subbands for
        a complete timeslice and the the next timeslice. The result value 
        contains all the information needed for a single subbandgroup to be computed 
        on a single compute node
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

        job_directory = self.config.get(
                            "layout", "job_directory")

        inputs_for_image_mapfile_path = os.path.join(
            job_directory, "mapfiles", "ms_per_image_{0}".format(idx_sb_group))

        self.logger.debug("Storing inputmap on location: {0}".format(
                                    inputs_for_image_mapfile_path))
        store_data_map(inputs_for_image_mapfile_path, inputs_for_image)
        return inputs_for_image_mapfile_path


    def _validate_input_map(self, input_map, output_map, slices_per_image,
                            subbands_per_image):
        """
        Return 1 if the inputs supplied are incorrect, the number if inputs and 
        output does not match. Return 0 if correct  
        """
        # The output_map contains a number of path/node pairs. The final data 
        # dataproduct of the prepare phase: The 'input' for each of these pairs
        # is a number of raw measurement sets: The number of time slices times
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
            return 1

        return 0


if __name__ == "__main__":
    sys.exit(imager_prepare().main())
