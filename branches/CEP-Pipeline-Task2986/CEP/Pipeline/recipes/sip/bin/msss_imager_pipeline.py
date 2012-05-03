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
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps


class msss_imager_pipeline(control):
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


    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = []
        self.target_data = []
        self.output_data = []
        self.scratch_directory = None


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
        super(msss_imager_pipeline, self).go()


    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        self.logger.info("Starting imager pipeline")

        # Define scratch directory to be used by the compute nodes.
        self.scratch_directory = os.path.join(
            self.inputs['working_directory'], self.inputs['job_name']
        )

        # Get input/output-data products specifications.
        self._get_io_product_specs()

        job_dir = self.config.get("layout", "job_directory")
        parset_dir = os.path.join(job_dir, "parsets")
        mapfile_dir = os.path.join(job_dir, "mapfiles")

        # Write input- and output data map-files.
        create_directory(parset_dir)
        create_directory(mapfile_dir)

        # ******************************************************************
        # (1) prepare phase: copy and collect the ms
        # TODO: some smart python-foo to get temp outputfilenames

        input_mapfile = os.path.join(mapfile_dir, "uvdata.mapfile")
        store_data_map(input_mapfile, self.input_data)
        self.logger.debug(
            "Wrote input UV-data mapfile: %s" % input_mapfile
        )
        target_mapfile = os.path.join(mapfile_dir, "target.mapfile")
        store_data_map(target_mapfile, self.target_data)
        self.logger.debug(
            "Wrote target mapfile: %s" % target_mapfile
        )
        output_image_mapfile = os.path.join(mapfile_dir, "images.mapfile")
        store_data_map(output_image_mapfile, self.output_data)
        self.logger.debug(
            "Wrote output sky-image mapfile: %s" % output_image_mapfile
        )

        # TODO: quick and dirty way to get the proper self.parset by just
        #       re-assigning it the subset of all PythonControl keys.
        self.parset = self.parset.makeSubset(
            self.parset.fullModuleName('PythonControl') + '.'
        )

        # Create the dir where found and processed ms are placed
        # raw_ms_per_image_map_path contains all the original ms locations:
        # this list contains possible missing files
        processed_ms_dir = os.path.join(self.scratch_directory, "subbands")
        concat_ms_map_path, timeslice_map_path, raw_ms_per_image_map_path = \
            self._prepare_phase(input_mapfile, target_mapfile, processed_ms_dir,
                                 skip = True)

        #We start with an empty source_list
        sourcelist_map = None
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
            aw_image_mapfile, maxbaseline = self._aw_imager(concat_ms_map_path,
                        idx_loop, sky_path,
                        skip = True)

            # *****************************************************************
            # (5) Source finding 
            sourcelist_map = self._source_finding(aw_image_mapfile,
                                    idx_loop, skip = True)
            #should the output be a sourcedb? instead of a sourcelist


        # The output does not contain the intermediate values
        #
        minbaseline = 0

        self._finalize(aw_image_mapfile, processed_ms_dir,
                       raw_ms_per_image_map_path, sourcelist_map,
                       minbaseline, maxbaseline, target_mapfile,
                       output_image_mapfile)

        return 0


    def _get_io_product_specs(self):
        """
        Get input- and output-data product specifications from the
        parset-file, and do some sanity checks.
        """
        odp = self.parset.makeSubset('ObsSW.Observation.DataProducts.')
        self.input_data = [tuple(os.path.join(*x).split(':')) for x in zip(
            odp.getStringVector('Input_Correlated.locations', []),
            odp.getStringVector('Input_Correlated.filenames', []))
        ]
        self.logger.debug("%d Input_Correlated data products specified" %
                          len(self.input_data))
        self.output_data = [tuple(os.path.join(*x).split(':')) for x in zip(
            odp.getStringVector('Output_SkyImage.locations', []),
            odp.getStringVector('Output_SkyImage.filenames', []))
        ]
        self.logger.debug("%d Output_SkyImage data products specified" %
                          len(self.output_data))
        # Sanity checks on input- and output data product specifications
        if not(validate_data_maps(self.input_data) and
               validate_data_maps(self.output_data)):
            raise PipelineException(
                "Validation of input/output data product specification failed!"
            )
        # Target data is basically scratch data, consisting of one concatenated
        # MS per image. It must be stored on the same host as the final image.
        for host, path in self.output_data:
            self.target_data.append(
                (host, os.path.join(self.scratch_directory, 'concat.ms'))
            )


    def _finalize(self, awimager_output_map, processed_ms_dir,
                  raw_ms_per_image_map, sourcelist_map, minbaseline,
                  maxbaseline, target_mapfile,
                  output_image_mapfile, skip = False):

        if skip:
            pass
        else:
            #run the awimager recipe
            self.run_task("imager_finalize", target_mapfile,
                          awimager_output_map = awimager_output_map,
                          raw_ms_per_image_map = raw_ms_per_image_map,
                          sourcelist_map = sourcelist_map,
                          minbaseline = minbaseline,
                          maxbaseline = maxbaseline,
                          target_mapfile = target_mapfile,
                          output_image_mapfile = output_image_mapfile,
                          processed_ms_dir = processed_ms_dir)


    def _source_finding(self, image_map_path, major_cycle, skip = True):
        bdsm_parset_pass_1 = self.parset.makeSubset("BDSM[0].")
        parset_path_pass_1 = self._write_parset_to_file(bdsm_parset_pass_1, "pybdsm_first_pass.par")
        bdsm_parset_pass_2 = self.parset.makeSubset("BDSM[1].")
        parset_path_pass_2 = self._write_parset_to_file(bdsm_parset_pass_2, "pybdsm_second_pass.par")

        # touch a mapfile to be filled with created sourcelists
        source_list_map = self._write_datamap_to_file(None,
             "source_finding_outputs")

        catalog_path = os.path.join(
            self.scratch_directory,
            "awimage_cycle_{0}".format(major_cycle),
            "bdsm_catalog"
        )

        # Run the sourcefinder
        if skip:
            return source_list_map
        else:
            self.run_task("imager_source_finding",
                        image_map_path,
                        bdsm_parset_file_run1 = parset_path_pass_1,
                        bdsm_parset_file_run2x = parset_path_pass_2,
                        working_directory = self.scratch_directory,
                        catalog_output_path = catalog_path,
                        mapfile = source_list_map)

            return source_list_map


    def _bbs(self, timeslice_map_path, parmdbs_map_path, sky_path, skip = False):
        #create parset for recipe
        parset = self.parset.makeSubset("BBS.")
        parset_path = self._write_parset_to_file(parset, "bbs")

        # create the output file path
        output_mapfile = self._write_datamap_to_file(None, "bbs_output")
        sky_parmdb_map_path = self._write_datamap_to_file(None, "sky_parmdb")
        if skip:
            return output_mapfile

        # The sky map contains a single sky file while imager_bbs expects a sky
        # file for each 'pardbm ms set combination'. 
        sky_map = load_data_map(sky_path)
        parmdbs_map = load_data_map(parmdbs_map_path)
        sky_parmdb_map = []
        for (sky, parmdbs) in zip(sky_map, parmdbs_map):
            (host_sky, sky_entry) = sky
            (host_parmdbs, parmdbs_entries) = parmdbs
            # sanity check: host should be the same
            if host_parmdbs != host_sky:
                self.logger.error("The input files for bbs do not contain "
                                  "matching host names for each entry")
                self.logger.error(repr(sky_map))
                self.logger.error(repr(parmdbs_map_path))

            #add the entries but with skymap multiplied with len (parmds list)
            sky_parmdb_map.append((host_sky, [sky_entry] * len(parmdbs_entries)))
        #save the new mapfile
        store_data_map(sky_parmdb_map_path, sky_parmdb_map)

        self.run_task("imager_bbs",
                      timeslice_map_path,
                      parset = parset_path,
                      instrument_mapfile = parmdbs_map_path,
                      sky_mapfile = sky_parmdb_map_path,
                      mapfile = output_mapfile,
                      working_directory = self.scratch_directory)

        return output_mapfile

    def _aw_imager(self, prepare_phase_output, major_cycle, sky_path, skip = False):
        parset = self.parset.makeSubset("AWimager.")
        mask_patch_size = self.parset.getInt("mask_patch_size")

        parset_path = self._write_parset_to_file(parset,
                            "awimager_cycle_{0}".format(major_cycle))
        image_path = os.path.join(
            self.scratch_directory, "awimage_cycle_{0}".format(major_cycle), "image"
        )
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
                          sourcedb_path = sky_path,
                          working_directory = self.scratch_directory)


        return output_mapfile, parset.getInt("maxbaseline")


    def _prepare_phase(self, input_ms_map_path, target_mapfile, processed_ms_dir,
                        skip = False):
        # get the parameters, create a subset for ndppp, save
        ndppp_parset = self.parset.makeSubset("DPPP.")
        ndppp_parset_path = self._write_parset_to_file(ndppp_parset,
                                                       "prepare_imager_ndppp")

        # create the output file paths
        #[1] output -> prepare_output
        output_mapfile = self._write_datamap_to_file(None, "prepare_output")
        time_slices_mapfile = self._write_datamap_to_file(None, "prepare_time_slices")
        raw_ms_per_image_mapfile = self._write_datamap_to_file(None, "raw_ms_per_image")

        # Run the prepare phase script 
        if skip:
            pass
        else:
            outputs = self.run_task("imager_prepare", input_ms_map_path,
                    parset = ndppp_parset_path,
                    target_mapfile = target_mapfile,
                    slices_per_image = self.parset.getInt("slices_per_image"),
                    subbands_per_image = self.parset.getInt("subbands_per_image"),
                    mapfile = output_mapfile,
                    slices_mapfile = time_slices_mapfile,
                    raw_ms_per_image_mapfile = raw_ms_per_image_mapfile,
                    working_directory = self.scratch_directory,
                    processed_ms_dir = processed_ms_dir)




        # If all is ok output_mapfile == target_mapfile
        return output_mapfile, time_slices_mapfile, raw_ms_per_image_mapfile


    def _create_dbs(self, input_map_path, timeslice_map_path, source_list = "" , skip_create_dbs = False):
        """
        Create for each of the concatenated input measurement sets 
        """
        # Create the parameters set
        parset = self.parset.makeSubset("GSM.")
        parset_path = self._write_parset_to_file(parset, "create_dbs")

        # create the files that will contain the output of the recipe
        parmdbs_path = self._write_datamap_to_file(None, "parmdbs")
        sky_path = self._write_datamap_to_file(None, "sky_files")

        #run the master script
        if skip_create_dbs:
            pass
        else:
            self.run_task("imager_create_dbs", input_map_path,
                        monetdb_hostname = parset.getString("monetdb_hostname"),
                        monetdb_port = parset.getInt("monetdb_port"),
                        monetdb_name = parset.getString("monetdb_name"),
                        monetdb_user = parset.getString("monetdb_user"),
                        monetdb_password = parset.getString("monetdb_password"),
                        assoc_theta = parset.getString("assoc_theta"),
                        sourcedb_suffix = ".sky",
                        slice_paths_mapfile = timeslice_map_path,
                        parmdb_suffix = ".parmdbm",
                        parmdbs_path = parmdbs_path,
                        sky_path = sky_path,
                        source_list_path = source_list,
                        working_directory = self.scratch_directory)

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
        Id supllied data is None then the file is touched if not existing, but
        existing files are kept as is
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


if __name__ == '__main__':
    sys.exit(msss_imager_pipeline().main())

