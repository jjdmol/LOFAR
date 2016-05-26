#!/usr/bin/env python
#                                                      STANDARD IMAGING PIPELINE
#
#                                                        Pre-Processing Pipeline
#                                                             Marcel Loose, 2012
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import os
import sys

from lofarpipe.support.control import control
from lofarpipe.support.data_map import DataMap, validate_data_maps
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.utilities import create_directory
from lofar.parameterset import parameterset
from lofarpipe.support.loggingdecorators import mail_log_on_exception, duration

class preprocessing_pipeline(control):
    """
    The pre-processing pipeline can be used to average raw UV-data in time
    and frequency, to flag RFI, and to demix strong A-team sources.
    
    This pipeline will perform the following operations:
    
    1. Prepare phase, collect data from parset and input mapfiles.
    2. Create VDS-file; it will contain important input-data for NDPPP
    3. Average and flag data, and demix A-team sources using NDPPP.
    """

    def __init__(self):
        super(preprocessing_pipeline, self).__init__()
        self.input_data = []
        self.output_data = []
        self.io_data_mask = []


    def _get_io_product_specs(self):
        """
        Get input- and output-data product specifications from the
        parset-file, and do some sanity checks.
        """
        dps = self.parset.makeSubset(
            self.parset.fullModuleName('DataProducts') + '.'
        )
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
        # Sanity checks on input- and output data product specifications
        if not validate_data_maps(self.input_data, self.output_data):
            raise PipelineException(
                "Validation of input/output data product specification failed!"
            )
#        # Validate input data, by searching the cluster for files
#        self._validate_input_data()
#        # Update input- and output-data product specifications if needed
#        if not all(self.io_data_mask):
#            self.logger.info("Updating input/output product specifications")
#            self.input_data = [
#                f for (f, m) in zip(self.input_data, self.io_data_mask) if m
#            ]
#            self.output_data = [
#                f for (f, m) in zip(self.output_data, self.io_data_mask) if m
#            ]


#    def _validate_input_data(self):
#        """
#        Search for the requested input files and mask the files in
#        `self.input_data[]` that could not be found on the system.
#        """
#        # Use filename glob-pattern as defined in LOFAR-USG-ICD-005.
#        self.io_data_mask = tally_data_map(
#            self.input_data, 'L*_SB???_uv.MS', self.logger
#        )
#        # Log a warning if not all input data files were found.
#        if not all(self.io_data_mask):
#            self.logger.warn(
#                "The following input data files were not found: %s" %
#                ', '.join(
#                    ':'.join(f) for (f, m) in zip(
#                        self.input_data, self.io_data_mask
#                    ) if not m
#                )
#            )


    @mail_log_on_exception
    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        # *********************************************************************
        # 1. Prepare phase, collect data from parset and input mapfiles.
        py_parset = self.parset.makeSubset(
            self.parset.fullModuleName('PythonControl') + '.')

        # Get input/output-data products specifications.
        self._get_io_product_specs()

        job_dir = self.config.get("layout", "job_directory")
        parset_dir = os.path.join(job_dir, "parsets")
        mapfile_dir = os.path.join(job_dir, "mapfiles")

        # Create directories for temporary parset- and map files
        create_directory(parset_dir)
        create_directory(mapfile_dir)

        # Write input- and output data map-files
        input_data_mapfile = os.path.join(mapfile_dir, "input_data.mapfile")
        self.input_data.save(input_data_mapfile)
        output_data_mapfile = os.path.join(mapfile_dir, "output_data.mapfile")
        self.output_data.save(output_data_mapfile)

        if len(self.input_data) == 0:
            self.logger.warn("No input data files to process. Bailing out!")
            return 0

        self.logger.debug("Processing: %s" %
            ', '.join(str(f) for f in self.input_data))

        # *********************************************************************
        # 2. Create VDS-file and databases. The latter are needed when doing
        #    demixing within DPPP.
        with duration(self, "vdsmaker"):
            gvds_file = self.run_task("vdsmaker", input_data_mapfile)['gvds']

        # Read metadata (start, end times, pointing direction) from GVDS.
        with duration(self, "vdsreader"):
            vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create a parameter database that will be used by the NDPPP demixing
        with duration(self, "setupparmdb"):
            parmdb_mapfile = self.run_task(
                "setupparmdb", input_data_mapfile,
                mapfile=os.path.join(mapfile_dir, 'dppp.parmdb.mapfile'),
                suffix='.dppp.parmdb'
            )['mapfile']
                
        # Create a source database from a user-supplied sky model
        # The user-supplied sky model can either be a name, in which case the
        # pipeline will search for a file <name>.skymodel in the default search
        # path $LOFARROOT/share/pipeline/skymodels; or a full path.
        # It is an error if the file does not exist.
        skymodel = py_parset.getString('PreProcessing.SkyModel')
        if not os.path.isabs(skymodel):
            skymodel = os.path.join(
                # This should really become os.environ['LOFARROOT']
                self.config.get('DEFAULT', 'lofarroot'),
                'share', 'pipeline', 'skymodels', skymodel + '.skymodel'
            )
        if not os.path.isfile(skymodel):
            raise PipelineException("Skymodel %s does not exist" % skymodel)
        with duration(self, "setupsourcedb"):
            sourcedb_mapfile = self.run_task(
                "setupsourcedb", input_data_mapfile,
                mapfile=os.path.join(mapfile_dir, 'dppp.sourcedb.mapfile'),
                skymodel=skymodel,
                suffix='.dppp.sourcedb',
                type='blob'
            )['mapfile']


        # *********************************************************************
        # 3. Average and flag data, using NDPPP.

        ndppp_parset = os.path.join(parset_dir, "NDPPP.parset")
        py_parset.makeSubset('DPPP.').writeFile(ndppp_parset)

        # Run the Default Pre-Processing Pipeline (DPPP);
        with duration(self, "ndppp"):
            output_data_mapfile = self.run_task("ndppp",
                (input_data_mapfile, output_data_mapfile),
                data_start_time=vdsinfo['start_time'],
                data_end_time=vdsinfo['end_time'],
                demix_always=
                    py_parset.getStringVector('PreProcessing.demix_always'),
                demix_if_needed=
                    py_parset.getStringVector('PreProcessing.demix_if_needed'),
                parset=ndppp_parset,
                parmdb_mapfile=parmdb_mapfile,
                sourcedb_mapfile=sourcedb_mapfile
            )['mapfile']

        # *********************************************************************
        # 6. Create feedback file for further processing by the LOFAR framework
        # Create a parset containing the metadata
        metadata_file = "%s_feedback_Correlated" % (self.parset_file,)
        with duration(self, "get_metadata"):
            self.run_task("get_metadata", output_data_mapfile,
                parset_prefix=(
                    self.parset.getString('prefix') +
                    self.parset.fullModuleName('DataProducts')),
                product_type="Correlated",
                metadata_file=metadata_file)

        self.send_feedback_processing(parameterset())
        self.send_feedback_dataproducts(parameterset(metadata_file))

        return 0


if __name__ == '__main__':
    sys.exit(preprocessing_pipeline().main())
