#                                                         LOFAR IMAGING PIPELINE
#
#                                                           imager_finalize
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
from lofarpipe.support.group_data import load_data_map
from lofarpipe.support.pipelinelogging import CatchLog4CPlus

import urllib2
import lofarpipe.recipes.helpers.MultipartPostHandler as mph

class imager_finalize(LOFARnodeTCP):
    """
    This script performs the folowing functions:
    
    1. Add the image info to the casa image:
       addimg.addImagingInfo (imageName, msNames, sourcedbName, minbl, maxbl)
    2. Convert the image to hdf5 and fits image
    3. Filling of the HDF5 root group
    4. Export fits image to msss image server
    4. Return the outputs
    """
    def run(self, awimager_output, raw_ms_per_image, sourcelist, target,
            output_image, minbaseline, maxbaseline, processed_ms_dir,
            fillrootimagegroup_exec, environment):
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
            raw_ms_per_image_map = load_data_map(raw_ms_per_image)

            # *****************************************************************
            # 1. add image info                      
            # Get all the files in the processed measurement dir
            file_list = os.listdir(processed_ms_dir)
            # TODO: BUG!! the meta data might contain files that were copied
            # but failed in imager_bbs 
            processed_ms_paths = []
            for (node, path) in raw_ms_per_image_map:
                raw_ms_file_name = os.path.split(path)[1]
                #if the raw ms is in the processed dir (additional check)
                if (raw_ms_file_name in file_list):
                    # save the path
                    processed_ms_paths.append(os.path.join(processed_ms_dir,
                                                            raw_ms_file_name))
            #add the information the image
            try:
                addimg.addImagingInfo(awimager_output, processed_ms_paths,
                    sourcelist, minbaseline, maxbaseline)

            except Exception, error:
                self.logger.error("addImagingInfo Threw Exception:")
                self.logger.error(error)
                # Catch raising of already done error: allows for rerunning
                # of the recipe
                if "addImagingInfo already done" in str(error):
                    pass
                else:
                    raise Exception(error) #Exception: Key Name unknown 
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
            self.logger.error(stdoutdata)
            self.logger.error(stderrdata)

            #if copy failed log the missing file
            if  exit_status != 0:
                self.logger.error("Error using the fillRootImageGroup command"
                    "see above lines. Exit status: {0}".format(exit_status))

                return 1

            # *****************************************************************
            # 4 Export the fits image to the msss server
            url = "http://tanelorn.astron.nl:8000/upload"
            try:
                self.logger.info("Starting upload of fits image data to server!")
                opener = urllib2.build_opener(mph.MultipartPostHandler)
                filedata = {"file": open(fits_output, "rb")}
                opener.open(url, filedata, timeout=2)
                # HTTPError needs to be caught first.
            except urllib2.HTTPError as httpe:
                self.logger.warn("HTTP status is: {0}".format(httpe.code))
                self.logger.warn("failed exporting fits image to server")

            except urllib2.URLError as urle:
                self.logger.warn(str(urle.reason))
                self.logger.warn("failed exporting fits image to server")

            except Exception, exc:
                self.logger.warn(str(exc))
                self.logger.warn("failed exporting fits image to server")


            # *****************************************************************
            # 5. export the sourcelist to the msss server
            url = "http://tanelorn.astron.nl:8000/upload_srcs"
            try:
                self.logger.info("Starting upload of sourcelist data to server!")
                opener = urllib2.build_opener(mph.MultipartPostHandler)
                filedata = {"file": open(sourcelist, "rb")}
                opener.open(url, filedata, timeout=2)
                # HTTPError needs to be caught first.
            except urllib2.HTTPError as httpe:
                self.logger.warn("HTTP status is: {0}".format(httpe.code))
                self.logger.warn("failed exporting sourcelist to server")

            except urllib2.URLError as urle:
                self.logger.warn(str(urle.reason))
                self.logger.warn("failed exporting sourcelist image to server")

            except Exception, exc:
                self.logger.warn(str(exc))
                self.logger.warn("failed exporting sourcelist image to serve")


            self.outputs["hdf5"] = "succes"
            self.outputs["image"] = output_image

        return 0


if __name__ == "__main__":

    _JOBID, _JOBHOST, _JOBPORT = sys.argv[1:4]
    sys.exit(imager_finalize(_JOBID, _JOBHOST,
                             _JOBPORT).run_with_stored_arguments())
