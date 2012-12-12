#!/usr/bin/env python
#                                                        LOFAR IMAGING PIPELINE
#
#                                                        Imager Pipeline recipe
#                                                            Marcel Loose, 2012
#                                                               loose@astron.nl
#                                                            Wouter Klijn, 2012
#                                                               klijn@astron.nl
# -----------------------------------------------------------------------------
import os
import sys
import copy

from lofarpipe.support.control import control
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.data_map import DataMap, validate_data_maps, MultiDataMap
from lofarpipe.support.utilities import patch_parset, get_parset
from lofarpipe.support.loggingdecorators import xml_node, mail_log_on_exception

from lofar.parameterset import parameterset


class msss_imager_pipeline(control):
    """
    The Automatic MSSS imager pipeline is used to generate MSSS images and find
    sources in the generated images. Generated images and lists of found
    sources are complemented with meta data and thus ready for consumption by
    the Long Term Storage (LTA)

    *subband groups*
    The imager_pipeline is able to generate images on the frequency range of
    LOFAR in parallel. Combining the frequency subbands together in so called
    subbandgroups. Each subband group will result in an image and sourcelist,
    (typically 8, because ten subband groups are combined).

    *Time Slices*
    MSSS images are compiled from a number of so-called (time) slices. Each
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
        return super(msss_imager_pipeline, self).go()

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
        #storedata_map(input_mapfile, self.input_data)
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

        # ******************************************************************
        # (1) prepare phase: copy and collect the ms
        concat_ms_map_path, timeslice_map_path, raw_ms_per_image_map_path, \
            processed_ms_dir = self._prepare_phase(input_mapfile,
                                    target_mapfile)

        #We start with an empty source_list
        source_list = ""  # path to local sky model (list of 'found' sources)
        number_of_major_cycles = self.parset.getInt(
                                    "Imaging.number_of_major_cycles")
        for idx_loop in range(number_of_major_cycles):
            # *****************************************************************
            # (2) Create dbs and sky model
            parmdbs_path, sourcedb_map_path = self._create_dbs(
                        concat_ms_map_path, timeslice_map_path,
                        source_list=source_list,
                        skip_create_dbs=False)

            # *****************************************************************
            # (3)  bbs_imager recipe.
            bbs_output = self._bbs(timeslice_map_path, parmdbs_path,
                        sourcedb_map_path, skip=False)

            # TODO: Extra recipe: concat timeslices using pyrap.concatms 
            # (see prepare)

            # *****************************************************************
            # (4) Get parameters awimager from the prepare_parset and inputs
            aw_image_mapfile, maxbaseline = self._aw_imager(concat_ms_map_path,
                        idx_loop, sourcedb_map_path,
                        skip=False)

            # *****************************************************************
            # (5) Source finding
            sourcelist_map, found_sourcedb_path = self._source_finding(
                    aw_image_mapfile, idx_loop, skip=False)
            #should the output be a sourcedb? instead of a sourcelist

        # TODO: minbaseline should be a parset value as is maxbaseline..
        minbaseline = 0

        # *********************************************************************
        # (6) Finalize:
        placed_data_image_map = self._finalize(aw_image_mapfile,
            processed_ms_dir, raw_ms_per_image_map_path, sourcelist_map,
            minbaseline, maxbaseline, target_mapfile, output_image_mapfile,
            found_sourcedb_path)

        # *********************************************************************
        # (7) Get metadata
        # Create a parset-file containing the metadata for MAC/SAS
        self.run_task("get_metadata", placed_data_image_map,
            parset_file=self.parset_feedback_file,
            parset_prefix=(
                full_parset.getString('prefix') +
                full_parset.fullModuleName('DataProducts')
            ),
            product_type="SkyImage")

        return 0

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

        ## Sanity checks on input- and output data product specifications
        #if not validate_data_maps(self.input_data, self.output_data):
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
                  raw_ms_per_image_map, sourcelist_map, minbaseline,
                  maxbaseline, target_mapfile,
                  output_image_mapfile, sourcedb_map, skip=False):
        """
        Perform the final step of the imager:
        Convert the output image to hdf5 and copy to output location
        Collect meta data and add to the image
        """

        placed_image_mapfile = self._write_datamap_to_file(None,
             "placed_image")
        self.logger.debug("Touched mapfile for correctly placed"
                        " hdf images: {0}".format(placed_image_mapfile))

        if skip:
            return placed_image_mapfile
        else:
            #run the awimager recipe
            placed_image_mapfile = self.run_task("imager_finalize",
                target_mapfile, awimager_output_map=awimager_output_map,
                    raw_ms_per_image_map=raw_ms_per_image_map,
                    sourcelist_map=sourcelist_map,
                    sourcedb_map=sourcedb_map,
                    minbaseline=minbaseline,
                    maxbaseline=maxbaseline,
                    target_mapfile=target_mapfile,
                    output_image_mapfile=output_image_mapfile,
                    processed_ms_dir=processed_ms_dir,
                    placed_image_mapfile=placed_image_mapfile
                    )["placed_image_mapfile"]

        return placed_image_mapfile

    @xml_node
    def _source_finding(self, image_map_path, major_cycle, skip=True):
        """
        Perform the sourcefinding step
        """
        # Create the parsets for the different sourcefinder runs
        bdsm_parset_pass_1 = self.parset.makeSubset("BDSM[0].")
        parset_path_pass_1 = self._write_parset_to_file(bdsm_parset_pass_1,
                "pybdsm_first_pass.par", "Sourcefinder first pass parset.")

        bdsm_parset_pass_2 = self.parset.makeSubset("BDSM[1].")
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
                        bdsm_parset_file_run1=parset_path_pass_1,
                        bdsm_parset_file_run2x=parset_path_pass_2,
                        working_directory=self.scratch_directory,
                        catalog_output_path=catalog_path,
                        mapfile=source_list_map,
                        sourcedb_target_path=sourcedb_path,
                        sourcedb_map_path=sourcedb_map_path
                         )

            return source_list_map, sourcedb_map_path

    @xml_node
    def _bbs(self, timeslice_map_path, parmdbs_map_path, sourcedb_map_path,
              skip=False):
        """
        Perform a calibration step. First with a set of sources from the
        gsm and in later iterations also on the found sources
        """
        #create parset for bbs run
        parset = self.parset.makeSubset("BBS.")
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
        self.logger.debug("debug 1")
        # get the original map data
        sourcedb_map = DataMap.load(sourcedb_map_path)
        parmdbs_map = MultiDataMap.load(parmdbs_map_path)
        converted_sourcedb_map = []
        self.logger.debug("debug 2")
        # sanity check for correcy output from previous recipes
        if not validate_data_maps(sourcedb_map, parmdbs_map):
            self.logger.error("The input files for bbs do not contain "
                                "matching host names for each entry content:")
            self.logger.error(repr(sourcedb_map))
            self.logger.error(repr(parmdbs_map))
            raise PipelineException("Invalid input data for imager_bbs recipe")

        self.logger.debug("debug 3")
        self.run_task("imager_bbs",
                      timeslice_map_path,
                      parset=parset_path,
                      instrument_mapfile=parmdbs_map_path,
                      sourcedb_mapfile=sourcedb_map_path,
                      mapfile=output_mapfile,
                      working_directory=self.scratch_directory)

        return output_mapfile

    @xml_node
    def _aw_imager(self, prepare_phase_output, major_cycle, sky_path,
                   skip=False):
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

        if skip:
            pass
        else:
            #run the awimager recipe
            self.run_task("imager_awimager", prepare_phase_output,
                          parset=aw_image_parset_path,
                          mapfile=output_mapfile,
                          output_image=intermediate_image_path,
                          mask_patch_size=mask_patch_size,
                          sourcedb_path=sky_path,
                          working_directory=self.scratch_directory)

        return output_mapfile, max_baseline

    @xml_node
    def _prepare_phase(self, input_ms_map_path, target_mapfile):
        """
        Copy ms to correct location, combine the ms in slices and combine
        the time slices into a large virtual measurement set
        """
        # Create the dir where found and processed ms are placed
        # raw_ms_per_image_map_path contains all the original ms locations:
        # this list contains possible missing files
        processed_ms_dir = os.path.join(self.scratch_directory, "subbands")

        # get the parameters, create a subset for ndppp, save
        ndppp_parset = self.parset.makeSubset("DPPP.")
        ndppp_parset_path = self._write_parset_to_file(ndppp_parset,
                    "prepare_imager_ndppp", "parset for ndpp recipe")

        # create the output file paths
        #[1] output -> prepare_output
        output_mapfile = self._write_datamap_to_file(None, "prepare_output")
        time_slices_mapfile = self._write_datamap_to_file(None,
                                                    "prepare_time_slices")
        raw_ms_per_image_mapfile = self._write_datamap_to_file(None,
                                                         "raw_ms_per_image")

        # get some parameters from the imaging pipeline parset:
        slices_per_image = self.parset.getInt("Imaging.slices_per_image")
        subbands_per_image = self.parset.getInt("Imaging.subbands_per_image")

        outputs = self.run_task("imager_prepare", input_ms_map_path,
                parset=ndppp_parset_path,
                target_mapfile=target_mapfile,
                slices_per_image=slices_per_image,
                subbands_per_image=subbands_per_image,
                mapfile=output_mapfile,
                slices_mapfile=time_slices_mapfile,
                raw_ms_per_image_mapfile=raw_ms_per_image_mapfile,
                working_directory=self.scratch_directory,
                processed_ms_dir=processed_ms_dir)

        #validate that the prepare phase produced the correct data
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
        if not ('raw_ms_per_image_mapfile' in output_keys):
            error_msg = "The imager_prepare master script did not"\
                    "return correct data. missing: {0}".format(
                                                'raw_ms_per_image_mapfile')
            self.logger.error(error_msg)
            raise PipelineException(error_msg)

        # Return the mapfiles paths with processed data
        return output_mapfile, outputs["slices_mapfile"], raw_ms_per_image_mapfile, \
            processed_ms_dir

    @xml_node
    def _create_dbs(self, input_map_path, timeslice_map_path, source_list="",
                    skip_create_dbs=False):
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

        #run the master script
        if skip_create_dbs:
            pass
        else:
            self.run_task("imager_create_dbs", input_map_path,
                        monetdb_hostname=parset.getString("monetdb_hostname"),
                        monetdb_port=parset.getInt("monetdb_port"),
                        monetdb_name=parset.getString("monetdb_name"),
                        monetdb_user=parset.getString("monetdb_user"),
                        monetdb_password=parset.getString("monetdb_password"),
                        assoc_theta=parset.getString("assoc_theta"),
                        sourcedb_suffix=".sourcedb",
                        slice_paths_mapfile=timeslice_map_path,
                        parmdb_suffix=".parmdb",
                        parmdbs_map_path=parmdbs_map_path,
                        sourcedb_map_path=sourcedb_map_path,
                        source_list_path=source_list,
                        working_directory=self.scratch_directory)

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
        #create the parset dir if it does not exist
        create_directory(parset_dir)

        #write the content to a new parset file
        parset_path = os.path.join(parset_dir,
                         "{0}.parset".format(parset_name))
        parset.writeFile(parset_path)

        #display a debug log entrie with path and message
        self.logger.debug("Wrote parset to path <{0}> : {1}".format(
                               parset_path, message))

        return parset_path

    def _write_datamap_to_file(self, datamap, mapfile_name, message=""):
        """
        Write the suplied the suplied map to the mapfile.
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

        #display a debug log entrie with path and message
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


if __name__ == '__main__':
    sys.exit(msss_imager_pipeline().main())
