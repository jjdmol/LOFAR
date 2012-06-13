from __future__ import with_statement
import sys
import os
import shutil

from lofar.parameterset import parameterset
from lofarpipe.support.lofarnode import LOFARnodeTCP
import lofar.bdsm as bdsm#@UnresolvedImport

from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.utilities import catch_segfaults


class imager_source_finding(LOFARnodeTCP):
    """
    The imager_source_finding    
    """
    def run(self, input_image, bdsm_parameter_run1_path,
            bdsm_parameter_run2x_path, catalog_output_path, image_output_path,
            sourcedb_target_path, init_script, working_directory, executable):
        self.logger.info("Starting imager_source_finding")
        # default frequency is None (read from image), save for later cycles
        frequency = None
        number_of_sourcefind_itterations = None
        sources_found = False
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

            self.logger.debug(
                "Starting sourcefinder bdsm on {0} using parameters:".format(
                                                        input_image_local))
            self.logger.debug(repr(bdsm_parameters))
            img = bdsm.process_image(bdsm_parameters,
                        filename = input_image_local, frequency = frequency)


            # If no more matching of sources with gausians is possible (nsrc==0)
            # break the loop
            if img.nsrc == 0:
                self.logger.debug("No sources found: exiting")
                number_of_sourcefind_itterations = idx
                break
            else:
                # We have at least found a single source!
                self.logger.debug("Number of source found: {0}".format(img.nsrc))
                sources_found = True


            #export the catalog and the image with gausians substracted
            img.write_catalog(outfile = catalog_output_path + "_{0}".format(str(idx)),
                              catalog_type = 'gaul', clobber = True, format = "bbs")

            self.logger.debug("Wrote list of sources to file at: {0})".format(
                                                        catalog_output_path))
            img.export_image(outfile = image_output_path_local,
                             img_type = 'gaus_resid', clobber = True,
                             img_format = "fits")
            self.logger.debug("Wrote fits image with substracted sources"
                              " at: {0})".format(catalog_output_path))
            #img does not have close()

            # Save the frequency from image header of the original input file,
            # This information is not written by pybdsm to the exported image
            frequency = img.cfreq

        # if not set the maximum number of itteration us performed
        if number_of_sourcefind_itterations == None:
            number_of_sourcefind_itterations = max_sourcefind_itter

        # The produced catalogs now need to be concatenated into a single list
        # Call with the number of loops and the path to the files, only combine
        # if we found sources
        if sources_found:
            self.logger.debug(
                "Writing source list to file: %s" % catalog_output_path
            )
            self._combine_source_lists(number_of_sourcefind_itterations,
                                   catalog_output_path)

        # TODO: return the sourcedb??
        # ik denk van niet: als er een fout op treed eindigd deze script

        self._create_source_db(catalog_output_path, sourcedb_target_path,
                init_script, working_directory, executable, False)

        return 0


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
        self.logger.debug("Wrote concatenated sourcelist to: {0}".format(
                                                catalog_output_path))


    def _create_source_db(self, source_list, sourcedb_target_path, init_script,
                          working_directory, executable, append = False):
        """
        _create_source_db consumes a skymap text file and produces a source db
        (pyraptable) 
        """
        #remove existing sourcedb if not appending
        if (append == False) and os.path.isdir(sourcedb_target_path):
            shutil.rmtree(sourcedb_target_path)
            self.logger.debug("Removed existing sky model: {0}".format(
                                            sourcedb_target_path))

        # The command and parameters to be run
        cmd = [executable, "in={0}".format(source_list),
               "out={0}".format(sourcedb_target_path),
               "format=<", # format according to Ger van Diepen
               "append=true"] # Always set append flag: no effect on non exist db
        self.logger.info(' '.join(cmd))

        try:
            environment = read_initscript(self.logger, init_script)
            with CatchLog4CPlus(working_directory,
                 self.logger.name + "." + os.path.basename("makesourcedb"),
                 os.path.basename(executable)
            ) as logger:
                    catch_segfaults(cmd, working_directory, environment,
                                            logger, cleanup = None)

        except Exception, e:
            self.logger.error("Execution of external failed:")
            self.logger.error(" ".join(cmd))
            self.logger.error("exception details:")
            self.logger.error(str(e))
            return 1

        return 0

if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_source_finding(jobid, jobhost, jobport).run_with_stored_arguments())

