#                                                         LOFAR IMAGING PIPELINE
#
#                                                           selfcal_finalize
#                                                            Wouter Klijn 2012
#                                                           klijn@astron.nl
# ------------------------------------------------------------------------------
from __future__ import with_statement
import sys
import subprocess
import os
import tempfile
import shutil

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import log_time, create_directory
import lofar.addImagingInfo as addimg
import pyrap.images as pim
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.data_map import DataMap
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.subprocessgroup import SubProcessGroup

import urllib2
import lofarpipe.recipes.helpers.MultipartPostHandler as mph

class selfcal_finalize(LOFARnodeTCP):
    """
    This script performs the folowing functions:
    
    1. Add the image info to the casa image:
       addimg.addImagingInfo (imageName, msNames, sourcedbName, minbl, maxbl)
    2. Convert the image to hdf5 and fits image
    3. Filling of the HDF5 root group
    4. Move meta data of selfcal to correct dir in ms
    5. Deepcopy ms to output location
    """
    def run(self, awimager_output, raw_ms_per_image, sourcelist, target,
            output_image, minbaseline, maxbaseline, processed_ms_dir,
            fillrootimagegroup_exec, environment, sourcedb, concat_ms, 
            correlated_output_location, msselect_executable):
        self.environment.update(environment)
        """
        :param awimager_output: Path to the casa image produced by awimager 
        :param raw_ms_per_image: The X (90) measurements set scheduled to 
            create the image
        :param sourcelist: list of sources found in the image 
        :param target: <unused>
        :param minbaseline: Minimum baseline used for the image 
        :param maxbaseline: largest/maximum baseline used for the image
        :param processed_ms_dir: The X (90) measurements set actually used to 
            create the image
        :param fillrootimagegroup_exec: Executable used to add image data to
            the hdf5 image  
                 
        :rtype: self.outputs['hdf5'] set to "succes" to signal node succes
        :rtype: self.outputs['image'] path to the produced hdf5 image
        """
        with log_time(self.logger):
            raw_ms_per_image_map = DataMap.load(raw_ms_per_image)

            # *****************************************************************
            # 1. add image info                      
            # Get all the files in the processed measurement dir
            file_list = os.listdir(processed_ms_dir)
            # TODO: BUG!! the meta data might contain files that were copied
            # but failed in imager_bbs 
            processed_ms_paths = []
            for item in raw_ms_per_image_map:
                path = item.file
                raw_ms_file_name = os.path.split(path)[1]
                #if the raw ms is in the processed dir (additional check)
                if (raw_ms_file_name in file_list):
                    # save the path
                    processed_ms_paths.append(os.path.join(processed_ms_dir,
                                                            raw_ms_file_name))
            #add the information the image
            try:
                addimg.addImagingInfo(awimager_output, processed_ms_paths,
                    sourcedb, minbaseline, maxbaseline)

            except Exception, error:
                self.logger.error("addImagingInfo Threw Exception:")
                self.logger.error(error)
                # Catch raising of already done error: allows for rerunning
                # of the recipe
                if "addImagingInfo already done" in str(error):
                    pass
                else:
                    raise Exception(error) 
                #The majority of the tables is updated correctly

            # ***************************************************************
            # 2. convert to hdf5 image format
            output_directory = None
            pim_image = pim.image(awimager_output)
            try:
                self.logger.info("Saving image in HDF5 Format to: {0}" .format(
                                output_image))
                # Create the output directory
                output_directory = os.path.dirname(output_image)
                create_directory(output_directory)
                # save the image
                pim_image.saveas(output_image, hdf5=True)

            except Exception, error:
                self.logger.error(
                    "Exception raised inside pyrap.images: {0}".format(
                                                                str(error)))
                raise error

            # Convert to fits
            # create target location
            fits_output = output_image + ".fits"
            # To allow reruns a possible earlier version needs to be removed!
            # image2fits fails if not done!!
            if os.path.exists(fits_output):
                os.unlink(fits_output)

            try:
                temp_dir = tempfile.mkdtemp()
                with CatchLog4CPlus(temp_dir,
                    self.logger.name + '.' + os.path.basename(awimager_output),
                            "image2fits") as logger:
                    catch_segfaults(["image2fits", '-in', awimager_output,
                                                 '-out', fits_output],
                                    temp_dir, self.environment, logger)
            except Exception, excp:
                self.logger.error(str(excp))
                return 1
            finally:
                shutil.rmtree(temp_dir)

            # ****************************************************************
            # 3. Filling of the HDF5 root group
            command = [fillrootimagegroup_exec, output_image]
            self.logger.info(" ".join(command))
            #Spawn a subprocess and connect the pipes
            proc = subprocess.Popen(
                        command,
                        stdin=subprocess.PIPE,
                        stdout=subprocess.PIPE,
                        stderr=subprocess.PIPE)

            (stdoutdata, stderrdata) = proc.communicate()

            exit_status = proc.returncode


            #if copy failed log the missing file
            if  exit_status != 0:
                self.logger.error("Error using the fillRootImageGroup command"
                    ". Exit status: {0}".format(exit_status))
                self.logger.error(stdoutdata)
                self.logger.error(stderrdata)

                return 1

            # *****************************************************************
            # 4. Move the meta information to the correct directory in the 
            # output mesurements set
            self.logger.info("Save-ing selfcal parameters to file:")
            meta_dir =  concat_ms + "_selfcal_information"
            meta_dir_target =  os.path.join(concat_ms, "selfcal_information")
            if os.path.exists(meta_dir) and os.path.exists(concat_ms):
                self.logger.info("Copy meta information to output measurementset")

                # Clear possible old data, allows for rerun of the pipeline
                # if needed.
                if os.path.exists(meta_dir_target):
                      shutil.rmtree(meta_dir_target)
                shutil.copytree(meta_dir, meta_dir_target)
                
            # *****************************************************************
            # 4 Copy the measurement set to the output directory
            # use msselect to copy all the data in the measurement sets
            
            cmd_string = "{0} in={1} out={2} baseline=* deep=True".format(
                   msselect_executable, concat_ms, correlated_output_location)
            msselect_proc_group = SubProcessGroup(self.logger)
            msselect_proc_group.run(cmd_string)
            if msselect_proc_group.wait_for_finish() != None:
                self.logger.error("failed copy of measurmentset to output dir")
                raise Exception("an MSselect run failed!")

            self.outputs["hdf5"] = "succes"
            self.outputs["image"] = output_image
            self.outputs["correlated"] = correlated_output_location


        
        return 0


if __name__ == "__main__":

    _JOBID, _JOBHOST, _JOBPORT = sys.argv[1:4]
    sys.exit(selfcal_finalize(_JOBID, _JOBHOST,
                             _JOBPORT).run_with_stored_arguments())
