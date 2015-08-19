#!/usr/bin/env python
#
# pulsar_pipeline.py is a wrapper around the actual pulsar pipeline pulp.py
# It is supplied with a pipeline parset which it will digest.
# It reads data input and output locations which are written to mapfiles
# These mapfiles can then be read by pulp to know which input data
# it should use and which output locations and filenames it should write to.
#
# Alwin de Jong (jong@astron.nl) March 2014
###################################################################

#import os, sys, re
#import optparse as opt
#from subprocess import PIPE, STDOUT, Popen

import os
import sys
import pulp

from string import join
from lofarpipe.support.control import control
from lofarpipe.support.data_map import DataMap, validate_data_maps
from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.utilities import create_directory
from lofar.parameterset import parameterset
from lofarpipe.support.loggingdecorators import mail_log_on_exception, duration


class pulsar_pipeline(control):
    """
    This is the main wrapper class around the pulsar pipeline for
    integration of the pulsar pipelines as a 'black-box'
    """

    def __init__(self):
        super(pulsar_pipeline, self).__init__()
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
        # Coherent Stokes input data
        self.coherentStokesEnabled = dps.getBool('Input_CoherentStokes.enabled', False)
        self.input_data['coherent'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Input_CoherentStokes.locations'),
                    dps.getStringVector('Input_CoherentStokes.filenames'),
                    dps.getBoolVector('Input_CoherentStokes.skip'))
        ])
        self.logger.debug("%d Input_CoherentStokes data products specified" %
                    len(self.input_data['coherent']))
        # Incoherent Stokes input data
        self.incoherentStokesEnabled = dps.getBool('Input_IncoherentStokes.enabled', False)
        self.input_data['incoherent'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Input_IncoherentStokes.locations'),
                    dps.getStringVector('Input_IncoherentStokes.filenames'),
                    dps.getBoolVector('Input_IncoherentStokes.skip'))
        ])
        self.logger.debug("%d Input_IncoherentStokes data products specified" %
                            len(self.input_data['incoherent']))
        self.output_data['data'] = DataMap([
            tuple(os.path.join(location, filename).split(':')) + (skip,)
                for location, filename, skip in zip(
                    dps.getStringVector('Output_Pulsar.locations'),
                    dps.getStringVector('Output_Pulsar.filenames'),
                    dps.getBoolVector('Output_Pulsar.skip'))
        ])
        self.logger.debug("%d Output_Pulsar data products specified" %
                            len(self.output_data['data']))
        
        # Sanity checks on input- and output data product specifications
        # the existing validate_data_maps will probably not work for pulsar pipeline,
        # were length of output equals length of input coherent + length of input incoherent arrays
        # in_len = len(self.input_data['coherent']) + len(self.input_data['incoherent'])
        # out_len = len(self.output_data['data'])
        # if (in_len != out_len):
        #     raise DataMapError("number of enabled input data products %s does not match the total number of output files %s" % 
        #                     (in_len, out_len))
        
    @mail_log_on_exception
    def pipeline_logic(self):
        """
        Define the individual tasks that comprise the current pipeline.
        This method will be invoked by the base-class's `go()` method.

        Note: return 0 on success, 1 on failure.
        """
        # *********************************************************************
        # 1. Prepare phase, collect data from parset and input mapfiles.
        #
        # Note that PULP will read many of these fields directly. That makes
        # the following fields, and possibly others, part of the API towards
        # PULP:
        #
        # self.config
        # self.logger
        # self.input_data
        # self.output_data
        # self.parset_feedback_file
        # self.job_dir

        # Get input/output-data products specifications.
        self._get_io_product_specs()

        self.job_dir = self.config.get("layout", "job_directory")
        parset_dir = os.path.join(self.job_dir, "parsets")
        mapfile_dir = os.path.join(self.job_dir, "mapfiles")
        
        # Create directories for temporary parset- and map files
        create_directory(parset_dir)
        create_directory(mapfile_dir)

        # Write input- and output data map-files
        # Coherent Stokes
        self.input_CS_mapfile = os.path.join(mapfile_dir, "input_CS_data.mapfile")
        self.input_data['coherent'].save(self.input_CS_mapfile)
        # Incoherent Stokes
        self.input_IS_mapfile = os.path.join(mapfile_dir, "input_IS_data.mapfile")
        self.input_data['incoherent'].save(self.input_IS_mapfile)
        # Output data
        self.output_data_mapfile = os.path.join(mapfile_dir, "output_data.mapfile")
        self.output_data['data'].save(self.output_data_mapfile)

        if len(self.input_data) == 0:
            self.logger.warn("No input data files to process. Bailing out!")
            return 0

        self.pulsar_parms = self.parset.makeSubset(self.parset.fullModuleName('Pulsar') + '.')
        pulsar_parset = os.path.join(parset_dir, "Pulsar.parset")
        self.pulsar_parms.writeFile(pulsar_parset)
            
        self.logger.debug("Processing: %s" %
          ', '.join(str(f) for f in self.input_data))
        
        # Rebuilding sys.argv without the options given automatically by framework
        # --auto = automatic run from framework
        # -q = quiet mode, no user interaction
        sys.argv = ['pulp.py', '--auto', '-q']
      
        if (not self.coherentStokesEnabled):
          sys.argv.extend(["--noCS", "--noCV", "--noFE"])
          
        if (not self.incoherentStokesEnabled):
          sys.argv.append("--noIS")       

        # Tell PULP where to write the feedback to
        self.parset_feedback_file =  "%s_feedback" % (self.parset_file,)
       
        # Run the pulsar pipeline
        self.logger.debug("Starting pulp with: " + join(sys.argv))
        p = pulp.pulp(self) # TODO: MUCK self to capture the API

        # NOTE: PULP returns 0 on SUCCESS!!
        if p.go():
          self.logger.error("PULP did not succeed. Bailing out!")
          return 1

        # Read and forward the feedback
        try:
          metadata = parameterset(self.parset_feedback_file)
        except IOError, e:
          self.logger.error("Could not read feedback from %s: %s" % (metadata_file,e))
          return 1

        self.send_feedback_processing(parameterset())
        self.send_feedback_dataproducts(metadata)
        return 0

    
if __name__ == '__main__':
    sys.exit(pulsar_pipeline().main())
