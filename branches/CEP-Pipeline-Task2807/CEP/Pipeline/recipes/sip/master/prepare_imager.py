# LOFAR IMAGING PIPELINE
#
# Prepare phase of the imager pipeline
#     collect data from nodes and collect it on the compute node
#     start bbs, concats the files and 
#     
# Wouter Klijn 
# 2011
# klijn@astron.nl
# ------------------------------------------------------------------------------

import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map, store_data_map

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
        self.logger.info("Starting prepare_imager run")     
        super(prepare_imager, self).go()
#command line arguments:
#python python prepare_imager.py ~/build/preparation/input.map --job prepare_imager --config ~/build/preparation/pipeline.cfg --initscript /opt/cep/LofIm/daily/lofar/lofarinit.sh --output-mapfile ~/build/preparation/output.map --parset ~/build/preparation/parset.par --working-directory "/data/scratch/klijn" --subbands-per-image 10 --slices-per-image 9 --ndppp /opt/cep/LofIm/daily/lofar/bin/NDPPP -v

        # *********************************************************************
        # Load datafiles containing node --> dataset pairs
        # The input mapfile is structured each line contains:
        # node name , measurement set
        #    the measurement sets are sorted first timeslice and the sub sorted
        #    on subbands eg:   slice1_SB001
        #                      slice1_SB002
        #                      slice2_SB001
        #                      etc
        # *********************************************************************  
        self.logger.debug("Loading input map from {0}".format(
                                                    self.inputs['args']))
        input_mapfile = eval(open(self.inputs['args'][0]).read())

        # output mapping locus -> file
        # The output locus also determines the computation node
        self.logger.debug("Loading output map from {0}".format(
                                                self.inputs['output_map']))
        output_map = eval(open(self.inputs['output_map']).read())
 
        # Compile the command to be executed on the remote machine
        nodeCommand = "python %s" % (self.__file__.replace("master", "nodes"))
        
        # Environment variables
        slices_per_image = self.inputs['slices_per_image']
        subbands_per_image = self.inputs['subbands_per_image']          
        init_script = self.inputs['initscript']
        parset = self.inputs['parset']                  #Using parameter set             
        working_dir = self.inputs['working_directory']     

        # Validate inputs:
        if len(input_mapfile) != len(output_map) * \
                                   (slices_per_image * subbands_per_image):
            self.logger.warn("ERROR: incorrect number of input ms for "+
                              "supplied parameters")
            return 1
                       
        # ********************************************************************    
        # For each output image (is a subband group) create a input mapfile and
        # send it to the compute node
        # ********************************************************************
#        n_subband_groups = len(output_map)
#        for  idx_sb_group, (node, path) in enumerate(output_map):
#
#            
#            inputs_for_image = []
#            # collect the inputs: first step over the time slices
#            for idx_slice in range(slices_per_image):
#                # calculate the first line for current time slice and subband group
#                line_idx_start = idx_slice * \
#                    (n_subband_groups * subbands_per_image) + \
#                    (idx_sb_group * subbands_per_image)
#                line_idx_end = line_idx_start + subbands_per_image
#                
#                #extend inputs with the files for the current time slice
#                inputs_for_image.extend(input_mapfile[line_idx_start: line_idx_end])  
#            
#            # Create a file, fill with the file names to send to the remote node
#            temp_input_map = os.path.join(self.config.get("layout", "job_directory"),
#                                          "node_input_{0}.map".format(idx_sb_group) )
#            
#            store_data_map(temp_input_map, inputs_for_image)
            
#            fp = open(temp_input_map, "w")  #TODO : working 
#            fp.write(repr(inputs_for_image))    
#            fp.close()
            #input_mapfile = eval(open(temp_input_map).read())
            # Send the file 
            
            
#            print "{0}:{1}".format(node, temp_input_map)          
#            system_command_string =  \
#                            "scp {0} {1}:{0}".format(temp_input_map ,node )
#
#            sys.stdout.flush()

            os.system(system_command_string)  #TODO test corrert write? or local node problem?         
               
        # Output
        #start the nodes 
        outnames = collections.defaultdict(list)             

        # Create the jobs
        jobs = []
        n_subband_groups = len(output_map)
        #for  idx_sb_group, (node, path) in enumerate(output_map):
        for idx_sb_group, (host, measurement_set) in enumerate(output_map):
            #construct and save the output name
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
            
            # Create a file, fill with the file names to send to the remote node
            temp_input_map = os.path.join(self.config.get("layout", "job_directory"),
                                          "node_input_{0}.map".format(idx_sb_group) )
            
            store_data_map(temp_input_map, inputs_for_image)
            outnames[host].append(os.path.join(
                            self.inputs['working_directory'],
                            self.inputs['job_name'],
                            os.path.basename(measurement_set.rstrip('/')) + 
                            self.inputs['suffix']))
            
            arguments=[init_script, parset, os.path.join(
                            self.inputs['working_directory'],
                            self.inputs['job_name']),
                        self.inputs['ndppp'], 
                        measurement_set, slices_per_image, 
                        subbands_per_image, temp_input_map]
            
            jobs.append(ComputeJob(host, nodeCommand, arguments))
        
        # Hand over the job(s) to the pipeline scheduler
        self._schedule_jobs(jobs)
        
        #todo: What to do with the output??
        
        # Test for errors
        if self.error.isSet():
            self.logger.warn("Failed prepare_imager run detected")
            return 1
        else:
            return 0

if __name__ == "__main__":
    print("we have a winner")
    sys.exit(prepare_imager().main())
