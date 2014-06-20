#!/usr/bin/env python
#                                                      STANDARD IMAGING PIPELINE
#
#                                                           Calibration Pipeline
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


class calibration_pipeline(control):
    """
    The calibration pipeline can be used to pre-process raw UV-data (i.e.
    average in time and frequency, flag RFI, and demix strong A-team sources),
    and to calibrate these data using a user-supplied sky model.
    
    This pipeline will perform the following operations:
    
    1. Prepare phase, collect data from parset and input mapfiles.
    2. Create VDS-file; it will contain important input-data for NDPPP
    3. Average and flag data, and demix A-team sources using NDPPP.
    4. Create a sourcedb from the user-supplied sky model, and an empty parmdb.
    5. Run BBS to calibrate the data.
    6. Copy the MS's to their final output destination.
    7. Create feedback file for further processing by the LOFAR framework (MAC)
    """

    def __init__(self):
        super(calibration_pipeline, self).__init__()
        self.parset = parameterset()
        self.input_data = {}
        self.output_data = {}
        self.parset_feedback_file = None


    def usage(self):
        """
        Display usage
        """
        print >> sys.stderr, "Usage: %s [options] <parset-file>" % sys.argv[0]
        return 1


    def _get_io_product_specs(self):
        """
        Get input- and output-data product specifications from the
        parset-file, and do some sanity checks.
        """
        dps = self.parset.makeSubset(
            self.parset.fullModuleName('DataProducts') + '.'
        )
        self.input_data['correlated'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Input_Correlated.locations'),
                    dps.getStringVector('Input_Correlated.filenames'),
                    dps.getBoolVector('Input_Correlated.skip'))
        ])
        self.logger.debug("%d Input_Correlated data products specified" %
                          len(self.input_data['correlated']))

        self.output_data['correlated'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Output_Correlated.locations'),
                    dps.getStringVector('Output_Correlated.filenames'),
                    dps.getBoolVector('Output_Correlated.skip'))
        ])
        self.logger.debug("%d Output_Correlated data products specified" %
                          len(self.output_data['correlated']))

        self.output_data['instrument'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Output_InstrumentModel.locations'),
                    dps.getStringVector('Output_InstrumentModel.filenames'),
                    dps.getBoolVector('Output_InstrumentModel.skip'))
        ])
        self.logger.debug("%d Output_InstrumentModel data products specified" %
                          len(self.output_data['instrument']))

        # Sanity checks on input- and output data product specifications
        if not validate_data_maps(
            self.input_data['correlated'],
            self.output_data['correlated'],
            self.output_data['instrument']):
            raise PipelineException(
                "Validation of input/output data product specification failed!"
            )


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
                os.path.splitext(os.path.basename(parset_file))[0])

        # Call the base-class's `go()` method.
        return super(calibration_pipeline, self).go()


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
        input_correlated_mapfile = os.path.join(
            mapfile_dir, "input_correlated.mapfile"
        )
        output_correlated_mapfile = os.path.join(
            mapfile_dir, "output_correlated.mapfile"
        )
        output_instrument_mapfile = os.path.join(
            mapfile_dir, "output_instrument.mapfile"
        )
        self.input_data['correlated'].save(input_correlated_mapfile)
        self.output_data['correlated'].save(output_correlated_mapfile)
        self.output_data['instrument'].save(output_instrument_mapfile)

        if len(self.input_data['correlated']) == 0:
            self.logger.warn("No input data files to process. Bailing out!")
            return 0

        self.logger.debug("Processing: %s" %
            ', '.join(str(f) for f in self.input_data['correlated']))

        # *********************************************************************
        # 2. Create VDS-file and databases. The latter are needed when doing
        #    demixing within DPPP.
        with duration(self, "vdsmaker"):
            gvds_file = self.run_task(
                "vdsmaker", input_correlated_mapfile
            )['gvds']

        # Read metadata (start, end times, pointing direction) from GVDS.
        with duration(self, "vdsreader"):
            vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create a parameter database that will be used by the NDPPP demixing
        with duration(self, "setupparmdb"):
            parmdb_mapfile = self.run_task(
                "setupparmdb", input_correlated_mapfile,
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
                "setupsourcedb", input_correlated_mapfile,
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
            dppp_mapfile = self.run_task(
                "ndppp", input_correlated_mapfile,
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
        # 4. Create a sourcedb from the user-supplied sky model, 
        #    and an empty parmdb.
        skymodel = py_parset.getString('Calibration.SkyModel')

        # The user-supplied sky model can either be a name, in which case the
        # pipeline will search for a file <name>.skymodel in the default search
        # path $LOFARROOT/share/pipeline/skymodels; or a full path.
        # It is an error if the file does not exist.
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
                "setupsourcedb", dppp_mapfile,
                skymodel=skymodel,
                suffix='.bbs.sourcedb'
            )['mapfile']

        with duration(self, "setupparmdb"):
            parmdb_mapfile = self.run_task(
                "setupparmdb", dppp_mapfile,
                suffix='.bbs.parmdb'
            )['mapfile']

        # *********************************************************************
        # 5. Run BBS to calibrate the data.

        # Create a parameter subset for BBS
        bbs_parset = os.path.join(parset_dir, "BBS.parset")
        py_parset.makeSubset('BBS.').writeFile(bbs_parset)
        with duration(self, "bbs_reducer"):
            bbs_mapfile = self.run_task(
                "bbs_reducer", dppp_mapfile,
                parset=bbs_parset,
                instrument_mapfile=parmdb_mapfile,
                sky_mapfile=sourcedb_mapfile
            )['data_mapfile']

        # *********************************************************************
        # 6. Copy output products to their final destination.
        #    a. copy the measurement sets
        #    b. copy the calculated instrument models
        #  When the copier recipe has run, the map-files named in
        #  output_correlated_mapfile and output_instrument_mapfile will
        #  contain an updated map of output files.
        with duration(self, "copier"):
            self.run_task("copier",
                mapfile_source=bbs_mapfile,
                mapfile_target=output_correlated_mapfile,
                mapfiles_dir=mapfile_dir,
                mapfile=output_correlated_mapfile
            )

        with duration(self, "copier"):
            self.run_task("copier",
                mapfile_source=parmdb_mapfile,
                mapfile_target=output_instrument_mapfile,
                mapfiles_dir=mapfile_dir,
                mapfile=output_instrument_mapfile
            )

        # *********************************************************************
        # 7. Create feedback file for further processing by the LOFAR framework
        #    a. get metadata of the measurement sets
        #    b. get metadata of the instrument models
        #    c. join the two files and write the final feedback file
        correlated_metadata = os.path.join(parset_dir, "correlated.metadata")
        instrument_metadata = os.path.join(parset_dir, "instrument.metadata")
        with duration(self, "get_metadata"):
            self.run_task("get_metadata", output_correlated_mapfile,
                parset_file=correlated_metadata,
                parset_prefix=(
                    self.parset.getString('prefix') +
                    self.parset.fullModuleName('DataProducts')),
                product_type="Correlated")

        with duration(self, "get_metadata"):
            self.run_task("get_metadata", output_instrument_mapfile,
                parset_file=instrument_metadata,
                parset_prefix=(
                    self.parset.getString('prefix') +
                    self.parset.fullModuleName('DataProducts')),
                product_type="InstrumentModel")

        parset = parameterset(correlated_metadata)
        parset.adoptFile(instrument_metadata)
        parset.writeFile(self.parset_feedback_file)

        return 0


if __name__ == '__main__':
    sys.exit(calibration_pipeline().main())
