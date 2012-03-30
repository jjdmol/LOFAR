#!/usr/bin/env python
#                                                         LOFAR IMAGING PIPELINE
#
#                                                         Imager Pipeline recipe
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys

from lofarpipe.support.control import control
from lofar.parameterset import parameterset #@UnresolvedImport
import lofarpipe.support.lofaringredient as ingredient
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.group_data import load_data_map, store_data_map


class imager_pipeline(control):
    """
    The MSSS imager pipeline can be used to generate MSSS images.

    MSSS images are compiled from a number of so-called slices. Each slice
    comprises a short (approx. 10 min) observation of a field (an area on the
    sky) containing 80 subbands. The number of slices will be different for LBA
    observations (typically 9) and HBA observations (typically 2), due to
    differences in sensitivity.

    One MSSS observation will produce a number of images (typically 8), one for
    each so-called subband-group (SBG). Each SBG consists of the same number
    of consecutive subbands (typically 10).
    
    Each image will be compiled on a different cluster node to balance the
    processing load. The input- and output- files and locations are determined
    by the scheduler and specified in the parset-file.

    This pipeline will perform the following operations:
    - Copy the preprocessed MS's from the different compute nodes to the nodes
      where the images will be compiled (the prepare phase).
    - Flag the long baselines using DPPP
    - Concatenate the MS's of the different slices as one virtual MS for
      imaging.
    - Generate a local sky model (LSM) from the global sky model (GSM) for the
      sources that are in the field-of-view (FoV).
    - Repeat until convergence (3 times for the time being):
      - Per slice: solve and correct for phases using BBS with TEC enabled
      - Run the awimager.
      - Run the source finder (PyBDSM) and update the local sky model (LSM).
      
    Per subband-group, the following output products will be delivered:
    - Calibration solutions and corrected visibilities
    - An image
    - A source list
    """


    inputs = {
        'input_mapfile': ingredient.FileField(
            '--input-mapfile',
            help = "mapfile with inputs specific to this node script"
        )
    }

    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        self.logger.info("Starting imager pipeline")
        # ******************************************************************
        # (1) prepare phase: copy and collect the ms
        # TODO: inputs to this step are not ready for automation: need input 
        # marcel
        target_mapfile = "/home/klijn/build/preparation/output.map"
        input_mapfile = self.inputs['input_mapfile']
        concat_ms_map_path, timeslice_map_path = self._prepare_phase(
                input_mapfile, target_mapfile, skip = True)


        #We start with an empty source_list
        source_list = ""
        number_of_major_cycles = self.parset.getInt("number_of_major_cycles")
        for idx_loop in range(number_of_major_cycles):
            # TODO: Remove debugging skip code
            if idx_loop == 1:
                skip_current_loop = False
            else:
                skip_current_loop = False

            # ******************************************************************
            # (2) Create dbs and sky model
            parmdbs_path, sky_path = self._create_dbs(
                        concat_ms_map_path, timeslice_map_path,
                        source_list = source_list,
                        skip_create_dbs = True)

            # *****************************************************************
            # (3)  bbs_imager recipe
            bbs_output = self._bbs(timeslice_map_path, parmdbs_path, sky_path,
                        skip = True)


            # ******************************************************************
            # (4) Get parameters awimager from the prepare_parset and inputs
            awimager_output_mapfile = self._aw_imager(concat_ms_map_path,
                        idx_loop,
                        skip = False)

            return 0
            # *****************************************************************
            # (5 )Source finding 
            source_list = self._source_finding(awimager_output_mapfile,
                        skip = False)
            return 0


            self.logger.info(source_list)

        return 0


    def usage(self):
        print >> sys.stderr, "Usage: %s <parset-file>  [options]" % sys.argv[0]
        return 1


    def go(self):
        """
        Read the parset-file that was given as input argument, and set the
        jobname before calling the base-class's `go()` method.
        """
        try:
            parset_file = self.inputs['args'][0]
        except IndexError:
            return self.usage()
        self.parset.adoptFile(parset_file)
        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not self.inputs.has_key('job_name'):
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0]
            )
        super(imager_pipeline, self).go()

    def _source_finding(self, image_map_path, skip = True):
        #todo: ugly code
        bdsm_parset_pass_1 = self.parset.makeSubset("Source_finding.first_pass.")
        parset_path_pass_1 = self._write_parset_to_file(bdsm_parset_pass_1, "pybdsm_first_pass.par")
        bdsm_parset_pass_2 = self.parset.makeSubset("Source_finding.second_pass.")
        parset_path_pass_2 = self._write_parset_to_file(bdsm_parset_pass_2, "pybdsm_second_pass.par")

        # the image_map_path contains the paths without an image_postfix 
        # specifying the correct image to use (there are... 8 generated by the)
        # imager
        # load the mapfile, correct it and write a new one.
        image_postfix = self.parset.getString("Source_finding.image_postfix")
        image_map = load_data_map(image_map_path)
        corrected_image_map = []
        for (host, path) in image_map:
            corrected_image_map.append((host, path + image_postfix))

        image_map_appended_path = image_map_path + image_postfix
        store_data_map(image_map_appended_path, corrected_image_map)

        # Run the sourcefinder
        if skip:
            return '/data/scratch/klijn/jobs/Pipeline/bdsm_output_cat'
        else:
            return self.run_task("imager_source_finding",
                        image_map_appended_path,
                        bdsm_parset_file_run1 = parset_path_pass_1,
                        bdsm_parset_file_run2x = parset_path_pass_2,
                        #TODO: deze moet dus nog dynamisch
                        catalog_output_path = "/data/scratch/klijn/jobs/Pipeline/bdsm_output_cat"
                        )


    def _bbs(self, timeslice_map_path, parmdbs_path, sky_path, skip = False):
        #create parset for recipe
        parset = self.parset.makeSubset("Bbs.BbsControl.")
        parset_path = self._write_parset_to_file(parset, "bbs")

        # create the output file path
        output_mapfile = self._write_datamap_to_file(None, "bbs_output")
        sky_parmdb_map_path = self._write_datamap_to_file(None, "sky_parmdb")
        if skip:
            return output_mapfile

        # The sky map contains a single sky file while imager_bbs expects a sky
        # file for each 'pardbm ms set combination'. 
        sky_map = load_data_map(sky_path)
        parmdbs_map = load_data_map(parmdbs_path)
        sky_parmdb_map = []
        for (sky, parmdbs) in zip(sky_map, parmdbs_map):
            (host_sky, sky_entry) = sky
            (host_parmdbs, parmdbs_entries) = parmdbs
            # sanity check: host should be the same
            if host_parmdbs != host_sky:
                self.logger.error("The input files for bbs do not contain "
                                  "matching host names for each entry")
                self.logger.error(repr(sky_map))
                self.logger.error(repr(parmdbs_path))

            #add the entries but with skymap multiplied with len (parmds list)
            sky_parmdb_map.append((host_sky, [sky_entry] * len(parmdbs_entries)))
        #save the new mapfile
        store_data_map(sky_parmdb_map_path, sky_parmdb_map)

        self.run_task("imager_bbs",
                      timeslice_map_path,
                      parset = parset_path,
                      instrument_mapfile = parmdbs_path,
                      sky_mapfile = sky_parmdb_map_path,
                      mapfile = output_mapfile)

        return output_mapfile

    def _aw_imager(self, prepare_phase_output, major_cycle, skip = False):
        parset = self.parset.makeSubset("Awimager.")
        mask_patch_size = parset.getInt("mask_patch_size")
        parset_path = self._write_parset_to_file(parset,
                            "awimager_cycle_{0}".format(major_cycle))
        image_path = os.path.join(self.config.get("DEFAULT", "default_working_directory"),
                                  "awimage_cycle_{0}".format(major_cycle), "image")
        output_mapfile = self._write_datamap_to_file(None, "awimager")
        if skip:
            pass
        else:
            #run the awimager recipe
            self.run_task("imager_awimager", prepare_phase_output,
                          parset = parset_path,
                          mapfile = output_mapfile,
                          output_image = image_path,
                          mask_patch_size = mask_patch_size,
                          sourcedb_path = "/data/scratch/klijn/jobs/Pipeline/concat.ms.sky")


        return output_mapfile


    def _prepare_phase(self, input_ms_map_path, target_mapfile, skip = False):
        # get the parameters, create a subset for ndppp, save
        prepare_parset = self.parset.makeSubset("Prepare.")
        ndppp_parset = prepare_parset.makeSubset("Ndppp.")
        ndppp_parset_path = self._write_parset_to_file(ndppp_parset,
                                                       "prepare_imager_ndppp")

        # create the output file paths
        output_mapfile = self._write_datamap_to_file(None, "prepare_output")
        time_slices_mapfile = self._write_datamap_to_file(None, "prepare_time_slices")

        # Run the prepare phase script 
        if skip:
            pass
        else:
            self.run_task("imager_prepare", input_ms_map_path,
                    parset = ndppp_parset_path,
                    target_mapfile = target_mapfile,
                    slices_per_image = prepare_parset.getInt("slices_per_image"),
                    subbands_per_image = prepare_parset.getInt("subbands_per_image"),
                    mapfile = output_mapfile,
                    slices_mapfile = time_slices_mapfile)


        # If all is ok output_mapfile == target_mapfile
        return output_mapfile, time_slices_mapfile


    def _create_dbs(self, input_map_path, timeslice_map_path, source_list = "" , skip_create_dbs = False):
        """
        Create for each of the concatenated input measurement sets 
        """
        # Create the parameters set
        parset = self.parset.makeSubset("Create_dbs.")
        parset_path = self._write_parset_to_file(parset, "create_dbs")

        # create the files that will contain the output of the recipe
        parmdbs_path = self._write_datamap_to_file(None, "parmdbs")
        sky_path = self._write_datamap_to_file(None, "sky_files")

        #run the master script
        if skip_create_dbs:
            pass
        else:
            self.run_task("imager_create_dbs", input_map_path,
                        parset = parset_path,
                        monetdb_hostname = parset.getString("monetdb_hostname"),
                        monetdb_port = parset.getInt("monetdb_port"),
                        monetdb_name = parset.getString("monetdb_name"),
                        monetdb_user = parset.getString("monetdb_user"),
                        monetdb_password = parset.getString("monetdb_password"),
                        assoc_theta = parset.getString("assoc_theta"),
                        sourcedb_suffix = parset.getString("sourcedb_suffix"),
                        slice_paths_mapfile = timeslice_map_path,
                        parmdb_suffix = parset.getString("parmdb_suffix"),
                        parmdbs_path = parmdbs_path,
                        sky_path = sky_path,
                        source_list_path = source_list
                        )

        return parmdbs_path, sky_path


    def _write_parset_to_file(self, parset, parset_name):
        """
        Write the suplied the suplied parameterset to the parameter set 
        directory in the jobs dir with the filename suplied in parset_name.
        Return the full path to the created file.

        """
        parset_dir = os.path.join(
            self.config.get("layout", "job_directory"), "parsets")
        #create the parset dir if it does not exist
        create_directory(parset_dir)

        #write the content to a new parset file
        parset_path = os.path.join(parset_dir,
                         "{0}.parset".format(parset_name))
        parset.writeFile(parset_path)

        return parset_path


    def _write_datamap_to_file(self, datamap, mapfile_name):
        """
        Write the suplied the suplied map to the mapfile  
        directory in the jobs dir with the filename suplied in mapfile_name.
        Return the full path to the created file.  
        Id supllied data is None then the file is touched, but not filled with 
        data
        """

        mapfile_dir = os.path.join(
            self.config.get("layout", "job_directory"), "mapfiles")
        #create the mapfile_dir if it does not exist
        create_directory(mapfile_dir)

        #write the content to a new parset file
        mapfile_path = os.path.join(mapfile_dir,
                         "{0}.map".format(mapfile_name))

        if datamap != None:
            store_data_map(mapfile_path, datamap)
        else:
            if not os.path.exists(mapfile_path):
                open(mapfile_path, 'w').close()

        return mapfile_path


    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = {}
        self.output_data = {}


if __name__ == '__main__':
    sys.exit(imager_pipeline().main())
