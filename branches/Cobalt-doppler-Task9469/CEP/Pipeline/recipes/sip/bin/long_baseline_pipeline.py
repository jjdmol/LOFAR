#!/usr/bin/env python
#                                                        LOFAR IMAGING PIPELINE
#
#                                                 long baseline Pipeline recipe
#                                                            Marcel Loose, 2012
#                                                               loose@astron.nl
#                                                            Wouter Klijn, 2012
#                                                               klijn@astron.nl
#      copy from imaging refactor to long baseline           Wouter Klijn, 2012
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


class longbaseline_pipeline(control):
    """
    The Automatic MSSS long baselione pipeline is used to generate MSSS 
    measurement sets combining information of multiple subbands and or 
    observations into measurements sets. They are are complemented with meta
    data and thus ready for consumption by the Long Term Storage (LTA)

    *subband groups*
    The pipeline is able to generate measurementssets on the frequency range of
    LOFAR in parallel. Combining the frequency subbands together in so called
    subbandgroups. 

    *Time Slices*
    the measurmentsets are compiled from a number of so-called (time) slices. Each
    slice comprises an observation of a field (an area on
    the sky) containing typically 80 subbands. The number of slices will be
    different for LBA observations (typically 9) and HBA observations
    (typically 2), due to differences in sensitivity.

    
    **This pipeline performs the following operations:**

    1. Long baseline . Copy the preprocessed MS's from the different compute
       nodes to the nodes where the images will be compiled (the prepare phase)
       Combine the subbands in subband groups, concattenate the timeslice in a
       single large measurement set and perform flagging, RFI and bad station
       exclusion.

    2. Generate meta information feedback based on dataproduct information
       and parset/configuration data

    **Per subband-group, the following output products will be delivered:**

    a. An measurement set
    """
    def __init__(self):
        """
        Initialize member variables and call superclass init function
        """
        control.__init__(self)
        self.input_data = DataMap()
        self.target_data = DataMap()
        self.output_data = DataMap()
        self.scratch_directory = None
        self.parset_dir = None
        self.mapfile_dir = None


    @mail_log_on_exception
    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        self.logger.info("Starting longbaseline pipeline")

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

        ## ***************************************************************
        #output_mapfile_path = os.path.join(self.mapfile_dir, "output.mapfile")
        #self.output_mapfile.save(output_mapfile_path)

        # storedata_map(input_mapfile, self.input_data)
        self.logger.debug(
            "Wrote input UV-data mapfile: {0}".format(input_mapfile))

        # Provides location for the scratch directory and concat.ms location
        target_mapfile = os.path.join(self.mapfile_dir, "target.mapfile")
        self.target_data.save(target_mapfile)
        self.logger.debug(
            "Wrote target mapfile: {0}".format(target_mapfile))

        # images datafiles
        output_ms_mapfile = os.path.join(self.mapfile_dir, "output.mapfile")
        self.output_data.save(output_ms_mapfile)
        self.logger.debug(
            "Wrote output sky-image mapfile: {0}".format(output_ms_mapfile))

        # TODO: This is a backdoor option to manually add beamtables when these
        # are missing on the provided ms. There is NO use case for users of the
        # pipeline
        add_beam_tables = self.parset.getBool(
                                    "Imaging.addBeamTables", False)

        # ******************************************************************
        # (1) prepare phase: copy and collect the ms
        concat_ms_map_path, timeslice_map_path, ms_per_image_map_path, \
            processed_ms_dir = self._long_baseline(input_mapfile,
                         target_mapfile, add_beam_tables, output_ms_mapfile)

        # *********************************************************************
        # (7) Get metadata
        # create a parset with information that is available on the toplevel
        toplevel_meta_data = parameterset()

        # get some parameters from the imaging pipeline parset:
        subbandgroups_per_ms = self.parset.getInt("LongBaseline.subbandgroups_per_ms")
        subbands_per_subbandgroup = self.parset.getInt("LongBaseline.subbands_per_subbandgroup")

        toplevel_meta_data.replace("subbandsPerSubbandGroup", 
                                           str(subbands_per_subbandgroup))
        toplevel_meta_data.replace("subbandGroupsPerMS", 
                                           str(subbandgroups_per_ms))
        
        # Create a parset-file containing the metadata for MAC/SAS at nodes
        metadata_file = "%s_feedback_Correlated" % (self.parset_file,)
        self.run_task("get_metadata", output_ms_mapfile,
            parset_prefix = (
                full_parset.getString('prefix') +
                full_parset.fullModuleName('DataProducts')
            ),
            product_type = "Correlated",
            metadata_file = metadata_file)

        self.send_feedback_processing(toplevel_meta_data)
        self.send_feedback_dataproducts(parameterset(metadata_file))

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
                    dps.getStringVector('Output_Correlated.locations'),
                    dps.getStringVector('Output_Correlated.filenames'),
                    dps.getBoolVector('Output_Correlated.skip'))
        ])
        self.logger.debug("%d Output_Correlated data products specified" %
                          len(self.output_data))

        # # Sanity checks on input- and output data product specifications
        # if not validate_data_maps(self.input_data, self.output_data):
        #    raise PipelineException(
        #        "Validation of input/output data product specification failed!"
        #    )#Turned off untill DataMap is extended..

        # Target data is basically scratch data, consisting of one concatenated
        # MS per image. It must be stored on the same host as the final image.
        self.target_data = copy.deepcopy(self.output_data)

        for idx, item in enumerate(self.target_data):
            item.file = os.path.join(self.scratch_directory, 'ms_per_image_%d' % idx, 'concat.ms')


    @xml_node
    def _finalize(self, awimager_output_map, processed_ms_dir,
                  ms_per_image_map, sourcelist_map, minbaseline,
                  maxbaseline, target_mapfile,
                  output_ms_mapfile, sourcedb_map, skip = False):
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
            placed_image_mapfile = self.run_task("imager_finalize",
                target_mapfile, awimager_output_map = awimager_output_map,
                    ms_per_image_map = ms_per_image_map,
                    sourcelist_map = sourcelist_map,
                    sourcedb_map = sourcedb_map,
                    minbaseline = minbaseline,
                    maxbaseline = maxbaseline,
                    target_mapfile = target_mapfile,
                    output_ms_mapfile = output_ms_mapfile,
                    processed_ms_dir = processed_ms_dir,
                    placed_image_mapfile = placed_image_mapfile
                    )["placed_image_mapfile"]

        return placed_image_mapfile


    @xml_node
    def _long_baseline(self, input_ms_map_path, target_mapfile,
        add_beam_tables, output_ms_mapfile):
        """
        Copy ms to correct location, combine the ms in slices and combine
        the time slices into a large virtual measurement set
        """
        # Create the dir where found and processed ms are placed
        # ms_per_image_map_path contains all the original ms locations:
        # this list contains possible missing files
        processed_ms_dir = os.path.join(self.scratch_directory, "subbands")

        # get the parameters, create a subset for ndppp, save
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
        subbandgroups_per_ms = self.parset.getInt("LongBaseline.subbandgroups_per_ms")
        subbands_per_subbandgroup = self.parset.getInt("LongBaseline.subbands_per_subbandgroup")


        outputs = self.run_task("long_baseline", input_ms_map_path,
                parset = ndppp_parset_path,
                target_mapfile = target_mapfile,
                subbandgroups_per_ms = subbandgroups_per_ms,
                subbands_per_subbandgroup = subbands_per_subbandgroup,
                mapfile = output_mapfile,
                slices_mapfile = time_slices_mapfile,
                ms_per_image_mapfile = ms_per_image_mapfile,
                working_directory = self.scratch_directory,
                processed_ms_dir = processed_ms_dir,
                add_beam_tables = add_beam_tables,
                output_ms_mapfile = output_ms_mapfile)

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


if __name__ == '__main__':
    sys.exit(longbaseline_pipeline().main())
