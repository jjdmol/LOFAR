#!/usr/bin/env python
#                                                         LOFAR IMAGING PIPELINE
#
#                                                         Imager Pipeline recipe
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys
import errno

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


    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = {}
        self.output_data = {}



    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        self.logger.info("Starting imager pipeline")
        # (1) ******************************************************************
        # prepare phase: copy and collect the ms
        concat_ms_map_path, timeslice_map_path = self._prepare_phase(
                self.inputs['input_mapfile'], skip = True)

        # (2) ******************************************************************
        # Create dbs and sky model
        parmdbs_path, sky_path = self._create_dbs(
                        concat_ms_map_path, skip_create_dbs = True)

        # (3) *******************************************************************#
        # bbs_imager recipe
        bbs_output = self._bbs(timeslice_map_path, parmdbs_path, sky_path, skip = True)

        # (4) ******************************************************************
        # Get parameters awimager from the prepare_parset and inputs
        awimager_output_mapfile = self._aw_imager(concat_ms_map_path, skip = False)

        self.logger.info(awimager_output_mapfile)

        # (5) *****************************************************************
        # Source finding
        found_sources_list = self._source_finding(awimager_output_mapfile, skip = False)

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

        bdsm_parset_pass_1 = self.parset.makeSubset("Source_finding.first_pass.")
        parset_path_pass_1 = self._write_parset_to_file(bdsm_parset_pass_1, "pybdsm_first_pass.par")
        bdsm_parset_pass_2 = self.parset.makeSubset("Source_finding.second_pass.")
        parset_path_pass_2 = self._write_parset_to_file(bdsm_parset_pass_2, "pybdsm_second_pass.par")


        # the image_map_path contains the paths without an image_postfix 
        # specifying the correct image to use (there are... 8)
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
            return [('lce068', '/data/scratch/klijn/temp_sourcelist')]
        else:
            return self.run_task("imager_source_finding",
                        image_map_appended_path,
                        bdsm_parset_file_run1 = parset_path_pass_1,
                        bdsm_parset_file_run2x = parset_path_pass_2,
                        catalog_output_path = "/data/scratch/klijn/jobs/Pipeline/bdsm_output_cat"
                        )


    def _bbs(self, timeslice_map_path, parmdbs_path, sky_path, skip = False):
        parset = self.parset.makeSubset("Bbs.")
        parset_path = self._write_parset_to_file(parset, "bbs")
        if skip:
            return parset.getString("output_mapfile")

        # The sky map contains a single sky file while imager_bbs expects a sky
        # file for each 'pardm ms set combination'
        sky_map = load_data_map(sky_path)
        parmdbs_map = load_data_map(parmdbs_path)
        sky_map_fixed = []
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
            sky_map_fixed.append((host_sky, [sky_entry] * len(parmdbs_entries)))

        sky_path_fixed = sky_path + ".bbs"
        store_data_map(sky_path_fixed, sky_map_fixed)
        bbs_imager_mapfile = \
            self.run_task("imager_bbs",
                          timeslice_map_path,
                          parset = parset_path,
                          instrument_mapfile = parmdbs_path,
                          sky_mapfile = sky_path_fixed,
                          mapfile = parset.getString("output_mapfile")
                          )['mapfile']

        return parset.getString("output_mapfile")

    def _aw_imager(self, prepare_phase_output, skip = False):
        parset = self.parset.makeSubset("Awimager.")
        parset_path = self._write_parset_to_file(parset, "awimager")
        awimager_output_mapfile = None
        if skip:
            awimager_output_mapfile = parset.getString("mapfile")
        else:
            #run the awimager recipe
            awimager_output_mapfile = \
            self.run_task("imager_awimager", prepare_phase_output,
                          parset = parset_path,
                          mapfile = parset.getString("mapfile"))["mapfile"]

        return awimager_output_mapfile


    def _prepare_phase(self, input_ms_map_path, skip = False):
        # get the parameters, create a subset for ndppp, save
        prepare_parset = self.parset.makeSubset("Prepare.")
        ndppp_parset = prepare_parset.makeSubset("Ndppp.")
        ndppp_parset_path = self._write_parset_to_file(ndppp_parset,
                                                       "prepare_imager_ndppp")

        # Run the prepare phase script 
        # TODO: wrapping to allow for skipping
        prepare_phase_output = None
        if skip:
          prepare_phase_output = prepare_parset.getString("mapfile")
        else:
          prepare_phase_output = \
            self.run_task("imager_prepare", input_ms_map_path,
                ndppp_path = prepare_parset.getString("ndppp_path"),
                parset = ndppp_parset_path,
                output_mapfile = prepare_parset.getString("output_mapfile"),
                slices_per_image = prepare_parset.getInt("slices_per_image"),
                subbands_per_image = prepare_parset.getInt("subbands_per_image"),
                mapfile = prepare_parset.getString("mapfile"),
                slices_mapfile = prepare_parset.getString("slices_mapfile"))['mapfile']

        self.logger.info(prepare_phase_output)
        return prepare_phase_output, prepare_parset.getString("slices_mapfile")


    def _create_dbs(self, input_map_path, skip_create_dbs = False):
        """
        The executable called by this script do not consume parsets:
        Parse the parset entries and pass as argument       
        """
        parset = self.parset.makeSubset("Create_dbs.")
        parset_path = self._write_parset_to_file(parset, "create_dbs")

        recipe_output = None
        if skip_create_dbs:
            recipe_output = {}
            recipe_output["parmdbs_path"] = parset.getString("parmdbs_path")
            recipe_output["sky_path"] = parset.getString("sky_path")
        else:
            recipe_output = \
                self.run_task("imager_create_dbs", input_map_path,
                        parset = parset_path,
                        monetdb_hostname = parset.getString("monetdb_hostname"),
                        monetdb_port = parset.getInt("monetdb_port"),
                        monetdb_name = parset.getString("monetdb_name"),
                        monetdb_user = parset.getString("monetdb_user"),
                        monetdb_password = parset.getString("monetdb_password"),
                        assoc_theta = parset.getString("assoc_theta"),
                        suffix = parset.getString("suffix"),
                        sourcedb_target_path = parset.getString("sourcedb_target_path"),
                        slice_paths_mapfile = parset.getString("slice_paths_mapfile"),
                        parmdb_suffix = parset.getString("parmdb_suffix"),
                        monetdb_path = parset.getString("monetdb_path"),
                        parmdbs_path = parset.getString("parmdbs_path"),
                        sky_path = parset.getString("sky_path")
                        )

        return recipe_output["parmdbs_path"], recipe_output["sky_path"]


    def _write_parset_to_file(self, parset, parset_name):
        """
        Write the suplied the suplied parameterset to the parameter set 
        directory in the jobs dir with the filename suplied in parset_name.
        Return the full path to the created file.
        
        """
        # todo make a subset parset and supply to the prepareimager        
        parset_dir = os.path.join(
            self.config.get("layout", "job_directory"), "parsets")
        #create the parset dir if it does not exist
        create_directory(parset_dir)

        #write the content to a new parset file
        prepare_imager_parset_file = os.path.join(parset_dir,
                         "{0}.parset".format(parset_name))
        parset.writeFile(prepare_imager_parset_file)

        return prepare_imager_parset_file

if __name__ == '__main__':
    sys.exit(imager_pipeline().main())
