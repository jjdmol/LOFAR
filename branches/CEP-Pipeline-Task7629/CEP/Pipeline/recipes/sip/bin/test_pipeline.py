#!/usr/bin/env python
#                                                        LOFAR IMAGING PIPELINE
#
#                                                        test toplevel recipe
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


class test_pipeline(control):
    """
    The Automatic MSSS imager pipeline is used to generate MSSS images and find
    sources in the generated images. Generated images and lists of found
    sources are complemented with meta data and thus ready for consumption by
    the Long Term Storage (LTA)

    """
    def __init__(self):
        """
        Initialize member variables and call superclass init function
        """
        control.__init__(self)
        self.input_data = DataMap()
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
        self.logger.info("Starting test pipeline")

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
        input_mapfile = os.path.join(self.mapfile_dir, "input.mapfile")
        self.input_data.save(input_mapfile)

        # storedata_map(input_mapfile, self.input_data)
        self.logger.debug(
            "Wrote input  mapfile: {0}".format(input_mapfile))

        # images datafiles
        output_mapfile = os.path.join(self.mapfile_dir, "output.mapfile")
        self.output_data.save(output_mapfile)
        self.logger.debug(
            "Wrote output mapfile: {0}".format(output_mapfile))


        # ******************************************************************
        # (1) Start the test recipe
        output_mapfile_testphase = self._test_phase(input_mapfile,
                                                    output_mapfile)


        ## *********************************************************************
        ## (7) Get metadata
        ## create a parset with information that is available on the toplevel

        ## Create a parset containing the metadata for MAC/SAS at nodes
        #metadata_file = "%s_feedback_SkyImage" % (self.parset_file,)
        #self.run_task("get_metadata", placed_data_image_map,
        #    parset_prefix = (
        #        full_parset.getString('prefix') +
        #        full_parset.fullModuleName('DataProducts')
        #    ),
        #    product_type = "SkyImage",
        #    metadata_file = metadata_file)

        #self.send_feedback_processing(toplevel_meta_data)
        #self.send_feedback_dataproducts(parameterset(metadata_file))

        return 0

    @xml_node
    def _test_phase(self, input_map_path, output_mapfile):
        """
        """


        outputs = self.run_task("test_recipe", input_map_path,
                output_map_path = output_mapfile,
                executable = "ls")

        # Return the mapfiles paths with processed data
        return output_mapfile

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
    sys.exit(test_pipeline().main())
