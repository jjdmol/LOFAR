#!/usr/bin/env python
#                                                         LOFAR IMAGING PIPELINE
#
#                                                         Imager Pipeline recipe
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys
import shutil

from lofarpipe.support.control import control
from lofar.parameterset import parameterset #@UnresolvedImport
from lofarpipe.support.utilities import create_directory
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.group_data import load_data_map, store_data_map
from lofarpipe.support.group_data import validate_data_maps
from lofarpipe.support.utilities import patch_parset


class msss_imager_pipeline(control):
    """    
    The Automatic MSSS imager pipeline is used to generate MSSS images and find
    sources in the generated images. Generated images and lists of found sources
    are complemented with meta data and thus ready for consumption by the 
    Long Term Storage (LTA)
    
    *subband groups*
    The imager_pipeline is able to generate images on the frequency range of
    LOFAR in parallel. Combining the frequency subbands together in so called 
    subbandgroups. Each subband group will result in an image and sourcelist, 
    (typically 8, because ten subband groups are combined). 
     
    *Time Slices*  
    MSSS images are compiled from a number of so-called (time) slices. Each slice
    comprises a short (approx. 10 min) observation of a field (an area on the
    sky) containing typically 80 subbands. The number of slices will be
    different for LBA observations (typically 9) and HBA observations
    (typically 2), due to differences in sensitivity.
    
    Each image will be compiled on a different cluster node to balance the
    processing load. The input- and output- files and locations are determined
    by the scheduler and specified in the parset-file.
    
    *steps*
    This pipeline performs the following operations:
    
    1. Prepare Phase: Copy the preprocessed MS's from the different compute
       nodes to the nodes where the images will be compiled (the prepare phase).
       Combine the subbands in subband groups, concattenate the timeslice in a
       single large measurement set and perform flagging, RFI and bad station
       exclusion.
    2. Create db: Generate a local sky model (LSM) from the global sky model
       (GSM) for the sources that are in the field-of-view (FoV). The LSM
       is stored as sourcedb.
       In step 3 calibration of the measurement sets is performed on these 
       sources and in step 4 to create a mask for the awimager. The calibration 
       solution will be placed in an instrument table/db also created in this
       step.
    3. BBS: Calibrate the measurement set with the sourcedb from the gsm.
       In later iterations sourced found in the created images will be added
       to this list. Resulting in a selfcalibration cycle.       
    4. Awimager: The combined  measurement sets are now imaged. The imaging 
       is performed using a mask: The sources in the sourcedb are used to create
       an casa image masking known sources. Together with the measurement set
       an image is created. 
    5. Sourcefinding: The images created in step 4 are fed to pyBDSM to find and
       describe sources. In multiple itterations substracting the found sources,
       all sources are collectedin a sourcelist.       
    I. The sources found in step 5 are fed back into step 2. This allows the 
       Measurement sets to be calibrated with sources currently found in the
       image. This loop will continue until convergence (3 times for the time
        being).  
    6. Finalize: Meta data with regards to the input, computations performed and
       results are collected an added to the casa image. The images created are
       converted from casa to HDF5 and copied to the correct output location. 
    7. Export meta data: An outputfile with meta data is generated ready for
       consumption by the LTA and/or the LOFAR framework
    
    Per subband-group, the following output products will be delivered:      
    a. An image
    b. A source list
    c. (Calibration solutions and corrected visibilities)

    """
    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = []
        self.target_data = []
        self.output_data = []
        self.scratch_directory = None
        self.parset_feedback_file = None


    def usage(self):
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
        if not self.inputs.has_key('job_name'):
            self.inputs['job_name'] = (
                os.path.splitext(os.path.basename(parset_file))[0]
            )
        return super(msss_imager_pipeline, self).go()


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
            "Wrote input UV-data mapfile: {0}".format(input_mapfile))

        target_mapfile = os.path.join(mapfile_dir, "target.mapfile")
        store_data_map(target_mapfile, self.target_data)
        self.logger.debug(
            "Wrote target mapfile: {0}".format(target_mapfile))

        output_image_mapfile = os.path.join(mapfile_dir, "images.mapfile")
        store_data_map(output_image_mapfile, self.output_data)
        self.logger.debug(
            "Wrote output sky-image mapfile: {0}".format(output_image_mapfile))

        # reset the parset to a 'parset' subset containing only leafs without 
        # prepending node names
        full_parset = self.parset
        self.parset = self.parset.makeSubset(
            self.parset.fullModuleName('PythonControl') + '.'
        )

        # Create the dir where found and processed ms are placed
        # raw_ms_per_image_map_path contains all the original ms locations:
        # this list contains possible missing files
        processed_ms_dir = os.path.join(self.scratch_directory, "subbands")
        concat_ms_map_path, timeslice_map_path, raw_ms_per_image_map_path = \
            self._prepare_phase(input_mapfile, target_mapfile, processed_ms_dir,
                                 skip=False)

        #We start with an empty source_list
        source_list = ""  #This variable contains possible 'new' star locations from 
        # found in the pipeline or external to use in the calibration and imaging
        # filled at least at the end of the major cycle.

        number_of_major_cycles = self.parset.getInt(
            "Imaging.number_of_major_cycles"
        )
        for idx_loop in range(number_of_major_cycles):
            # ******************************************************************
            # (2) Create dbs and sky model
            parmdbs_path, sourcedb_map_path = self._create_dbs(
                        concat_ms_map_path, timeslice_map_path,
                        source_list=source_list,
                        skip_create_dbs=False)

            # *****************************************************************
            # (3)  bbs_imager recipe.
            bbs_output = self._bbs(timeslice_map_path, parmdbs_path, sourcedb_map_path,
                        skip=False)


            # ******************************************************************
            # (4) Get parameters awimager from the prepare_parset and inputs
            aw_image_mapfile, maxbaseline = self._aw_imager(concat_ms_map_path,
                        idx_loop, sourcedb_map_path,
                        skip=False)

            # *****************************************************************
            # (5) Source finding 
            sourcelist_map, found_sourcedb_path = self._source_finding(aw_image_mapfile,
                                    idx_loop, skip=False)
            #should the output be a sourcedb? instead of a sourcelist


        # The output does not contain the intermediate values
        #
        minbaseline = 0

        # *********************************************************************
        # (6) Finalize:
        placed_data_image_map = self._finalize(aw_image_mapfile, processed_ms_dir,
                       raw_ms_per_image_map_path, found_sourcedb_path,
                       minbaseline, maxbaseline, target_mapfile,
                       output_image_mapfile)

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
        odp = self.parset.makeSubset(
            self.parset.fullModuleName('DataProducts') + '.'
        )
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
                  output_image_mapfile, skip=False):

        placed_image_mapfile = self._write_datamap_to_file(None,
             "placed_image")
        self.logger.debug("Touched mapfile for correctly placed"
                        " hdf images: {0}".format(placed_image_mapfile))

        if skip:
            return placed_image_mapfile
        else:
            #run the awimager recipe
            placed_image_mapfile = self.run_task("imager_finalize", target_mapfile,
                    awimager_output_map=awimager_output_map,
                    raw_ms_per_image_map=raw_ms_per_image_map,
                    sourcelist_map=sourcelist_map,
                    minbaseline=minbaseline,
                    maxbaseline=maxbaseline,
                    target_mapfile=target_mapfile,
                    output_image_mapfile=output_image_mapfile,
                    processed_ms_dir=processed_ms_dir,
                    placed_image_mapfile=placed_image_mapfile)["placed_image_mapfile"]

        return placed_image_mapfile

    def _source_finding(self, image_map_path, major_cycle, skip=True):
        bdsm_parset_pass_1 = self.parset.makeSubset("BDSM[0].")
        parset_path_pass_1 = self._write_parset_to_file(bdsm_parset_pass_1,
                                                 "pybdsm_first_pass.par")
        self.logger.debug("Wrote sourcefinder first pass parset: {0}".format(
                                                        parset_path_pass_1))
        bdsm_parset_pass_2 = self.parset.makeSubset("BDSM[1].")
        parset_path_pass_2 = self._write_parset_to_file(bdsm_parset_pass_2,
                                             "pybdsm_second_pass.par")
        self.logger.debug("Wrote sourcefinder second pass parset: {0}".format(
                                                        parset_path_pass_2))
        # touch a mapfile to be filled with created sourcelists
        source_list_map = self._write_datamap_to_file(None,
             "source_finding_outputs")
        self.logger.debug("Touched mapfile for sourcefinding output: {0}".format(
                                                        source_list_map))
        sourcedb_map_path = self._write_datamap_to_file(None,
             "source_dbs_outputs")
        self.logger.debug("Touched mapfile for sourcedb based in found sources: {0}".format(
                                                        sourcedb_map_path))
        catalog_path = os.path.join(
            self.scratch_directory,
            "awimage_cycle_{0}".format(major_cycle),
            "bdsm_catalog"
        )
        sourcedb_path = os.path.join(
            self.scratch_directory,
            "awimage_cycle_{0}".format(major_cycle),
            "bdsm_sourcedb"
        )
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


    def _bbs(self, timeslice_map_path, parmdbs_map_path, sourcedb_map_path, skip=False):
        #create parset for recipe
        parset = self.parset.makeSubset("BBS.")
        parset_path = self._write_parset_to_file(parset, "bbs")
        self.logger.debug(
            "Wrote parset for bbs: {0}".format(parset_path))
        # create the output file path
        output_mapfile = self._write_datamap_to_file(None, "bbs_output")
        self.logger.debug(
            "Touched mapfile for bbs output: {0}".format(output_mapfile))
        converted_sourcedb_map_path = self._write_datamap_to_file(None, "parmdb")
        self.logger.debug(
            "Touched correctly shaped mapfile for input sourcedbs : {0}".format(
                                converted_sourcedb_map_path))

        if skip:
            return output_mapfile

        # The source_db_pair map contains a single source_db_pair file while imager_bbs expects a source_db_pair
        # file for each 'pardbm ms set combination'. 
        sourcedb_map = load_data_map(sourcedb_map_path)
        parmdbs_map = load_data_map(parmdbs_map_path)
        converted_sourcedb_map = []
        for (source_db_pair, parmdbs) in zip(sourcedb_map, parmdbs_map):
            (host_sourcedb, sourcedb_path) = source_db_pair
            (host_parmdbs, parmdbs_entries) = parmdbs
            # sanity check: host should be the same
            if host_parmdbs != host_sourcedb:
                self.logger.error("The input files for bbs do not contain "
                                  "matching host names for each entry")
                self.logger.error(repr(sourcedb_map))
                self.logger.error(repr(parmdbs_map_path))

            #add the entries but with skymap multiplied with len (parmds list)
            converted_sourcedb_map.append((host_sourcedb, [sourcedb_path] * len(parmdbs_entries)))
        #save the new mapfile
        store_data_map(converted_sourcedb_map_path, converted_sourcedb_map)
        self.logger.debug("Wrote converted sourcedb datamap with: {0}".format(
                                      converted_sourcedb_map_path))

        self.run_task("imager_bbs",
                      timeslice_map_path,
                      parset=parset_path,
                      instrument_mapfile=parmdbs_map_path,
                      sourcedb_mapfile=converted_sourcedb_map_path,
                      mapfile=output_mapfile,
                      working_directory=self.scratch_directory)

        return output_mapfile

    def _aw_imager(self, prepare_phase_output, major_cycle, sky_path, skip=False):
        """
        
        """
        parset = self.parset.makeSubset("AWimager.")

        #add the baseline parameter from the head parset node: TODO: pass as parameter
        patch_dictionary = {"maxbaseline":str(self.parset.getInt("Imaging.maxbaseline"))}
        temp_parset_filename = patch_parset(parset, patch_dictionary)
        # Now create the correct parset path
        parset_path = os.path.join(
            self.config.get("layout", "job_directory"), "parsets",
            "awimager_cycle_{0}".format(major_cycle))
        # copy
        shutil.copy(temp_parset_filename, parset_path)
        os.unlink(temp_parset_filename) # delete old file

        image_path = os.path.join(
            self.scratch_directory, "awimage_cycle_{0}".format(major_cycle),
                "image")

        output_mapfile = self._write_datamap_to_file(None, "awimager")
        self.logger.debug("Touched output map for awimager recipe: {0}".format(
                                      output_mapfile))


        mask_patch_size = self.parset.getInt("Imaging.mask_patch_size")
        if skip:
            pass
        else:
            #run the awimager recipe
            self.run_task("imager_awimager", prepare_phase_output,
                          parset=parset_path,
                          mapfile=output_mapfile,
                          output_image=image_path,
                          mask_patch_size=mask_patch_size,
                          sourcedb_path=sky_path,
                          working_directory=self.scratch_directory)


        return output_mapfile, self.parset.getInt("Imaging.maxbaseline")


    def _prepare_phase(self, input_ms_map_path, target_mapfile, processed_ms_dir,
                        skip=False):
        # get the parameters, create a subset for ndppp, save
        ndppp_parset = self.parset.makeSubset("DPPP.")
        ndppp_parset_path = self._write_parset_to_file(ndppp_parset,
                                                       "prepare_imager_ndppp")
        self.logger.debug(
            "Wrote parset for ndpp: {0}".format(ndppp_parset_path))
        # create the output file paths
        #[1] output -> prepare_output
        output_mapfile = self._write_datamap_to_file(None, "prepare_output")
        time_slices_mapfile = self._write_datamap_to_file(None,
                                                          "prepare_time_slices")
        raw_ms_per_image_mapfile = self._write_datamap_to_file(None,
                                                         "raw_ms_per_image")
        self.logger.debug(
            "Touched the following map files used for output: {0}, {1}, {2}".format(
                output_mapfile, time_slices_mapfile, raw_ms_per_image_mapfile))
        # Run the prepare phase script 
        if skip:
            pass
        else:
            outputs = self.run_task("imager_prepare", input_ms_map_path,
                    parset=ndppp_parset_path,
                    target_mapfile=target_mapfile,
                    slices_per_image=self.parset.getInt(
                        "Imaging.slices_per_image"
                    ),
                    subbands_per_image=self.parset.getInt(
                        "Imaging.subbands_per_image"
                    ),
                    mapfile=output_mapfile,
                    slices_mapfile=time_slices_mapfile,
                    raw_ms_per_image_mapfile=raw_ms_per_image_mapfile,
                    working_directory=self.scratch_directory,
                    processed_ms_dir=processed_ms_dir)

            #validate that the prepare phase produced the correct data
            output_keys = outputs.keys()
            if not ('mapfile' in output_keys):
                error_msg = "The imager_prepare master script did not"\
                        "return correct data missing: {0}".format('mapfile')
                self.logger.error(error_msg)
                raise PipelineException(error_msg)
            if not ('slices_mapfile' in output_keys):
                error_msg = "The imager_prepare master script did not"\
                        "return correct data missing: {0}".format(
                                                            'slices_mapfile')
                self.logger.error(error_msg)
                raise PipelineException(error_msg)
            if not ('raw_ms_per_image_mapfile' in output_keys):
                error_msg = "The imager_prepare master script did not"\
                        "return correct data missing: {0}".format(
                                                    'raw_ms_per_image_mapfile')
                self.logger.error(error_msg)
                raise PipelineException(error_msg)

        # Return the mapfiles paths with processed data
        return output_mapfile, time_slices_mapfile, raw_ms_per_image_mapfile


    def _create_dbs(self, input_map_path, timeslice_map_path, source_list="",
                    skip_create_dbs=False):
        """
        Create for each of the concatenated input measurement sets 
        """
        # Create the parameters set
        parset = self.parset.makeSubset("GSM.")

        # create the files that will contain the output of the recipe
        parmdbs_map_path = self._write_datamap_to_file(None, "parmdbs")
        self.logger.debug(
            "touched parmdbs output mapfile: {0}".format(parmdbs_map_path))
        sourcedb_map_path = self._write_datamap_to_file(None, "sky_files")
        self.logger.debug(
            "touched source db output mapfile: {0}".format(sourcedb_map_path))
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

        # This solution is not perfect but, the skip does not
        # return the complete output and this the data's will be empty
        # TODO
        if datamap != None:
            store_data_map(mapfile_path, datamap)
        else:
            if not os.path.exists(mapfile_path):
                store_data_map(mapfile_path, [])
                #open(mapfile_path, 'w').close()

        return mapfile_path


if __name__ == '__main__':
    sys.exit(msss_imager_pipeline().main())

