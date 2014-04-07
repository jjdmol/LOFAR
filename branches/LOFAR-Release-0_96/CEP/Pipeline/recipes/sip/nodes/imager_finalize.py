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

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import log_time, create_directory
import lofar.addImagingInfo as addimg #@UnresolvedImport
import pyrap.images as pim #@UnresolvedImport
from lofarpipe.support.group_data import load_data_map

class imager_finalize(LOFARnodeTCP):
    """
    This script performs the folowing functions
    1. Add the image info to the casa image:
    addimg.addImagingInfo (imageName, msNames, sourcedbName, minbl, maxbl)
        imageName is the final image created
        msNames is the original set of MSs from which the image is created (thus 90 MSs)
        sourcedbName is the SourceDB containing the found sources
        minbl and maxbl are the minimum and maximum baselines length used in m (thus 0 en 10000)
    2. Convert the image to hdf5 image format:
    3. Filling of the HDF5 root group
    """
    def run(self, awimager_output, raw_ms_per_image, sourcelist, target,
            output_image, minbaseline, maxbaseline, processed_ms_dir,
            fillRootImageGroup_exec):
        with log_time(self.logger):
            raw_ms_per_image_map = load_data_map(raw_ms_per_image)
            self.logger.info(repr(raw_ms_per_image_map))
            # 1. add image info           
            processed_ms_paths = []
            # Get all the files in the processed measurement dir
            file_list = os.listdir(processed_ms_dir)


            for (node, path) in raw_ms_per_image_map:
                raw_ms_file_name = os.path.split(path)[1]
                #if the raw ms is in the processed dir (additional check)
                if (raw_ms_file_name in file_list):
                    # save the path
                    processed_ms_paths.append(os.path.join(processed_ms_dir,
                                                            raw_ms_file_name))
            #add the information the image
            try:
                addimg.addImagingInfo (awimager_output, processed_ms_paths,
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

            # 2. convert to hdf5 image format
            im = pim.image(awimager_output) #im = pim.image("image.restored")

            try:
                self.logger.info("Saving image in HDF5 Format to: {0}" .format(
                                output_image))
                # Create the output directory
                create_directory(os.path.split(output_image)[0])
                # save the image
                im.saveas(output_image, hdf5 = True)
                # TODO: HDF5 version of PIM is different to the system version
                # dunno the solution: the script breaks.
            except Exception, error:
                self.logger.error(
                    "Exception raised inside pyrap.images: {0}".format(str(error)))
                raise Exception(str(error))


            # 3. Filling of the HDF5 root group
            command = [fillRootImageGroup_exec, output_image]
            self.logger.info(" ".join(command))
            #Spawn a subprocess and connect the pipes
            proc = subprocess.Popen(
                        command,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)

            (stdoutdata, stderrdata) = proc.communicate()

            exit_status = proc.returncode
            self.logger.error(stdoutdata)
            self.logger.error(stderrdata)

            #if copy failed log the missing file
            if  exit_status != 0:
                self.logger.error("Error using the fillRootImageGroup command"
                    "see above lines. Exit status: {0}".format(exit_status))

                return 1

        return 0


if __name__ == "__main__":

    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_finalize(jobid, jobhost, jobport).run_with_stored_arguments())
