from __future__ import with_statement
import sys
import os
import shutil

from lofar.parameterset import parameterset
from lofarpipe.support.lofarnode import LOFARnodeTCP

from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.utilities import catch_segfaults


class imager_source_finding(LOFARnodeTCP):
    """
    The imager_source_finding recipe. In this script a number of pyBDSM call is 
    made. pyBDSM is a source finder which produces a list of sources and images
    with those sources removed.
    By using multiple iterations weak sources can be found and indexed. 
    
    For (max iter) or (no sources found):
    
    1. Select correct input image and parset based on the current iteration
    2. Convert the string values retrieved from the parset to python types
    3. Start pybdsm
    4. Export a sourcelist if sources found and save the image with source
       substracted
    
    And then:
    
    5. Combine the source lists into a single large sourcelist
    6. Create sourcedb based on the sourcelist and return this
       
    """
    def run(self, input_image, bdsm_parameter_run1_path,
            bdsm_parameter_run2x_path, catalog_output_path, image_output_path,
            sourcedb_target_path, environment, working_directory,
            create_sourcdb_exec):
        """
        :param input_image: image to look for sources in
        :param bdsm_parameter_run1_path: parset with bdsm parameters for the 
               first run
        :param bdsm_parameter_run2x_path: second ron bdsm parameters
        :param catalog_output_path: Path to full list of sources found
        :param image_output_path: Path to fits image with all sources 
               substracted
        :param sourcedb_target_path: Path to store the sourcedb created from 
            containing all the found sources
        :param environment: environment for runwithlog4cplus
        :param working_directory: Working dir
        :param create_sourcdb_exec: Path to create sourcedb executable 
        
        :rtype: self.outputs['source_db'] sourcedb_target_path
        
        """

        import lofar.bdsm as bdsm#@UnresolvedImport
        self.logger.info("Starting imager_source_finding")
        self.environment.update(environment)
        # default frequency is None (read from image), save for later cycles.
        # output of pybdsm forgets freq of source image
        frequency = None
        # Output of the for loop: n iterations and any source found
        n_itter_sourcefind = None
        sources_found = False
        max_sourcefind_itter = 5  # TODO: maximum itter is a magic value
        for idx in range(max_sourcefind_itter):
            # ******************************************************************
            # 1. Select correct input image
            # The first iteration uses the input image, second and later use the
            # output of the previous iteration. The 1+ iteration have a 
            # seperate parameter set. 
            if idx == 0:
                input_image_local = input_image # input_image_cropped
                image_output_path_local = image_output_path + "_0"
                bdsm_parameter_local = parameterset(bdsm_parameter_run1_path)
            else:
                input_image_local = image_output_path + "_{0}".format(
                                                                str(idx - 1))
                image_output_path_local = image_output_path + "_{0}".format(
                                                                    str(idx))
                bdsm_parameter_local = parameterset(bdsm_parameter_run2x_path)

            # *****************************************************************
            # 2. parse the parameters and convert to python if possible 
            # this is needed for pybdsm
            bdsm_parameters = {}
            for key in bdsm_parameter_local.keys():
                parameter_value = bdsm_parameter_local.getStringVector(key)[0]
                try:
                    parameter_value = eval(parameter_value)
                except:
                    pass  #do nothing
                bdsm_parameters[key] = parameter_value


            # *****************************************************************
            # 3. Start pybdsm
            self.logger.debug(
                "Starting sourcefinder bdsm on {0} using parameters:".format(
                                                        input_image_local))
            self.logger.debug(repr(bdsm_parameters))
            img = bdsm.process_image(bdsm_parameters,
                        filename = input_image_local, frequency = frequency)

            # Always export the catalog 
            img.write_catalog(
                outfile = catalog_output_path + "_{0}".format(str(idx)),
                catalog_type = 'gaul', clobber = True,
                format = "bbs", force_output = True)

            # If no more matching of sources with gausians is possible (nsrc==0)
            # break the loop
            if img.nsrc == 0:
                n_itter_sourcefind = idx
                break

            # We have at least found a single source!
            self.logger.debug("Number of source found: {0}".format(
                                                                img.nsrc))
            # *****************************************************************
            # 4. export the image with 

            self.logger.debug("Wrote list of sources to file at: {0})".format(
                                                            catalog_output_path))
            img.export_image(outfile = image_output_path_local,
                                 img_type = 'gaus_resid', clobber = True,
                                 img_format = "fits")
            self.logger.debug("Wrote fits image with substracted sources"
                                  " at: {0})".format(image_output_path_local))

            # Save the frequency from image header of the original input file,
            # This information is not written by pybdsm to the exported image
            frequency = img.frequency


        # if not set the maximum number of itteration us performed
        if n_itter_sourcefind == None:
            n_itter_sourcefind = max_sourcefind_itter

        # ********************************************************************
        # 5. The produced catalogs now need to be combined into a single list
        # Call with the number of loops and the path to the files, only combine
        # if we found sources
        #if sources_found:
        self.logger.debug(
                "Writing source list to file: {0}".format(catalog_output_path))
        self._combine_source_lists(n_itter_sourcefind, catalog_output_path)

        # *********************************************************************
        # 6. Convert sourcelist to sourcedb
        self._create_source_db(catalog_output_path, sourcedb_target_path,
            working_directory, create_sourcdb_exec, False)
        # Assign the outputs
        self.outputs["catalog_output_path"] = catalog_output_path
        self.outputs["source_db"] = sourcedb_target_path
        return 0

    def _combine_source_lists(self, n_itter_sourcefind, catalog_output_path):
        """
        Parse  and concate the produces sourcelists, files are numbered using 
        the sourcefind iteration. 
        For all sourcefind itterations with sources produced:
        
        1. Open the file for this itteration
        2. parse the files:
        
            a. get the format line
            b. skip whiteline
            c. collect all sources as strings
        
        3. Save the collected data:
        
            a. The format line (only a single formatter is need, same for each file)
            b. add the sources
            c. finish with an endl
            
        """
        source_list_lines = []
        format_line = None

        # If no sources are found at all n_itter_sourcefind == 0
        # But we do need to create a combined sourcelist and read this list
        if n_itter_sourcefind == 0:
            n_itter_sourcefind = 1 # at least use the first empty bdsm output  

        for idx_source_file in range(n_itter_sourcefind):
            # *****************************************************************
            # 1 . Open the file
            filepointer = open(catalog_output_path + "_{0}".format(
                                                            idx_source_file))
            #**************************************************
            # 2. Parse the files
            #   a. Read the format line and save (same for all bdsm runs)
            self.logger.error("opening file:")
            self.logger.error(catalog_output_path + "_{0}".format(
                                                            idx_source_file))

            format_line = filepointer.readline()

            #read the rest of the file
            for line in filepointer.readlines():
            #   b. if empty line (only endl)   
                if len(line) == 1:
                    continue
            #   c. Collect the sources a strings
                source_list_lines.append(line)
            filepointer.close()

        #**************************************************
        #3. write the concatenated sourcelist to a file (the full catalog path)
        filepointer = open(catalog_output_path, "w")
        #   a. first the header
        filepointer.write(format_line)
        filepointer.write("\n")

        #   b. then the sources
        for line in source_list_lines:
            filepointer.write(line)
        #   c. Whiteline
        filepointer.write("\n")
        filepointer.close()
        self.logger.debug("Wrote concatenated sourcelist to: {0}".format(
                                                catalog_output_path))


    def _create_source_db(self, source_list, sourcedb_target_path,
                          working_directory, create_sourcdb_exec, append = False):
        """
        Convert a sourcelist to a sourcedb:
        
        1. Remove existing sourcedb if not appending (sourcedb fails else)
        2. Call the sourcedb executable with the supplied parameters
         
        """
        # *********************************************************************
        # 1. remove existing sourcedb if not appending
        if (append == False) and os.path.isdir(sourcedb_target_path):
            shutil.rmtree(sourcedb_target_path)
            self.logger.debug("Removed existing sky model: {0}".format(
                                            sourcedb_target_path))

        # *********************************************************************
        # 2. The command and parameters to be run
        cmd = [create_sourcdb_exec, "in={0}".format(source_list),
               "out={0}".format(sourcedb_target_path),
               "format=<", # format according to Ger van Diepen
               "append=true"] # Always set append: no effect on non exist db
        self.logger.info(' '.join(cmd))

        try:
            with CatchLog4CPlus(working_directory,
                 self.logger.name + "." + os.path.basename("makesourcedb"),
                 os.path.basename(create_sourcdb_exec)
            ) as logger:
                catch_segfaults(cmd, working_directory, self.environment,
                                            logger, cleanup = None)

        except Exception, exception:
            self.logger.error("Execution of external failed:")
            self.logger.error(" ".join(cmd))
            self.logger.error("exception details:")
            self.logger.error(str(exception))
            return 1

        return 0

if __name__ == "__main__":
    #sys.path.insert(0, "/usr/lib/pymodules/python2.6")  #matlib plot fix (might not be needed anymore)
    _JOBID, _JOBHOST, _JOBPORT = sys.argv[1:4]
    sys.exit(imager_source_finding(_JOBID, _JOBHOST,
                                   _JOBPORT).run_with_stored_arguments())
    #del sys.path[0] # TODO: REMOVE FIRST ENTRY

