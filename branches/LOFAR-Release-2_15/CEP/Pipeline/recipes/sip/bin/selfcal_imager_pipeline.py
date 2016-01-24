#!/usr/bin/env python
#                                                        LOFAR IMAGING PIPELINE
#
#                                                      selfcal Pipeline recipe
#                                                            Marcel Loose, 2012
#                                                               loose@astron.nl
#                                                            Wouter Klijn, 2012
#                                                               klijn@astron.nl
#                                                         Nicolas Vilchez, 2014
#                                                             vilchez@astron.nl
# -----------------------------------------------------------------------------
import os
import sys
import copy
import shutil

from lofarpipe.support.control import control
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.data_map import DataMap, validate_data_maps,\
                                       MultiDataMap, align_data_maps
from lofarpipe.support.utilities import patch_parset, get_parset
from lofarpipe.support.loggingdecorators import xml_node, mail_log_on_exception

from lofar.parameterset import parameterset


class selfcal_imager_pipeline(control):
    """
    The self calibration pipeline is used to generate images and find
    sources in the generated images. Generated images and lists of found
    sources are complemented with meta data and thus ready for consumption by
    the Long Term Storage (LTA)

    *subband groups*
    The imager_pipeline is able to generate images on the frequency range of
    LOFAR in parallel. Combining the frequency subbands together in so called
    subbandgroups. Each subband group will result in an image and sourcelist,
    (typically 8, because ten subband groups are combined).

    *Time Slices*
    selfcal images are compiled from a number of so-called (time) slices. Each
    slice comprises a short (approx. 10 min) observation of a field (an area on
    the sky) containing typically 80 subbands. The number of slices will be
    different for LBA observations (typically 9) and HBA observations
    (typically 2), due to differences in sensitivity.

    Each image will be compiled on a different cluster node to balance the
    processing load. The input- and output- files and locations are determined
    by the scheduler and specified in the parset-file.

    **This pipeline performs the following operations:**

    1. Prepare Phase. Copy the preprocessed MS's from the different compute
       nodes to the nodes where the images will be compiled (the prepare phase)
       Combine the subbands in subband groups, concattenate the timeslice in a
       single large measurement set and perform flagging, RFI and bad station
       exclusion.
    2. Create db. Generate a local sky model (LSM) from the global sky model
       (GSM) for the sources that are in the field-of-view (FoV). The LSM
       is stored as sourcedb.
       In step 3 calibration of the measurement sets is performed on these
       sources and in step 4 to create a mask for the awimager. The calibration
       solution will be placed in an instrument table/db also created in this
       step.
    3. BBS. Calibrate the measurement set with the sourcedb from the gsm.
       In later iterations sourced found in the created images will be added
       to this list. Resulting in a selfcalibration cycle.
    4. Awimager. The combined  measurement sets are now imaged. The imaging
       is performed using a mask: The sources in the sourcedb are used to
       create an casa image masking known sources. Together with the
       measurement set an image is created.
    5. Sourcefinding. The images created in step 4 are fed to pyBDSM to find
       and describe sources. In multiple itterations substracting the found
       sources, all sources are collectedin a sourcelist.
       Step I. The sources found in step 5 are fed back into step 2.
       This allows the Measurement sets to be calibrated with sources currently
       found in the image. This loop will continue until convergence (3 times
       for the time being).
    6. Finalize. Meta data with regards to the input, computations performed
       and results are collected an added to the casa image. The images created
       are converted from casa to HDF5 and copied to the correct output
       location.
    7. Export meta data: An outputfile with meta data is generated ready for
       consumption by the LTA and/or the LOFAR framework.


    **Per subband-group, the following output products will be delivered:**

    a. An image
    b. A source list
    c. (Calibration solutions and corrected visibilities)

    """
    def __init__(self):
        """
        Initialize member variables and call superclass init function
        """
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = DataMap()
        self.target_data = DataMap()
        self.output_data = DataMap()
        self.output_correlated_data = DataMap()
        self.scratch_directory = None
        self.parset_feedback_file = None
        self.parset_dir = None
        self.mapfile_dir = None

    def usage(self):
        """
        Display usage information
        """
        print >> sys.stderr, "Usage: %s <parset-file>  [options]" % sys.argv[0]
        return 1

    def go(self):
        """
        Read the parset-file that was given as input argument, and set the
        jobname before calling the base-class's `go()` method.
        """
        try:
            parset_file = os.path.abspath(self.inputs['args'][0])
        except IndexError:
            return self.usage()
        self.parset.adoptFile(parset_file)
        self.parset_feedback_file = parset_file + "_feedback"
        # Set job-name to basename of parset-file w/o extension, if it's not
        # set on the command-line with '-j' or '--job-name'
        if not 'job_name' in self.inputs:
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0]
            )
        return super(selfcal_imager_pipeline, self).go()

    @mail_log_on_exception
    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        self.logger.info("Starting imager pipeline")

        # Define scratch directory to be used by the compute nodes.
        self.scratch_directory = os.path.join(
            self.inputs['working_directory'], self.inputs['job_name'])
        # Get input/output-data products specifications.
        self._get_io_product_specs()

        # remove prepending parset identifiers, leave only pipelinecontrol
        full_parset = self.parset
        self.parset = self.parset.makeSubset(
            self.parset.fullModuleName('PythonControl') + '.')  # remove this

        # Create directories to store communication and data files

        job_dir = self.config.get("layout", "job_directory")

        self.parset_dir = os.path.join(job_dir, "parsets")
        create_directory(self.parset_dir)
        self.mapfile_dir = os.path.join(job_dir, "mapfiles")
        create_directory(self.mapfile_dir)

        # *********************************************************************
        # (INPUT) Get the input from external sources and create pipeline types
        # Input measure ment sets
        input_mapfile = os.path.join(self.mapfile_dir, "uvdata.mapfile")
        self.input_data.save(input_mapfile)
        # storedata_map(input_mapfile, self.input_data)
        self.logger.debug(
            "Wrote input UV-data mapfile: {0}".format(input_mapfile))

        # Provides location for the scratch directory and concat.ms location
        target_mapfile = os.path.join(self.mapfile_dir, "target.mapfile")
        self.target_data.save(target_mapfile)
        self.logger.debug(
            "Wrote target mapfile: {0}".format(target_mapfile))

        # images datafiles
        output_image_mapfile = os.path.join(self.mapfile_dir, "images.mapfile")
        self.output_data.save(output_image_mapfile)
        self.logger.debug(
            "Wrote output sky-image mapfile: {0}".format(output_image_mapfile))

        # Location of the output measurement set
        output_correlated_mapfile = os.path.join(self.mapfile_dir, 
                                                 "correlated.mapfile")
        self.output_correlated_data.save(output_correlated_mapfile)
        self.logger.debug(
            "Wrote output correlated mapfile: {0}".format(output_correlated_mapfile))

        # Get pipeline parameters from the toplevel recipe
        # TODO: This is a backdoor option to manually add beamtables when these
        # are missing on the provided ms. There is NO use case for users of the
        # pipeline
        add_beam_tables = self.parset.getBool(
                                    "Imaging.addBeamTables", False)


        number_of_major_cycles = self.parset.getInt(
                                    "Imaging.number_of_major_cycles")

        # Almost always a users wants a partial succes above a failed pipeline
        output_result_of_last_succesfull_cycle = self.parset.getBool(
                            "Imaging.output_on_error", True)


        if number_of_major_cycles < 3:
            self.logger.error(
                "The number of major cycles must be 3 or higher, correct"
                " the key: Imaging.number_of_major_cycles")
            raise PipelineException(
                     "Incorrect number_of_major_cycles in the parset")


        # ******************************************************************
        # (1) prepare phase: copy and collect the ms
        concat_ms_map_path, timeslice_map_path, ms_per_image_map_path, \
            processed_ms_dir = self._prepare_phase(input_mapfile,
                                    target_mapfile, add_beam_tables)

        # We start with an empty source_list map. It should contain n_output
        # entries all set to empty strings
        source_list_map_path = os.path.join(self.mapfile_dir,
                                        "initial_sourcelist.mapfile")
        source_list_map = DataMap.load(target_mapfile) # copy the output map
        for item in source_list_map:
            item.file = ""             # set all to empty string
        source_list_map.save(source_list_map_path)

        succesfull_cycle_mapfiles_dict = None
        for idx_cycle in range(number_of_major_cycles):
            try:
                # *****************************************************************
                # (2) Create dbs and sky model
                parmdbs_path, sourcedb_map_path = self._create_dbs(
                            concat_ms_map_path, timeslice_map_path, idx_cycle,
                            source_list_map_path = source_list_map_path,
                            skip_create_dbs = False)


                # *****************************************************************
                # (3)  bbs_imager recipe.
                bbs_output = self._bbs(concat_ms_map_path, timeslice_map_path, 
                        parmdbs_path, sourcedb_map_path, idx_cycle, skip = False)

            
                # TODO: Extra recipe: concat timeslices using pyrap.concatms
                # (see prepare) redmine issue #6021
                # Done in imager_bbs.p at the node level after calibration 

                # *****************************************************************
                # (4) Get parameters awimager from the prepare_parset and inputs
                aw_image_mapfile, maxbaseline = self._aw_imager(concat_ms_map_path,
                            idx_cycle, sourcedb_map_path, number_of_major_cycles,
                            skip = False)

                # *****************************************************************
                # (5) Source finding
                source_list_map_path, found_sourcedb_path = self._source_finding(
                        aw_image_mapfile, idx_cycle, skip = False)
                # should the output be a sourcedb? instead of a sourcelist

                # save the active mapfiles: locations and content
                # Used to output last succesfull cycle on error
                mapfiles_to_save = {'aw_image_mapfile':aw_image_mapfile,
                                    'source_list_map_path':source_list_map_path,
                                    'found_sourcedb_path':found_sourcedb_path,
                                    'concat_ms_map_path':concat_ms_map_path}
                succesfull_cycle_mapfiles_dict = self._save_active_mapfiles(idx_cycle, 
                                      self.mapfile_dir, mapfiles_to_save)

            # On exception there is the option to output the results of the 
            # last cycle without errors
            except KeyboardInterrupt, ex:
                raise ex

            except Exception, ex:
                self.logger.error("Encountered an fatal exception during self"
                                  "calibration. Aborting processing and return"
                                  " the last succesfull cycle results")
                self.logger.error(str(ex))

                # if we are in the first cycle always exit with exception
                if idx_cycle == 0:
                    raise ex

                if not output_result_of_last_succesfull_cycle:
                    raise ex
                
                # restore the mapfile variables
                aw_image_mapfile = succesfull_cycle_mapfiles_dict['aw_image_mapfile']
                source_list_map_path = succesfull_cycle_mapfiles_dict['source_list_map_path']
                found_sourcedb_path = succesfull_cycle_mapfiles_dict['found_sourcedb_path']
                concat_ms_map_path = succesfull_cycle_mapfiles_dict['concat_ms_map_path']

                # set the number_of_major_cycles to the correct number
                number_of_major_cycles = idx_cycle - 1
                max_cycles_reached = False
                break
            else:
                max_cycles_reached = True


        # TODO: minbaseline should be a parset value as is maxbaseline..
        minbaseline = 0

        # *********************************************************************
        # (6) Finalize:
        placed_data_image_map, placed_correlated_map =  \
                                        self._finalize(aw_image_mapfile, 
            processed_ms_dir, ms_per_image_map_path, source_list_map_path,
            minbaseline, maxbaseline, target_mapfile, output_image_mapfile,
            found_sourcedb_path, concat_ms_map_path, output_correlated_mapfile)

        # *********************************************************************
        # (7) Get metadata
        # create a parset with information that is available on the toplevel

        self._get_meta_data(number_of_major_cycles, placed_data_image_map,
                       placed_correlated_map, full_parset, 
                       max_cycles_reached)


        return 0

    def _save_active_mapfiles(self, cycle_idx, mapfile_dir, mapfiles = {}):
        """
        receives a dict with active mapfiles, var name to path
        Each mapfile is copier to a seperate directory and saved
        THis allows us to exit the last succesfull run
        """
        # create a directory for storing the saved mapfiles, use cycle idx
        mapfile_for_cycle_dir = os.path.join(mapfile_dir, "cycle_" + str(cycle_idx))
        create_directory(mapfile_for_cycle_dir)

        saved_mapfiles = {}
        for (var_name,mapfile_path) in mapfiles.items():
            shutil.copy(mapfile_path, mapfile_for_cycle_dir)
            # save the newly created file, get the filename, and append it
            # to the directory name
            saved_mapfiles[var_name] = os.path.join(mapfile_for_cycle_dir,
                                          os.path.basename(mapfile_path))

        return saved_mapfiles
            
            


    def _get_meta_data(self, number_of_major_cycles, placed_data_image_map,
                       placed_correlated_map, full_parset, max_cycles_reached):
        """
        Function combining all the meta data collection steps of the processing
        """
        parset_prefix = full_parset.getString('prefix') + \
                full_parset.fullModuleName('DataProducts')
                    
        toplevel_meta_data = parameterset()
        toplevel_meta_data.replace(
             parset_prefix + ".numberOfMajorCycles", 
                                           str(number_of_major_cycles))
        toplevel_meta_data_path = os.path.join(
                self.parset_dir, "toplevel_meta_data.parset")

        toplevel_meta_data.replace(parset_prefix + ".max_cycles_reached",
                                  str(max_cycles_reached))

        try:
            toplevel_meta_data.writeFile(toplevel_meta_data_path)
            self.logger.info("Wrote meta data to: " + 
                    toplevel_meta_data_path)
        except RuntimeError, err:
            self.logger.error(
              "Failed to write toplevel meta information parset: %s" % str(
                                    toplevel_meta_data_path))
            return 1

        skyimage_metadata = "%s_feedback_SkyImage" % (self.parset_file,)
        correlated_metadata = "%s_feedback_Correlated" % (self.parset_file,)

        # Create a parset-file containing the metadata for MAC/SAS at nodes
        self.run_task("get_metadata", placed_data_image_map,           
            parset_prefix = parset_prefix,
            product_type = "SkyImage",
            metadata_file = skyimage_metadata)

        self.run_task("get_metadata", placed_correlated_map,
            parset_prefix = parset_prefix,
            product_type = "Correlated",
            metadata_file = correlated_metadata)

        self.send_feedback_processing(toplevel_meta_data)
        self.send_feedback_dataproducts(parameterset(skyimage_metadata))
        self.send_feedback_dataproducts(parameterset(correlated_metadata))

    def _get_io_product_specs(self):
        """
        Get input- and output-data product specifications from the
        parset-file, and do some sanity checks.
        """
        dps = self.parset.makeSubset(
            self.parset.fullModuleName('DataProducts') + '.'
        )
        # convert input dataproducts from parset value to DataMap
        self.input_data = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Input_Correlated.locations'),
                    dps.getStringVector('Input_Correlated.filenames'),
                    dps.getBoolVector('Input_Correlated.skip'))
        ])
        self.logger.debug("%d Input_Correlated data products specified" %
                          len(self.input_data))

        self.output_data = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Output_SkyImage.locations'),
                    dps.getStringVector('Output_SkyImage.filenames'),
                    dps.getBoolVector('Output_SkyImage.skip'))
        ])
        self.logger.debug("%d Output_SkyImage data products specified" %
                          len(self.output_data))

        self.output_correlated_data = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Output_Correlated.locations'),
                    dps.getStringVector('Output_Correlated.filenames'),
                    dps.getBoolVector('Output_Correlated.skip'))
        ])

        # assure that the two output maps contain the same skip fields
        align_data_maps( self.output_data, self.output_correlated_data)

        self.logger.debug("%d Output_Correlated data products specified" %
                          len(self.output_correlated_data))

        # # Sanity checks on input- and output data product specifications
        # if not validate_data_maps(self.input_data, self.output_data):
        #    raise PipelineException(
        #        "Validation of input/output data product specification failed!"
        #    )#Turned off untill DataMap is extended..

        # Target data is basically scratch data, consisting of one concatenated
        # MS per image. It must be stored on the same host as the final image.
        self.target_data = copy.deepcopy(self.output_data)

        for item in self.target_data:
            item.file = os.path.join(self.scratch_directory, 'concat.ms')


    @xml_node
    def _finalize(self, awimager_output_map, processed_ms_dir,
                  ms_per_image_map, sourcelist_map, minbaseline,
                  maxbaseline, target_mapfile,
                  output_image_mapfile, sourcedb_map, concat_ms_map_path,
                  output_correlated_mapfile, skip = False):
        """
        Perform the final step of the imager:
        Convert the output image to hdf5 and copy to output location
        Collect meta data and add to the image
        """

        placed_image_mapfile = self._write_datamap_to_file(None,
             "placed_image")
        self.logger.debug("Touched mapfile for correctly placed"
                        " hdf images: {0}".format(placed_image_mapfile))

        placed_correlated_mapfile = self._write_datamap_to_file(None,
             "placed_correlated")
        self.logger.debug("Touched mapfile for correctly placed"
                        " correlated datasets: {0}".format(placed_correlated_mapfile))

        if skip:
            return placed_image_mapfile, placed_correlated_mapfile
        else:
            # run the awimager recipe
            outputs = self.run_task("selfcal_finalize",
                target_mapfile, awimager_output_map = awimager_output_map,
                    ms_per_image_map = ms_per_image_map,
                    sourcelist_map = sourcelist_map,
                    sourcedb_map = sourcedb_map,
                    minbaseline = minbaseline,
                    maxbaseline = maxbaseline,
                    target_mapfile = target_mapfile,
                    output_image_mapfile = output_image_mapfile,
                    processed_ms_dir = processed_ms_dir,
                    placed_image_mapfile = placed_image_mapfile,
                    placed_correlated_mapfile = placed_correlated_mapfile,
                    concat_ms_map_path = concat_ms_map_path,
                    output_correlated_mapfile = output_correlated_mapfile
                    )

        return outputs["placed_image_mapfile"], \
                outputs["placed_correlated_mapfile"]

    @xml_node
    def _source_finding(self, image_map_path, major_cycle, skip = True):
        """
        Perform the sourcefinding step
        """
        # Create the parsets for the different sourcefinder runs
        bdsm_parset_pass_1 = self.parset.makeSubset("BDSM[0].")

        self._selfcal_modify_parset(bdsm_parset_pass_1, "pybdsm_first_pass.par")
        parset_path_pass_1 = self._write_parset_to_file(bdsm_parset_pass_1,
                "pybdsm_first_pass.par", "Sourcefinder first pass parset.")

        bdsm_parset_pass_2 = self.parset.makeSubset("BDSM[1].")
        self._selfcal_modify_parset(bdsm_parset_pass_2, "pybdsm_second_pass.par")
        parset_path_pass_2 = self._write_parset_to_file(bdsm_parset_pass_2,
                "pybdsm_second_pass.par", "sourcefinder second pass parset")

        # touch a mapfile to be filled with created sourcelists
        source_list_map = self._write_datamap_to_file(None,
             "source_finding_outputs",
             "map to sourcefinding outputs (sourcelist)")
        sourcedb_map_path = self._write_datamap_to_file(None,
             "source_dbs_outputs", "Map to sourcedbs based in found sources")

        # construct the location to save the output products of the
        # sourcefinder
        cycle_path = os.path.join(self.scratch_directory,
                                  "awimage_cycle_{0}".format(major_cycle))
        catalog_path = os.path.join(cycle_path, "bdsm_catalog")
        sourcedb_path = os.path.join(cycle_path, "bdsm_sourcedb")

        # Run the sourcefinder
        if skip:
            return source_list_map, sourcedb_map_path
        else:
            self.run_task("imager_source_finding",
                        image_map_path,
                        bdsm_parset_file_run1 = parset_path_pass_1,
                        bdsm_parset_file_run2x = parset_path_pass_2,
                        working_directory = self.scratch_directory,
                        catalog_output_path = catalog_path,
                        mapfile = source_list_map,
                        sourcedb_target_path = sourcedb_path,
                        sourcedb_map_path = sourcedb_map_path
                         )

            return source_list_map, sourcedb_map_path

    @xml_node
    def _bbs(self, concat_ms_map_path, timeslice_map_path, parmdbs_map_path, sourcedb_map_path,
              major_cycle, skip = False):
        """
        Perform a calibration step. First with a set of sources from the
        gsm and in later iterations also on the found sources
        """
        # create parset for bbs run
        parset = self.parset.makeSubset("BBS.")
        self._selfcal_modify_parset(parset, "bbs")
        parset_path = self._write_parset_to_file(parset, "bbs",
                        "Parset for calibration with a local sky model")

        # create the output file path
        output_mapfile = self._write_datamap_to_file(None, "bbs_output",
                        "Mapfile with calibrated measurement sets.")

        converted_sourcedb_map_path = self._write_datamap_to_file(None,
                "source_db", "correctly shaped mapfile for input sourcedbs")

        if skip:
            return output_mapfile

        # The create db step produces a mapfile with a single sourcelist for
        # the different timeslices. Generate a mapfile with copies of the
        # sourcelist location: This allows validation of maps in combination
        # get the original map data
        sourcedb_map = DataMap.load(sourcedb_map_path)
        parmdbs_map = MultiDataMap.load(parmdbs_map_path)
        converted_sourcedb_map = []

        # sanity check for correcy output from previous recipes
        if not validate_data_maps(sourcedb_map, parmdbs_map):
            self.logger.error("The input files for bbs do not contain "
                                "matching host names for each entry content:")
            self.logger.error(repr(sourcedb_map))
            self.logger.error(repr(parmdbs_map))
            raise PipelineException("Invalid input data for imager_bbs recipe")

        self.run_task("selfcal_bbs",
                      timeslice_map_path,
                      parset = parset_path,
                      instrument_mapfile = parmdbs_map_path,
                      sourcedb_mapfile = sourcedb_map_path,
                      mapfile = output_mapfile,
                      working_directory = self.scratch_directory,
                      concat_ms_map_path=concat_ms_map_path,
                      major_cycle=major_cycle)

        return output_mapfile

    @xml_node
    def _aw_imager(self, prepare_phase_output, major_cycle, sky_path,
                  number_of_major_cycles,   skip = False):
        """
        Create an image based on the calibrated, filtered and combined data.
        """
        # Create parset for the awimage recipe
        parset = self.parset.makeSubset("AWimager.")
        # Get maxbaseline from 'full' parset
        max_baseline = self.parset.getInt("Imaging.maxbaseline")
        patch_dictionary = {"maxbaseline": str(
                                    max_baseline)}
        try:
            temp_parset_filename = patch_parset(parset, patch_dictionary)
            aw_image_parset = get_parset(temp_parset_filename)
            aw_image_parset_path = self._write_parset_to_file(aw_image_parset,
                "awimager_cycle_{0}".format(major_cycle),
                "Awimager recipe parset")
        finally:
            # remove tempfile
            os.remove(temp_parset_filename)

        # Create path to write the awimage files
        intermediate_image_path = os.path.join(self.scratch_directory,
            "awimage_cycle_{0}".format(major_cycle), "image")

        output_mapfile = self._write_datamap_to_file(None, "awimager",
                                    "output map for awimager recipe")

        mask_patch_size = self.parset.getInt("Imaging.mask_patch_size")
        autogenerate_parameters = self.parset.getBool(
                                    "Imaging.auto_imaging_specs")
        specify_fov = self.parset.getBool(
                                    "Imaging.specify_fov")
        if skip:
            pass
        else:
            # run the awimager recipe
            self.run_task("selfcal_awimager", prepare_phase_output,
                          parset = aw_image_parset_path,
                          mapfile = output_mapfile,
                          output_image = intermediate_image_path,
                          mask_patch_size = mask_patch_size,
                          sourcedb_path = sky_path,
                          working_directory = self.scratch_directory,
                          autogenerate_parameters = autogenerate_parameters,
                          specify_fov = specify_fov, major_cycle = major_cycle,
                          nr_cycles = number_of_major_cycles,
                          perform_self_cal = True)

        return output_mapfile, max_baseline

    @xml_node
    def _prepare_phase(self, input_ms_map_path, target_mapfile,
        add_beam_tables):
        """
        Copy ms to correct location, combine the ms in slices and combine
        the time slices into a large virtual measurement set
        """
        # Create the dir where found and processed ms are placed
        # ms_per_image_map_path contains all the original ms locations:
        # this list contains possible missing files
        processed_ms_dir = os.path.join(self.scratch_directory, "subbands")

        # get the parameters, create a subset for ndppp, save
        # Aditional parameters are added runtime on the node, based on data
        ndppp_parset = self.parset.makeSubset("DPPP.")
        ndppp_parset_path = self._write_parset_to_file(ndppp_parset,
                    "prepare_imager_ndppp", "parset for ndpp recipe")

        # create the output file paths
        # [1] output -> prepare_output
        output_mapfile = self._write_datamap_to_file(None, "prepare_output")
        time_slices_mapfile = self._write_datamap_to_file(None,
                                                    "prepare_time_slices")
        ms_per_image_mapfile = self._write_datamap_to_file(None,
                                                         "ms_per_image")

        # get some parameters from the imaging pipeline parset:
        slices_per_image = self.parset.getInt("Imaging.slices_per_image")
        subbands_per_image = self.parset.getInt("Imaging.subbands_per_image")

        outputs = self.run_task("imager_prepare", input_ms_map_path,
                parset = ndppp_parset_path,
                target_mapfile = target_mapfile,
                slices_per_image = slices_per_image,
                subbands_per_image = subbands_per_image,
                mapfile = output_mapfile,
                slices_mapfile = time_slices_mapfile,
                ms_per_image_mapfile = ms_per_image_mapfile,
                working_directory = self.scratch_directory,
                processed_ms_dir = processed_ms_dir,
                add_beam_tables = add_beam_tables,
                do_rficonsole = False)

        # validate that the prepare phase produced the correct data
        output_keys = outputs.keys()
        if not ('mapfile' in output_keys):
            error_msg = "The imager_prepare master script did not"\
                    "return correct data. missing: {0}".format('mapfile')
            self.logger.error(error_msg)
            raise PipelineException(error_msg)
        if not ('slices_mapfile' in output_keys):
            error_msg = "The imager_prepare master script did not"\
                    "return correct data. missing: {0}".format(
                                                        'slices_mapfile')
            self.logger.error(error_msg)
            raise PipelineException(error_msg)

        if not ('ms_per_image_mapfile' in output_keys):
            error_msg = "The imager_prepare master script did not"\
                    "return correct data. missing: {0}".format(
                                                'ms_per_image_mapfile')
            self.logger.error(error_msg)
            raise PipelineException(error_msg)

        # Return the mapfiles paths with processed data
        return output_mapfile, outputs["slices_mapfile"], ms_per_image_mapfile, \
            processed_ms_dir

    @xml_node
    def _create_dbs(self, input_map_path, timeslice_map_path, 
                    major_cycle, source_list_map_path , 
                    skip_create_dbs = False):
        """
        Create for each of the concatenated input measurement sets
        an instrument model and parmdb
        """
        # Create the parameters set
        parset = self.parset.makeSubset("GSM.")

        # create the files that will contain the output of the recipe
        parmdbs_map_path = self._write_datamap_to_file(None, "parmdbs",
                    "parmdbs output mapfile")
        sourcedb_map_path = self._write_datamap_to_file(None, "sky_files",
                    "source db output mapfile")

        # run the master script
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
                        sourcedb_suffix = ".sourcedb",
                        slice_paths_mapfile = timeslice_map_path,
                        parmdb_suffix = ".parmdb",
                        parmdbs_map_path = parmdbs_map_path,
                        sourcedb_map_path = sourcedb_map_path,
                        source_list_map_path = source_list_map_path,
                        working_directory = self.scratch_directory,
                        major_cycle = major_cycle)

        return parmdbs_map_path, sourcedb_map_path

    # TODO: Move these helpers to the parent class
    def _write_parset_to_file(self, parset, parset_name, message):
        """
        Write the suplied the suplied parameterset to the parameter set
        directory in the jobs dir with the filename suplied in parset_name.
        Return the full path to the created file.
        """
        parset_dir = os.path.join(
            self.config.get("layout", "job_directory"), "parsets")
        # create the parset dir if it does not exist
        create_directory(parset_dir)

        # write the content to a new parset file
        parset_path = os.path.join(parset_dir,
                         "{0}.parset".format(parset_name))
        parset.writeFile(parset_path)

        # display a debug log entrie with path and message
        self.logger.debug("Wrote parset to path <{0}> : {1}".format(
                               parset_path, message))

        return parset_path

    def _write_datamap_to_file(self, datamap, mapfile_name, message = ""):
        """
        Write the suplied the suplied map to the mapfile.
        directory in the jobs dir with the filename suplied in mapfile_name.
        Return the full path to the created file.
        If suplied data is None then the file is touched if not existing, but
        existing files are kept as is
        """

        mapfile_dir = os.path.join(
            self.config.get("layout", "job_directory"), "mapfiles")
        # create the mapfile_dir if it does not exist
        create_directory(mapfile_dir)

        # write the content to a new parset file
        mapfile_path = os.path.join(mapfile_dir,
                         "{0}.map".format(mapfile_name))

        # display a debug log entrie with path and message
        if datamap != None:
            datamap.save(mapfile_path)

            self.logger.debug(
            "Wrote mapfile <{0}>: {1}".format(mapfile_path, message))
        else:
            if not os.path.exists(mapfile_path):
                DataMap().save(mapfile_path)

                self.logger.debug(
                    "Touched mapfile <{0}>: {1}".format(mapfile_path, message))

        return mapfile_path


    # This functionality should be moved outside into MOM/ default template.
    # This is now a static we should be able to control this.
    def _selfcal_modify_parset(self, parset, parset_name):    
        """ 
        Modification of the BBS parset for selfcal implementation, add, 
        remove, modify some values in bbs parset, done by 
        done by Nicolas Vilchez
        """            
        
        if parset_name == "bbs":			
        
             parset.replace('Step.solve.Model.Beam.UseChannelFreq', 'True')
             parset.replace('Step.solve.Model.Ionosphere.Enable', 'F')
             parset.replace('Step.solve.Model.TEC.Enable', 'F')
             parset.replace('Step.correct.Model.Beam.UseChannelFreq', 'True')
             parset.replace('Step.correct.Model.TEC.Enable', 'F')
             parset.replace('Step.correct.Model.Phasors.Enable', 'T')
             parset.replace('Step.correct.Output.WriteCovariance', 'T')             
                         
             #must be erased, by default I replace to the default value
             parset.replace('Step.solve.Baselines', '*&')
             
             parset.replace('Step.solve.Solve.Mode', 'COMPLEX')
             parset.replace('Step.solve.Solve.CellChunkSize', '100')                             
             parset.replace('Step.solve.Solve.PropagateSolutions', 'F')                    
             parset.replace('Step.solve.Solve.Options.MaxIter', '100')  
                   

        if parset_name == "pybdsm_first_pass.par":
             
             parset.replace('advanced_opts', 'True')
             parset.replace('atrous_do', 'True')
             parset.replace('rms_box', '(80.0,15.0)')
             parset.replace('thresh_isl', '5')
             parset.replace('thresh_pix', '5')
             parset.replace('adaptive_rms_box', 'True')
             parset.replace('blank_limit', '1E-4')
             parset.replace('ini_method', 'curvature')
             parset.replace('atrous_do', 'True')
             parset.replace('thresh', 'hard')              
             

        if parset_name == "pybdsm_second_pass.par":
             
             parset.replace('advanced_opts', 'True')
             parset.replace('atrous_do', 'True')
             parset.replace('rms_box', '(80.0,15.0)')
             parset.replace('thresh_isl', '5')
             parset.replace('thresh_pix', '5')
             parset.replace('adaptive_rms_box', 'True')
             parset.replace('blank_limit', '1E-4')
             parset.replace('ini_method', 'curvature')
             parset.replace('atrous_do', 'True')
             parset.replace('thresh', 'hard')              


if __name__ == '__main__':
    sys.exit(selfcal_imager_pipeline().main())
