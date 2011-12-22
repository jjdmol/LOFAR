# Prepare fase of the LOFAR AWIMAGER RECIPE
# collect datasets and start prepropressing fases:
# dppp and concat of timeslices
# Wouter Klijn 2012
# klijn@astron.nl
# ------------------------------------------------------------------------------

from __future__ import with_statement
import sys
import errno
import os.path

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarnode import  LOFARnodeTCP
from subprocess import CalledProcessError
import pyrap.tables as pt #@UnresolvedImport
 
class prepare_imager(LOFARnodeTCP):
    def run(self, init_script, parset, working_dir, ndppp, measurement_set,
            slices_per_image, subbands_per_image, input_map_path):
        #working_dir = tempfile.mkdtemp()
        log4CPlusName = "prepare_imager_node_recipe" 
        group_dir = "group_sets"
        sets_dir = "working_sets"
        
        # Time execution of this job 
        # TODO: The datacolumn is depending on output/functioning of previous
        # pipelines. When update ms then use corrected data
        
        with log_time(self.logger):          
            # Load the input file names
            self.logger.info("Loading input map from {0}".format(
                os.path.join(working_dir, "node_input.map")))
            input_map = eval(open(input_map_path).read())
            
            # *********************************************************
            # Collect all the measurement sets in a single directory
            # ******************************************************
            working_set_path = os.path.join(working_dir, sets_dir) 
            self._create_dir(working_set_path)
            missing_files = []
            
            #loop all measurement sets
            for idx, (node, path) in enumerate(input_map):
                self.logger.info("copy file: {0}".format(path))

                #construct copy command
                copy_command = "rsync -r {0}:{1} {2} ".format(node, path, 
                                                            working_set_path)
                exit_status = os.system(copy_command)

                #if copy failed log the missing file
                if  exit_status != 0: 
                    missing_files.append(path.split('/')[-1])
                    self.logger.info("Failed loading file: {0}".format(path))
                    self.logger.info("continuing")
            
            # ***************************************************
            # start NDPPP:  
            # for each timeslice 
            # 1. Remove international stations 
            # 2. Combine subbands into groups
            # ******************************************************           
            #create the directory to save subband group data sets
            group_measurement_directory = os.path.join(working_dir, group_dir)
            group_measurements_collected = []
            self._create_dir(group_measurement_directory)
                
            for idx_time_slice in range(slices_per_image):
                #collect the subband for this timeslice in a single list
                input_map_subgroup = \
                   input_map[(idx_time_slice * subbands_per_image): \
                             ((idx_time_slice + 1) * subbands_per_image)]
                   
                #get the path on the remote machine. get the directory names
                input_subgroups = map(lambda x: x.split("/")[-1] ,
                                          list(zip(*input_map_subgroup)[1]))

                #Remove the missing files
                input_subgroups = filter(lambda x: not(x in missing_files),
                                      input_subgroups)
               
                #join with the group_measurement_directory
                # TODO: Fixme the write of the ms is a layer to deep!!
                # Remove addition x: due to incorrect path structure of source files!!!
                input_path = map(lambda x: os.path.join(working_dir,
                             sets_dir, x, x), input_subgroups)
                
                group_ms_name = "time_slice_{0}.dppp.ms".format(idx_time_slice)
                group_measurements_collected.append(
                        os.path.join(working_dir, group_dir, group_ms_name))
                
                # create the directory to save group measurement set for this 
                # group
                group_ms_path = os.path.join(group_measurement_directory,
                                             group_ms_name)
                
                # Update the parset with calculated parameters
                patchDictionary = {'uselogger': 'True', # enables log4cplus
                                   'msin': repr(input_path),  
                                   'msout':group_ms_path
                                   }               
                temp_parset_filename = None                     
                try:
                    temp_parset_filename = patch_parset(parset, patchDictionary)
                except Exception, e:
                    self.logger.error("failed loading and updating the " +
                                          "parset: {0}".format(parset))

                #run ndppp
                cmd = [ndppp, temp_parset_filename]              
                try:
                    environment = read_initscript(self.logger, init_script)               
                    with CatchLog4CPlus(working_dir, self.logger.name + 
                                        "." + os.path.basename(log4CPlusName),
                                        os.path.basename(ndppp)) as logger:                      
                            catch_segfaults(cmd, working_dir, environment,
                                            logger, cleanup=None)
                except CalledProcessError, e:
                    self.logger.error(str(e))
                    return 1
                except Exception, e:
                    self.logger.error(str(e))
                    return 1
                finally:
                    os.unlink(temp_parset_filename)
                     

            # *************************************************************
            # run pyrap to combine the time slices in a single ms
            # *************************************************************           
            tn = pt.table(group_measurements_collected)            
            tn.rename(os.path.join(working_dir, "output_concat.ms")) 
            tn.close()
 
# ***************************************************
# TODO: Currently the concat does not work wait for fix ger
# ***************************************************
#            pt.msconcat(group_measurements_collected, 
#                        os.path.join(working_dir, "output"), concatTime=True)
#            self.logger.info("pyrap test !!1")

                
        # TODO: clean up of temporary files??
        # TODO: FAILURE TO END?? WHAT TO DO: timeout 
        return 0
    
    def _create_dir(self,path):
        try:
            os.mkdir(path)
        except OSError as exc:
            if exc.errno == errno.EEXIST: #if dir exists continue  
                pass
            else:
                raise

if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(prepare_imager(jobid, jobhost, jobport).run_with_stored_arguments())
