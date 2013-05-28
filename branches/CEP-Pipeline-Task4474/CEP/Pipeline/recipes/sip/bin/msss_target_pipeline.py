#!/usr/bin/env python
#                                                     LOFAR CALIBRATION PIPELINE
#
#                                          Target Pre-Processing Pipeline recipe
#                                                             Marcel Loose, 2011
#                                                                loose@astron.nl
# ------------------------------------------------------------------------------

import copy
import os
import sys

from lofarpipe.support.control import control
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.data_map import DataMap, validate_data_maps
#from lofarpipe.support.group_data import tally_data_map
from lofarpipe.support.utilities import create_directory
from lofar.parameterset import parameterset
from lofarpipe.support.loggingdecorators import mail_log_on_exception, duration


class msss_target_pipeline(control):
    """
    The target pipeline can be used to calibrate the observation of a "target"
    source using an instrument database that was previously determined using
    the calibrator_pipeline.

    This pipeline will perform the following operations:
    
    1. Prepare phase, collect data from parset and input mapfiles
    2. Copy the instrument files to the correct node, create new file with
       succesfull copied mss.
    3. Create database needed for performing work: 
       Vds, descibing data on the nodes sourcedb, For skymodel (A-team)
       parmdb for outputtting solutions
    4. Run NDPPP to demix the A-Team sources
    5. Run bss using the instrument file from the target observation, to correct for instrumental effects
    6. Second dppp run for  flaging NaN's in the MS. 
    7. Create feedback file for further processing by the LOFAR framework (MAC)

    **Per subband-group, the following output products will be delivered:**

    1. A new MS with a DATA column containing calibrated data

    """

    def __init__(self):
        control.__init__(self)
        self.parset = parameterset()
        self.input_data = {}
        self.output_data = {}
        self.parset_feedback_file = None


    def usage(self):
        """
        Display usage information
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
        self.input_data['data'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Input_Correlated.locations'),
                    dps.getStringVector('Input_Correlated.filenames'),
                    dps.getBoolVector('Input_Correlated.skip'))
        ])
        self.logger.debug("%d Input_Correlated data products specified" %
                          len(self.input_data['data']))

        self.input_data['instrument'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Input_InstrumentModel.locations'),
                    dps.getStringVector('Input_InstrumentModel.filenames'),
                    dps.getBoolVector('Input_InstrumentModel.skip'))
        ])
        self.logger.debug("%d Input_InstrumentModel data products specified" %
                          len(self.input_data['instrument']))
                          
        self.output_data['data'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Output_Correlated.locations'),
                    dps.getStringVector('Output_Correlated.filenames'),
                    dps.getBoolVector('Output_Correlated.skip'))
        ])
        self.logger.debug("%d Output_Correlated data products specified" %
                          len(self.output_data['data']))


    def _validate_io_product_specs(self):
        """
        Sanity checks on input- and output data product specifications
        """
        if not validate_data_maps(
            self.input_data['data'],
            self.input_data['instrument'],
            self.output_data['data']
        ):  raise PipelineException(
                "Validation of input/output data product specification failed!"
            )


    def _create_target_map_for_instruments(self):
        """
        Create a mapfile with target locations: based on the host found in
        the input data map, the name of the instrument file in the input
        instrument map, and the working directory + job name
        """
        scratch_dir = os.path.join(
            self.inputs['working_directory'], self.inputs['job_name'])

        target_locations = copy.deepcopy(self.input_data['instrument'])
        for data, target in zip(self.input_data['data'], target_locations):
            target.host = data.host
            target.file = os.path.join(
                scratch_dir, os.path.basename(target.file)
            )

        return target_locations


    def _copy_instrument_files(self, mapfile_dir):
        # For the copy recipe a target mapfile is needed
        # create target map based on the node and the dir in the input data map
        # with the filename based on the
        copier_map_path = os.path.join(mapfile_dir, "copier")
        create_directory(copier_map_path)
        target_map = self._create_target_map_for_instruments()

        #Write the two needed maps to file
        source_path = os.path.join(copier_map_path, "source_instruments.map")
        self.input_data['instrument'].save(source_path)

        target_path = os.path.join(copier_map_path, "target_instruments.map")
        target_map.save(target_path)

        copied_files_path = os.path.join(copier_map_path, "copied_instruments.map")

        # The output of the copier is a mapfile containing all the host, path
        # of succesfull copied files.
        copied_instruments_mapfile = self.run_task("copier",
                      mapfile_source=source_path,
                      mapfile_target=target_path,
                      mapfiles_dir=copier_map_path,
                      mapfile=copied_files_path)['mapfile_target_copied']

        # Some copy action might fail; the skip fields in the other map-files
        # need to be updated these to reflect this.
        self.input_data['instrument'] = DataMap.load(copied_instruments_mapfile)
        for data, inst, outp in zip(
            self.input_data['data'],
            self.input_data['instrument'],
            self.output_data['data']
        ):
            data.skip = inst.skip = outp.skip = (
                data.skip or inst.skip or outp.skip
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
                os.path.splitext(os.path.basename(parset_file))[0]
            )

        # Call the base-class's `go()` method.
        return super(msss_target_pipeline, self).go()


    @mail_log_on_exception
    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.
        """
        # *********************************************************************
        # 1. Prepare phase, collect data from parset and input mapfiles
        # Create a parameter-subset containing only python-control stuff.
        py_parset = self.parset.makeSubset(
            'ObsSW.Observation.ObservationControl.PythonControl.')

        # Get input/output-data products specifications.
        self._get_io_product_specs()

        # Create some needed directories
        job_dir = self.config.get("layout", "job_directory")
        mapfile_dir = os.path.join(job_dir, "mapfiles")
        create_directory(mapfile_dir)
        parset_dir = os.path.join(job_dir, "parsets")
        create_directory(parset_dir)

        # *********************************************************************
        # 2. Copy the instrument files to the correct node
        # The instrument files are currently located on the wrong nodes
        # Copy to correct nodes and assign the instrument table the now
        # correct data

        # Copy the instrument files to the corrent nodes: failures might happen
        # update both intrument and datamap to contain only successes!
        self._copy_instrument_files(mapfile_dir)

        # Write input- and output data map-files.
        data_mapfile = os.path.join(mapfile_dir, "data.mapfile")
        self.input_data['data'].save(data_mapfile)
        copied_instrument_mapfile = os.path.join(mapfile_dir, "copied_instrument.mapfile")
        self.input_data['instrument'].save(copied_instrument_mapfile)
        self.logger.debug(
            "Wrote input data mapfile: %s" % data_mapfile
        )

        # Save copied files to a new mapfile
        corrected_mapfile = os.path.join(mapfile_dir, "corrected_data.mapfile")
        self.output_data['data'].save(corrected_mapfile)
        self.logger.debug(
            "Wrote output corrected data mapfile: %s" % corrected_mapfile
        )

        # Validate number of copied files, abort on zero files copied
        if len(self.input_data['data']) == 0:
            self.logger.warn("No input data files to process. Bailing out!")
            return 0

        self.logger.debug("Processing: %s" %
            ', '.join(str(f) for f in self.input_data['data'])
        )

        # *********************************************************************
        # 3. Create database needed for performing work: 
        #    Vds, descibing data on the nodes
        #    sourcedb, For skymodel (A-team)
        #    parmdb for outputtting solutions
        # Produce a GVDS file describing the data on the compute nodes.
        with duration(self, "vdsmaker"):
            gvds_file = self.run_task("vdsmaker", data_mapfile)['gvds']

        # Read metadata (e.g., start- and end-time) from the GVDS file.
        with duration(self, "vdsreader"):
            vdsinfo = self.run_task("vdsreader", gvds=gvds_file)

        # Create an empty parmdb for DPPP
        with duration(self, "setupparmdb"):
            parmdb_mapfile = self.run_task("setupparmdb", data_mapfile)['mapfile']

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
                "setupsourcedb", data_mapfile,
                skymodel=skymodel,
                suffix='.dppp.sourcedb',
                type='blob'
            )['mapfile']

        # *********************************************************************
        # 4. Run NDPPP to demix the A-Team sources
        # Create a parameter-subset for DPPP and write it to file.
        ndppp_parset = os.path.join(parset_dir, "NDPPP[0].parset")
        py_parset.makeSubset('DPPP[0].').writeFile(ndppp_parset)

        # Run the Default Pre-Processing Pipeline (DPPP);
        with duration(self, "ndppp"):
            dppp_mapfile = self.run_task("ndppp",
                data_mapfile,
                data_start_time=vdsinfo['start_time'],
                data_end_time=vdsinfo['end_time'],
                demix_always=
                    py_parset.getStringVector('PreProcessing.demix_always'),
                demix_if_needed=
                    py_parset.getStringVector('PreProcessing.demix_if_needed'),
                parset=ndppp_parset,
                parmdb_mapfile=parmdb_mapfile,
                sourcedb_mapfile=sourcedb_mapfile,
                mapfile=os.path.join(mapfile_dir, 'dppp[0].mapfile')
            )['mapfile']

#        demix_mapfile = dppp_mapfile
#        # Demix the relevant A-team sources
#        demix_mapfile = self.run_task("demixing", dppp_mapfile)['mapfile']

        # ********************************************************************
        # 5. Run bss using the instrument file from the target observation
        # Create an empty sourcedb for BBS
        with duration(self, "setupsourcedb"):
            sourcedb_mapfile = self.run_task(
                "setupsourcedb", data_mapfile
            )['mapfile']

        # Create a parameter-subset for BBS and write it to file.
        bbs_parset = os.path.join(parset_dir, "BBS.parset")
        py_parset.makeSubset('BBS.').writeFile(bbs_parset)

        # Run BBS to calibrate the target source(s).
        with duration(self, "bbs_reducer"):
            bbs_mapfile = self.run_task("bbs_reducer",
                dppp_mapfile,
                parset=bbs_parset,
                instrument_mapfile=copied_instrument_mapfile,
                sky_mapfile=sourcedb_mapfile
            )['data_mapfile']

        # *********************************************************************
        # 6. Second dppp run for  flaging NaN's in the MS.  
        # Create another parameter-subset for a second DPPP run.
        ndppp_parset = os.path.join(parset_dir, "NDPPP[1].parset")
        py_parset.makeSubset('DPPP[1].').writeFile(ndppp_parset)

        # Do a second run of DPPP, just to flag NaN's from the MS. Store the
        # results in the files specified in the corrected data map-file
        # WARNING: This will create a new MS with a DATA column containing the
        # CORRECTED_DATA column of the original MS.
        with duration(self, "ndppp"):
            self.run_task("ndppp",
                (bbs_mapfile, corrected_mapfile),
                clobber=False,
                suffix='',
                parset=ndppp_parset,
                mapfile=os.path.join(mapfile_dir, 'dppp[1].mapfile')
            )

        # 7. Create feedback file for further processing by the LOFAR framework
        # (MAC)
        # Create a parset-file containing the metadata for MAC/SAS
        with duration(self, "get_metadata"):
            self.run_task("get_metadata", corrected_mapfile,
                parset_file=self.parset_feedback_file,
                parset_prefix=(
                    self.parset.getString('prefix') +
                    self.parset.fullModuleName('DataProducts')
                ),
                product_type="Correlated")

        return 0


if __name__ == '__main__':
    sys.exit(msss_target_pipeline().main())
