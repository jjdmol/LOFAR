# LOFAR IMAGING PIPELINE
#
# Prepare phase of the imager pipeline: node (also see master recipe)
#
# 1 collect measurementsets: copy to current node
# 2. Start dppp remove missing dataset 
# 3. concat the measurement sets  
#
# Wouter Klijn 
# 2012
# klijn@astron.nl
        # TODO: The data column is depending on output/functioning of previous
        # pipelines. When update ms then use corrected data
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
    def run(self, init_script, parset, working_dir, ndppp, output_measurement_set,
            slices_per_image, subbands_per_image, input_map_repr):
  
        log4CPlusName = "prepare_imager_node" 
        group_dir = "group_sets"
        collected_ms_dir = "collected_ms"    
        self._create_dir(working_dir)  #TODO: Moet mischien nog worden verwijderd

        with log_time(self.logger):          
            # Load the input file names
            self.logger.info("Loading input map from {0}".format(
                os.path.join(working_dir, "node_input.map")))
            
            input_map = eval(input_map_repr)
##            self.logger.info(input_map)
#      
#           missing_files = self._copy_input_files(working_dir, collected_ms_dir,
#                                                   input_map) 
      
            # ********************* temp solution for quick debugging
            missing_files = []  
            skip_copy = False
            temp_missing = os.path.join(working_dir, "temp_missing" )
            if not skip_copy:
                #Collect all files and copy to current node
                missing_files = self._copy_input_files(working_dir, collected_ms_dir,
                                                   input_map)                 
                                   
                #Temp solution for the missing files
                temp_missing = os.path.join(working_dir, "temp_missing" )
                fp = open(temp_missing, 'w')
                fp.write(repr(missing_files))
            else:
                missing_files = eval(open(temp_missing).read())
            # ********************* temp solution for quick debugging
            
            if len(missing_files):
                self.logger.info(repr(missing_files))

            #run dppp
            group_measurements_collected = \
                self._run_dppp( working_dir, group_dir, slices_per_image,
                    input_map, subbands_per_image, missing_files, collected_ms_dir, 
                    parset, ndppp, init_script, log4CPlusName)
            
            # Perform the concat of the timeslices
            self._concat_timeslices(group_measurements_collected, 
                                    output_measurement_set)       

        # TODO: clean up of temporary files??
        # The subgroup ms cannot be removed: they are collected in the final 
        # concat set. The raw data is copied and good candidate
        self.outputs["completed"] = "true"      
        return 0
    
    def _create_dir(self,path):
        """
        Create the directory suplied in path. Continues if directory exist
        Creates parent directory of this does not exsist
        throw original exception in other error cases
        """
        try:
            os.mkdir(path)
        except OSError as exc:
            if exc.errno == errno.EEXIST:    #if dir exists continue 
                pass
            elif exc.errno == errno.ENOENT:  #if parent dir does not excist
                parent_path = os.path.split(path)[0]
                parent_parent_path = os.path.split(parent_path)[0]
                #check if parent of parent dir exsist
                if os.path.exists(parent_parent_path):
                    os.mkdir(parent_path)
                    os.mkdir(path)
                else: 
                    raise                
        except:  
            raise

    def _copy_input_files(self, working_dir, sets_dir, input_map):
        """
        Collect all the measurement sets in a single directory:
        The measurement sets are located on different nodes on the cluster.
        This function collects all the file in the input map in the sets_dir
        Return value is a set of missing files
        """
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
        
        return set(missing_files)

    def _run_dppp(self, working_dir, group_dir, slices_per_image, input_map,
                  subbands_per_image, missing_files, sets_dir, parset, ndppp, 
                  init_script, log4CPlusName):
        """
        Run NDPPP:  
        Create dir for grouped measurements, assure clean workspace
        Create input map for ndppp: Remove missing files
        CAll ndppp in save mode. Actual parameters are located in 
        temp_parset_filename
        """           
        #create the directory to save subband group data sets
        group_measurement_directory = os.path.join(working_dir, group_dir)
        group_measurements_collected = []
        self._create_dir(group_measurement_directory)
        

        # assure empty dir
        
        os.system("rm -rf {0}/*".format(group_measurement_directory))
                
        for idx_time_slice in range(slices_per_image):
            #collect the subband for this timeslice in a single list
            input_map_subgroup = \
                input_map[(idx_time_slice * subbands_per_image): \
                             ((idx_time_slice + 1) * subbands_per_image)]
                   
            #get the path on the remote machine. get the directory names
            input_subgroups = map(lambda x: x.split("/")[-1] ,
                                          list(zip(*input_map_subgroup)[1]))

            #Remove the missing files
            #input_subgroups = filter(lambda x: not(x in missing_files),
            #                          input_subgroups)
               
            #join with the group_measurement_directory
            # TODO: Fixme the write of the ms is a layer to deep!! The current
            # input location is incorrect after fix of the input file fix this 
            # Remove addition x: due to incorrect path structure of source files
        
           
            ndppp_input_ms = map(lambda x: os.path.join(working_dir,
                         sets_dir, x, x), input_subgroups)
                
            output_ms_name = "time_slice_{0}.dppp.ms".format(idx_time_slice)
            group_measurements_collected.append(
                os.path.join(working_dir, group_dir, output_ms_name))

            # create the directory to save group measurement set for this 
            # group
            group_ms_path = os.path.join(group_measurement_directory,
                                         output_ms_name)
            
            # Update the parset with calculated parameters
            patchDictionary = {'uselogger': 'True', # enables log4cplus
                               'msin': repr(ndppp_input_ms),  
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
                return 1
            except Exception, e:
                return 1
            finally:
                os.unlink(temp_parset_filename)
            
        return group_measurements_collected

    def _concat_timeslices(self, group_measurements_collected, 
                                    output_file_path):
        """
        run pyrap to combine the time slices in a single ms:
        Virtual copy, a new ms with symbolic links to actual data is created!
        # *************************************************************
        # TODO: Currently the concat does not work wait for fix ger
        # The next line should be the target copde

                   self.logger.info("pyrap test !!1")
        # Temp Fix:                      
        """         
#        self.logger.info("***************1234****************")
#        self.logger.info(group_measurements_collected)
#        self.logger.info("debug0: {0}".format(output_file_path))

#        pt.msconcat(group_measurements_collected, 
#                                output_file_path, concatTime=True)
        tn = pt.table(group_measurements_collected)            
        tn.rename(output_file_path) 
        tn.close()    
        
                 

if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(
        prepare_imager(jobid, jobhost, jobport).run_with_stored_arguments())
