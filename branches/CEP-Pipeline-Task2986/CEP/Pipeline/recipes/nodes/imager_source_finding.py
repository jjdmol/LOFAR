from __future__ import with_statement
import sys

from lofar.parameterset import parameterset
from lofarpipe.support.lofarnode import LOFARnodeTCP
import bdsm
import pyrap.images as pim

class imager_source_finding(LOFARnodeTCP):
    def run(self, input_image, bdsm_parameter_run1_path,
            bdsm_parameter_run2x_path, catalog_output_path, image_output_path):
        """
        
        """
        self.logger.info("Starting imager_source_finding")

        # Crop the image to remove artifacts at the border of the the image
        # TODO: This must be implemented such that the requested image size 
        # is resultant in this image!!
        img = pim.image(input_image)
        #subimage(blc=(), trc=(), inc=(), dropdegenerate=True)
        img_cropped = img.subimage(blc = (0, 0, 75, 75), trc = (0, 3, 437, 437), dropdegenerate = False)
        input_image_cropped = input_image + ".cropped"
        img_cropped.saveas(input_image_cropped)



        # default frequency is None (read from image)
        frequency = None
        for idx in range(5):
            # The first iteration uses the input image, second and later use the 
            # output of the previous iteration. The 1+ itteration have a 
            # seperate parameter set. 
            if idx == 0:
                input_image_local = input_image_cropped
                image_output_path_local = image_output_path + "_0"
                bdsm_parameter_local = parameterset(bdsm_parameter_run1_path)
            else:
                input_image_local = image_output_path + "_{0}".format(str(idx - 1))
                image_output_path_local = image_output_path + "_{0}".format(str(idx))
                bdsm_parameter_local = parameterset(bdsm_parameter_run2x_path)

            # parse the parameters and convert to python if possible 
            # this is needed for pybdsm
            bdsm_parameters = {}
            for key in bdsm_parameter_local.keys():
                parameter_value = bdsm_parameter_local.getStringVector(key)[0]
                try:
                    parameter_value = eval(parameter_value)
                except:
                    pass  #do nothing
                bdsm_parameters[key] = parameter_value

            # Do the source finding   
            img = bdsm.process_image(bdsm_parameters,
                        filename = input_image_local, frequency = frequency)

            # If no more matching of sources with gausians is possible (nsrc==0)
            # break the loop
            if img.nsrc == 0:
                break

            #export the catalog and the image with gausians substracted
            img.write_catalog(outfile = catalog_output_path + "_{0}".format(str(idx)),
                              catalog_type = 'srl', clobber = True, format = "ascii")
            img.export_image(outfile = image_output_path_local,
                             img_type = 'gaus_resid', clobber = True,
                             img_format = "fits")

            # Save the frequency from image header of the original input file,
            # This information is not written by pybdsm to the exported image
            frequency = img.cfreq

        return 0



if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_source_finding(jobid, jobhost, jobport).run_with_stored_arguments())

