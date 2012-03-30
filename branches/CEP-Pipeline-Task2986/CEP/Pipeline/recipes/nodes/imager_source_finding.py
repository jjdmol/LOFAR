from __future__ import with_statement
import sys
import os
import subprocess

from lofar.parameterset import parameterset
from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.utilities import create_directory
import bdsm #@UnresolvedImport
import pyrap.images as pim #@UnresolvedImport


class imager_source_finding(LOFARnodeTCP):
    """
    The imager_source_finding 
        
    """
    def run(self, input_image, bdsm_parameter_run1_path,
            bdsm_parameter_run2x_path, catalog_output_path, image_output_path):

        self.logger.info("Starting imager_source_finding")
        # default frequency is None (read from image), save for later cycles
        frequency = None
        number_of_sourcefind_itterations = None
        max_sourcefind_itter = 5  # TODO: Dit moet eigenlijkj controleerbaar zijn van buiten af
        for idx in range(max_sourcefind_itter):
            # The first iteration uses the input image, second and later use the 
            # output of the previous iteration. The 1+ iteration have a 
            # seperate parameter set. 
            if idx == 0:
                input_image_local = input_image # input_image_cropped
                image_output_path_local = image_output_path + "_0"
                bdsm_parameter_local = parameterset(bdsm_parameter_run1_path)
            else:
                input_image_local = image_output_path + "_{0}".format(str(idx - 1))
                image_output_path_local = image_output_path + "_{0}".format(str(idx))
                bdsm_parameter_local = parameterset(bdsm_parameter_run2x_path)

            # parse the parameters and convert to python if possible 
            # this is needed for pybdsm (parset function TODO)
            bdsm_parameters = {}
            for key in bdsm_parameter_local.keys():
                parameter_value = bdsm_parameter_local.getStringVector(key)[0]
                try:
                    parameter_value = eval(parameter_value)
                except:
                    pass  #do nothing
                bdsm_parameters[key] = parameter_value

            img = bdsm.process_image(bdsm_parameters,
                        filename = input_image_local, frequency = frequency)


            # If no more matching of sources with gausians is possible (nsrc==0)
            # break the loop
            if img.nsrc == 0:
                number_of_sourcefind_itterations = idx
                self.logger.info("debug itterations = {0}".format(idx))
                break

            #export the catalog and the image with gausians substracted
            img.write_catalog(outfile = catalog_output_path + "_{0}".format(str(idx)),
                              catalog_type = 'gaul', clobber = True, format = "bbs")
            img.export_image(outfile = image_output_path_local,
                             img_type = 'gaus_resid', clobber = True,
                             img_format = "fits")

            # Save the frequency from image header of the original input file,
            # This information is not written by pybdsm to the exported image
            frequency = img.cfreq

        # if not set the maximum number of itteration us performed
        if number_of_sourcefind_itterations == None:
            number_of_sourcefind_itterations = max_sourcefind_itter

        # The produced catalogs now need to be concatenated into a single list
        # Call with the number of loops and the path to the files
        self._combine_source_lists(number_of_sourcefind_itterations,
                                   catalog_output_path)

        # **********************************************************************
        # TODO: Copy dataproducts to goal location 
        # This should be done in a seperate recipe (or in the frame work)
        # **********************************************************************
        return self._save_output_products(input_image, number_of_sourcefind_itterations)


    def _save_output_products(self, input_image_cropped, number_of_sourcefind_itterations):
        # TODO: Deze f unctie moet echt op een beter plaats (eigen recept?)
        # De extra input die nodig is voor deze step heeft echt geen plaats in 
        # dit recept

        # create a dir to save the files!\
        working_dir = "/data/scratch/klijn/jobs/Pipeline"
        create_directory(os.path.join(working_dir, "major_loop_0"))

        #**********************************************************************
        # Deep copy of the concat.ms
        command = ["msselect",
                   "in={0}".format(os.path.join(working_dir, "concat.ms")),
                    "out={0}".format(os.path.join(working_dir, "major_loop_0", "concat.ms")),
                    "baseline='*'",
                    "deep=true"]
        #Spawn a subprocess and connect the pipes
        copy_process = subprocess.Popen(
                        command,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)

        (stdoutdata, stderrdata) = copy_process.communicate()
        self.logger.info(stdoutdata)

        exit_status = copy_process.returncode
        #if copy failed log the missing file
        if  exit_status != 0:
            self.logger.info("Copying data failed:")
            self.logger.info(stderrdata)

            return 1

        #**********************************************************************
        # Deep copy of the final awimager output
        command = ["cp",
                   "-r",
                   "{0}".format(os.path.join(working_dir, "TestImage.restored.cropped")),
                    "{0}".format(os.path.join(working_dir, "major_loop_0", "TestImage.restored.cropped")),
                    ]
        #Spawn a subprocess and connect the pipes
        copy_process = subprocess.Popen(
                        command,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)

        (stdoutdata, stderrdata) = copy_process.communicate()
        self.logger.info(stdoutdata)

        exit_status = copy_process.returncode
        #if copy failed log the missing file
        if  exit_status != 0:
            self.logger.info("Copying data failed:")
            self.logger.info(stderrdata)
            return 1

        #**********************************************************************
        # intermedate sourcefinder images
        for idx_source_find in range(number_of_sourcefind_itterations):
            command = ["cp",
                   "-r",
                   "{0}".format(os.path.join(working_dir,
                                 "bdsm_output.img_{0}".format(idx_source_find))),
                    "{0}".format(os.path.join(working_dir, "major_loop_0",
                                "bdsm_output.img_{0}".format(idx_source_find))),
                    ]
        #Spawn a subprocess and connect the pipes
            copy_process = subprocess.Popen(
                        command,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)

            (stdoutdata, stderrdata) = copy_process.communicate()
            self.logger.info(stdoutdata)

            exit_status = copy_process.returncode
        #if copy failed log the missing file
            if  exit_status != 0:
                self.logger.info("Copying data failed:")
                self.logger.info(stderrdata)
                return 1

        #**********************************************************************
        command = ["cp",
                   "-r",
                   "{0}".format(os.path.join(working_dir, "bdsm_output_cat")),
                    "{0}".format(os.path.join(working_dir, "major_loop_0", "bdsm_output_cat")),
                    ]
        #Spawn a subprocess and connect the pipes
        copy_process = subprocess.Popen(
                        command,
                        stdin = subprocess.PIPE,
                        stdout = subprocess.PIPE,
                        stderr = subprocess.PIPE)

        (stdoutdata, stderrdata) = copy_process.communicate()
        self.logger.info(stdoutdata)

        exit_status = copy_process.returncode
        #if copy failed log the missing file
        if  exit_status != 0:
            self.logger.info("Copying data failed:")
            self.logger.info(stderrdata)

            return 1



    def _combine_source_lists(self, number_of_sourcefind_itterations,
                              catalog_output_path):
        """
        quick function parsing and concatenating the produces sourcelists:
        parse the files:
        1. get the format line
        2. skip whiteline
        3. collect all sources as strings
        
        save
        1. The format line (only a single formatter is need, same for each file)
        3. add the sources
        4. finish with an endl
        """
        source_list_lines = []

        format_line = None
        self.logger.info("*"*30)
        self.logger.info(number_of_sourcefind_itterations)
        for idx_source_file in range(number_of_sourcefind_itterations):
            fp = open(catalog_output_path + "_{0}".format(idx_source_file))
            #**************************************************
            # Read the format line and save
            format_line = fp.readline()

            #read the rest of the file
            for line in fp.readlines():

            #if empty line (only endl)   
                if len(line) == 1:
                    continue

                source_list_lines.append(line)

            fp.close()

        #**************************************************
        #write the concatenated sourcelist to a file (the full catalog path)
        fp = open(catalog_output_path, "w")
        #first the header
        fp.write(format_line)
        fp.write("\n")

        #then the sources
        for line in source_list_lines:
            fp.write(line)

        fp.write("\n")
        fp.close()


if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_source_finding(jobid, jobhost, jobport).run_with_stored_arguments())

