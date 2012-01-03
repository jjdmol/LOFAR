# LOFAR IMAGING PIPELINE
#
# Prepare phase of the imager pipeline: master node (also see node recipe)
# 
# 1. Create input files for individual nodes based on the structered input mapfile
# 2. Perform basic input parsing and input validation
# 3. Call the nodescipts with correct input
# 4. validate performance (minimal in the current implementation)
#
# Wouter Klijn 
# 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------
# TODO: command line argument example:
#python prepare_imager.py ~/build/preparation/input.map --job prepare_imager --config ~/build/preparation/pipeline.cfg --initscript /opt/cep/LofIm/daily/lofar/lofarinit.sh --output-mapfile ~/build/preparation/output.map --parset ~/build/preparation/parset.par --working-directory "/data/scratch/klijn" --subbands-per-image 10 --slices-per-image 9 --ndppp /opt/cep/LofIm/daily/lofar/bin/NDPPP -v

#parset:
#msin.missingdata=true
#msin.baseline="[CR]S*&"
#msin.datacolumn=DATA
#steps=[]


import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob

class prepare_imager(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Run the AWImager on the nodes and the data files suplied in the mapfile
    **Arguments**
    A mapfile containing node->datafile pairs

    """
    inputs = {
        'ndppp': ingredient.ExecField(
            '--ndppp',
            help="The full path to the ndppp executable"
        ),
        'initscript': ingredient.FileField(
            '--initscript',
            help='''The full path to an (Bourne) shell script which will\
             intialise the environment (ie, ``lofarinit.sh``)'''
        ),
        'parset': ingredient.FileField(
            '-p', '--parset',
            help="The full path to a PreparePhase configuration parset."
        ),
        'working_directory': ingredient.StringField(
            '-w', '--working-directory',
            help="Working directory used on output nodes. Results location"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            default=".prepare_imager",
            help="Added to the input filename to generate the output filename"
        ),
        'output_mapfile': ingredient.FileField(
            '--output-mapfile',
            help="Added to the input filename to generate the output filename"
        ),
        'slices_per_image': ingredient.IntField(
            '--slices-per-image',
            help="The number of (time) slices for each output image"
        ),
        'subbands_per_image': ingredient.IntField(
            '--subbands-per-image',
            help="The number of subbands to be collected in each output image"
        )                
    } 
                  
    def go(self): 
        """
        Main function for recipe: Called by the pipeline framework
        """
        self.logger.info("Starting prepare_imager run")     
        super(prepare_imager, self).go()
              
        # *********************************************************************
        # Load inputs, validate
        # *********************************************************************        
        # load mapfiles
        input_map, output_map = self._load_input_output_map()
       
        # Environment variables
        slices_per_image = self.inputs['slices_per_image']
        subbands_per_image = self.inputs['subbands_per_image']
        init_script = self.inputs['initscript']
        parset = self.inputs['parset']

        # Validate inputs:
        if len(input_map) != len(output_map) * \
                                   (slices_per_image * subbands_per_image):
            self.logger.warn("ERROR: incorrect number of input ms for "+
                              "supplied parameters")
            return 1
        
        # *********************************************************************            
        # construct command and create variables for running on remote nodes
        # TODO: candidate for refactoring
        # *********************************************************************
        # Compile the command to be executed on the remote machine, fi
        nodeCommand = "python %s" % (self.__file__.replace("master", "nodes")) 

        outnames = collections.defaultdict(list)
        jobs = []
        n_subband_groups = len(output_map)
        for idx_sb_group, (host, output_measurement_set) in enumerate(output_map):
            #construct and save the output name
            input_map_for_subband = self._create_input_map_for_subband(
                                slices_per_image, n_subband_groups, 
                                subbands_per_image, idx_sb_group, input_map)
                     
            outnames[host].append(output_measurement_set)
            
            
            arguments=[init_script, parset, os.path.join(
                            self.inputs['working_directory'],
                            self.inputs['job_name']),
                        self.inputs['ndppp'], 
                        output_measurement_set, slices_per_image, 
                        subbands_per_image, repr(input_map_for_subband)]
            
            # TODO: The size of input_for_image could surpass the command line
            # length: Use rar instead?
            jobs.append(ComputeJob(host, nodeCommand, arguments))
        
        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)
                  
        # *********************************************************************
        # validate performace and cleanup
        # *********************************************************************
        #TODO: Output validation!!!
        
        # Test for errors
        if self.error.isSet():
            self.logger.warn("Failed prepare_imager run detected")
            return 1
        else:
            return 0


    def _create_input_map_for_subband(self, slices_per_image, n_subband_groups,
                        subbands_per_image, idx_sb_group, input_mapfile):
        """
        _create_input_map_for_subband() Creates an input mapfile representation:
        This is a subset of the complete input_mapfile based on the subband 
        details suplied: The input_mapfile is structured: First all subbands for
        a complete timeslice and the the next timeslice. The result value 
        contains all the information needed for a single subband
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
        
        return inputs_for_image  

    
    def _load_input_output_map(self):
        """
        Load datafiles containing node --> dataset pairs
        The input mapfile is structured each line contains:
        node name , measurement set
        
        the input mapfile measurement sets are sorted first timeslice and the 
        sub sorted on subbands eg:   slice1_SB001
                                     slice1_SB002
                                     slice2_SB001
                                     etc
        """  
        self.logger.debug("Loading input map: {0}".format(self.inputs['args']))
        input_map = eval(open(self.inputs['args'][0]).read())

        # The output locus also determines the computation node
        self.logger.debug("Loading output map:{0}".format(
                                                self.inputs['output_mapfile']))
        output_map = eval(open(self.inputs['output_mapfile']).read())
        return input_map, output_map
                    

if __name__ == "__main__":
    sys.exit(prepare_imager().main())
