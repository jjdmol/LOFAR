#!/usr/bin/env python
#                                                      STANDARD IMAGING PIPELINE
#
#                                                       MSSS Calibrator Pipeline
#                                                        Marcel Loose, 2011-2012
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


class msss_calibrator_pipeline(control):
    """
    The calibrator pipeline can be used to determine the instrument database
    (parmdb) from the observation of a known "calibrator" source. It creates an
    instrument model of the current LOFAR instrument (As sum of instrumental
    properties and Ionospere disturbances TODOW). The output of this toplevel
    pipeline recipe is this instrument model. Which can be used in a later
    target pipeline calibrate target data. 

    **This pipeline will perform the following operations:**

    1. Preparations, Parse and validate input and set local variables
    2. Create database (files), A sourcedb with A-Team sources, a vds file
       describing the nodes, a parmdb for calibration solutions
    3. DPPP. flagging, using standard parset
       Demix the relevant A-team sources), using the A-team sourcedb.
    4. Run BBS to calibrate the calibrator source(s), again using standard
       parset, and the sourcedb made earlier
    5. Perform gain correction on the created instrument table
    6. Copy corrected MS's to their final output destination
    7. Create metadata for consumption by the LOFAR framework

    **Per subband-group, the following output products will be delivered:**

    1. An parmdb with instrument calibration solution to be applied to a target
       measurement set  in the target pipeline

    """

    def __init__(self):
        control.__init__(self)
        self.input_data = {}
        self.output_data = {}


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


    @mail_log_on_exception
    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        # *********************************************************************
        # 1. Get input from parset, validate and cast to pipeline 'data types'
        #    Only perform work on existing files
        #    Created needed directories 
        # Create a parameter-subset containing only python-control stuff.
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
        # 2. Create database needed for performing work: 
        #    Vds, descibing data on the nodes
        #    sourcedb, For skymodel (A-team)
        #    parmdb for outputtting solutions
        # Produce a GVDS file describing the data on the compute nodes.
        with duration(self, "vdsmaker"):
            gvds_file = self.run_task(
                "vdsmaker", input_correlated_mapfile
            )['gvds']

        # Read metadata (start, end times, pointing direction) from GVDS.
        with duration(self, "vdsreader"):
            vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create an empty parmdb for DPPP
        with duration(self, "setupparmdb"):
            parmdb_mapfile = self.run_task(
                "setupparmdb", input_correlated_mapfile,
                mapfile=os.path.join(mapfile_dir, 'dppp.parmdb.mapfile'),
                suffix='.dppp.parmdb'
            )['mapfile']

        # Create a sourcedb to be used by the demixing phase of DPPP
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
        # 3. Run NDPPP to demix the A-Team sources
        #    TODOW: Do flagging?
        # Create a parameter-subset for DPPP and write it to file.
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
        # 4. Run BBS with a model of the calibrator
        #    Create a parmdb for calibration solutions
        #    Create sourcedb with known calibration solutions
        #    Run bbs with both
        # Create an empty parmdb for BBS
        with duration(self, "setupparmdb"):
            parmdb_mapfile = self.run_task(
                "setupparmdb", dppp_mapfile,
                mapfile=os.path.join(mapfile_dir, 'bbs.parmdb.mapfile'),
                suffix='.bbs.parmdb'
            )['mapfile']

        # Create a sourcedb based on sourcedb's input argument "skymodel"
        with duration(self, "setupsourcedb"):
            sourcedb_mapfile = self.run_task(
                "setupsourcedb", input_correlated_mapfile,
                skymodel=os.path.join(
                    self.config.get('DEFAULT', 'lofarroot'),
                    'share', 'pipeline', 'skymodels',
                    py_parset.getString('Calibration.SkyModel') +
                        '.skymodel'),
                mapfile=os.path.join(mapfile_dir, 'bbs.sourcedb.mapfile'),
                suffix='.bbs.sourcedb')['mapfile']

        # Create a parameter-subset for BBS and write it to file.
        bbs_parset = os.path.join(parset_dir, "BBS.parset")
        py_parset.makeSubset('BBS.').writeFile(bbs_parset)

        # Run BBS to calibrate the calibrator source(s).
        with duration(self, "bbs_reducer"):
            bbs_mapfile = self.run_task(
                "bbs_reducer", dppp_mapfile,
                parset=bbs_parset,
                instrument_mapfile=parmdb_mapfile,
                sky_mapfile=sourcedb_mapfile
            )['data_mapfile']

        # *********************************************************************
        # 5. Perform gain outlier correction on the found calibration solutions
        #    Swapping outliers in the gains with the median 
        # Export the calibration solutions using gainoutliercorrection and store
        # the results in the files specified in the instrument mapfile.
        export_instrument_model = py_parset.getBool(
            'Calibration.exportCalibrationParameters', False)

        with duration(self, "gainoutliercorrection"):
            self.run_task("gainoutliercorrection",
                      (parmdb_mapfile, output_instrument_mapfile),
                      sigma=1.0,
                      export_instrument_model=export_instrument_model) # TODO: Parset parameter

        # *********************************************************************
        # 6. Copy corrected MS's to their final output destination.
        with duration(self, "copier"):
            self.run_task("copier",
                mapfile_source=bbs_mapfile,
                mapfile_target=output_correlated_mapfile,
                mapfiles_dir=mapfile_dir,
                mapfile=output_correlated_mapfile
            )

        # *********************************************************************
        # 7. Create feedback for further processing by the LOFAR framework
        #    a. get metadata of the measurement sets
        #    b. get metadata of the instrument models
        #    c. join the two and write the final feedback
        correlated_metadata_file = "%s_feedback_Correlated" % (self.parset_file,)
        with duration(self, "get_metadata"):
            self.run_task("get_metadata", output_correlated_mapfile,
                parset_prefix=(
                    self.parset.getString('prefix') +
                    self.parset.fullModuleName('DataProducts')),
                product_type="Correlated",
                metadata_file=correlated_metadata_file)

        instrument_metadata_file = "%s_feedback_InstrumentModel" % (self.parset_file,)
        with duration(self, "get_metadata"):
            self.run_task("get_metadata", output_instrument_mapfile,
                parset_prefix=(
                    self.parset.getString('prefix') +
                    self.parset.fullModuleName('DataProducts')),
                product_type="InstrumentModel",
                metadata_file=instrument_metadata_file)

        self.send_feedback_processing(parameterset())
        self.send_feedback_dataproducts(parameterset(correlated_metadata_file))
        self.send_feedback_dataproducts(parameterset(instrument_metadata_file))

        return 0

if __name__ == '__main__':
    sys.exit(msss_calibrator_pipeline().main())
