#                                                         LOFAR IMAGING PIPELINE
#
#                                    Example recipe with simple job distribution
#                                                            John Swinbank, 2010
#                                                      swinbank@transientskp.org
# ------------------------------------------------------------------------------

import os
import sys
import collections
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.baserecipe import BaseRecipe
from lofarpipe.support.remotecommand import RemoteCommandRecipeMixIn
from lofarpipe.support.remotecommand import ComputeJob
from lofarpipe.support.group_data import load_data_map

class prepare_imager(BaseRecipe, RemoteCommandRecipeMixIn):
    """
    Run the AWImager on the nodes and the data files suplied in the mapfile
    **Arguments**
    A mapfile containing node->datafile pairs

    """
    inputs = {
# hier zullen wel executables voor de dppp voor in de plaats moeten komen
#        'executable': ingredient.ExecField(
#            '--executable',
#            help="The full path to the relevant AWImager executable"
#        ),
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
            help="Working directory used on outpuconfigt nodes. Results location"
        ),
        'suffix': ingredient.StringField(
            '--suffix',
            default=".prepare_imager",
            help="Added to the input filename to generate the output filename"
        ),
        'output_mapfile': ingredient.FileField(
            '--output-mapfile',
            help="Added to the input filename to generate the output filename"
        )    
    } 
                  
    def go(self): 
        self.logger.info("Starting prepare_imager run")     
        super(prepare_imager, self).go()
#command line arguments
#python prepare_imager.py ~/build/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/input.map --job-name "prepare_mager" --config ~/build/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/pipeline.cfg --initscript /opt/cep/LofIm/daily/lofar/lofarinit.sh -p ~/build/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/master/parset.test -w "/data/scratch/klijn" --output-mapfile ~/build/gnu_debug/installed/lib/python2.6/dist-packages/lofarpipe/recipes/output.map
         
   
#        # Load datafiles containing node --> dataset pairs     
        self.logger.debug("Loading input map from {0}".format(self.inputs['args']))
        input_mapfile = eval(open(self.inputs['args'][0]).read())

        #output mapping locus -> file
        self.logger.debug("Loading output map from {0}".format(self.inputs['output_mapfile']))
        output_mapfile = eval(open(self.inputs['output_mapfile']).read())
 
        

        
        # Compile the command to be executed on the remote machine
        nodeCommand = "python %s" % (self.__file__.replace("master", "nodes"))
        
        # Environment type variables
        slices_per_image = 9 # inputs['slices_per_image']
        subbands_per_image = 10 # inputs['subbands_per_image']          
        initScript = self.inputs['initscript']
        parset = self.inputs['parset']                  #Using parameter set             
        workingDir = self.inputs['working_directory']     

        #todo:
        # 1. copy the files to the correct location
        #   a. Create a submapset and copy to the node runtime location
        # 
        
        

        
        # Output
        #outnames = collections.defaultdict(list)             

        print "we have a winner!!!!"
#        # Create the jobs
#        jobs = []
#        for host, measurementSet in input_mapfile:
#            #construct and save the output name
#            outnames[host].append(os.path.join(
#                            self.inputs['working_directory'],
#                            self.inputs['job_name'],
#                            os.path.basename(measurementSet.rstrip('/')) + 
#                            self.inputs['suffix']))
#            
#            arguments=[executable, initScript, parset, workingDir, \
#                       measurementSet]
#            
#            jobs.append(ComputeJob(host, nodeCommand, arguments))
#        
#        # Hand over the job(s) to the pipeline scheduler
#        self._schedule_jobs(jobs)
        
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
