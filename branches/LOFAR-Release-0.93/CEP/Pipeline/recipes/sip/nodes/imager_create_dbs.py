## LOFAR AWIMAGER RECIPE
## Wouter Klijn 2012
## klijn@astron.nl
## -----------------------------------------------------------------------------
#
from __future__ import with_statement
import sys
import subprocess
import math

import pyrap.tables as pt                                                       #@UnresolvedImport
import monetdb.sql as db                                                        #@UnresolvedImport 
import gsmutils as gsm                                                          #@UnresolvedImport 

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import log_process_output
#from lofar.parameterset import parameterset #@UnresolvedImport #marcel new im

#TODO: de template moet mischien toch ergens anders vandaan worden gehaalt.
template_parmdb = """
create tablename="{0}"
adddef Gain:0:0:Ampl  values=1.0
adddef Gain:1:1:Ampl  values=1.0
adddef Gain:0:0:Real  values=1.0
adddef Gain:1:1:Real  values=1.0
adddef DirectionalGain:0:0:Ampl  values=1.0
adddef DirectionalGain:1:1:Ampl  values=1.0
adddef DirectionalGain:0:0:Real  values=1.0
adddef DirectionalGain:1:1:Real  values=1.0
adddef AntennaOrientation values=5.497787144
quit
"""


class imager_create_dbs(LOFARnodeTCP):
    def run(self, concatenated_measurement_set, sourcedb_target_path,
            monet_db_hostname, monet_db_port, monet_db_name, monet_db_user,
            monet_db_password, assoc_theta, parmdb_executable, slice_paths,
            parmdb_suffix):
        """
        run() creates a source db for the concatenated measurement set. For each
        of the individual time slices a paramdb is created.
        All inputs are path wise, and no information regarding the number of 
        slices is needed 
        """
        self.logger.info("Starting imager_create_dbs Node")

        # create the source db
        # returns zero on succes
        if self._create_bbs_sky_model(concatenated_measurement_set,
                 sourcedb_target_path, monet_db_hostname, monet_db_port, 
                 monet_db_name, monet_db_user, monet_db_password, assoc_theta):            
            return 1 
     
        # for each slice_set, create a parmdb in the target path
        # slice_paths is string representation of am array, convert
        slice_paths = eval(slice_paths)
        if self._create_parmdb_for_timeslices(parmdb_executable, slice_paths,
                                           parmdb_suffix):
            return 1

        return 0


    def _field_of_view(self, measurement_set, alpha_one = None):
        """
        _field_of_view calculates the fov, which is dependend on the
        station type, location and mode:
        For details see:        
        (1) http://www.astron.nl/radio-observatory/astronomers/lofar-imaging-capabilities-sensitivity/lofar-imaging-capabilities/lofa
        
        """
        # Open the ms
        t = pt.table(measurement_set)

        # Get antenna name and observation mode
        antenna = pt.table(t.getkeyword("ANTENNA"))
        antenna_name = antenna.getcell('NAME', 0)
        antenna.close()
        
        observation = pt.table(t.getkeyword("OBSERVATION"))
        antenna_set = observation.getcell('LOFAR_ANTENNA_SET', 0)
        observation.close
     
        #static parameters for the station diameters ref (1)     
        hba_core_diameter   = 30.8
        hba_remote_diameter = 41.1
        lba_inner           = 32.3
        lba_outer           = 81.3
        
        #use measurement set information to assertain antenna diameter
        station_diameter = None
        if antenna_name.count('HBA'):
            if antenna_name.count('CS'):
                station_diameter = hba_core_diameter
            elif antenna_name.count('RS'):
                station_diameter = hba_remote_diameter
        elif antenna_name.count('LBA'):
            if antenna_set.count('INNER'):
                station_diameter = lba_inner
            elif antenna_set.count('OUTER'):
                station_diameter = lba_outer 

        #raise exception if the antenna is not of a supported type
        if station_diameter == None:
            self.logger.error('Unknown antenna type for antenna: {0} , {1}'.format(\
                              antenna_name, antenna_set))
            raise Exception("Unknown antenna type encountered in Measurement set")
          
        #Get the wavelength
        spectral_window_table = pt.table(t.getkeyword("SPECTRAL_WINDOW"))
        freq = float(spectral_window_table.getcell("REF_FREQUENCY", 0))
        wave_length = pt.taql('CALC C()') / freq
        
        # Now calculate the FOV see ref (1)
        # alpha_one is a magic parameter: The value 1.3 is representative for a 
        # WSRT dish, where it depends on the dish illumination
        alpha_one = 1.3
        
        #alpha_one is in radians so transform to degrees for output
        fwhm = alpha_one * (wave_length / station_diameter) * (180 / math.pi)
        fov = fwhm / 2.0
        t.close()

        return fov
    

    def _create_parmdb(self, parmdb_executable, target_dir_path):
        """
        _create_parmdb, creates a parmdb_executable at the target_dir_path using the
        suplied executable. Does not test for existence of target parent dir       
        returns 1 if parmdb_executable failed 0 otherwise
        """
        # Format the template string by inserting the target dir
        formatted_template = template_parmdb.format(target_dir_path)
        try:
            # Spawn a subprocess and connect the pipelines
            parmdbm_process = subprocess.Popen(
                parmdb_executable,
                stdin = subprocess.PIPE,
                stdout = subprocess.PIPE,
                stderr = subprocess.PIPE
            )
            # Send formatted template on stdin
            sout, serr = parmdbm_process.communicate(formatted_template)

            # Log the output
            log_process_output("parmdbm", sout, serr, self.logger)
        except OSError, e:
            self.logger.error("Failed to spawn parmdbm: {0}".format(str(e)))
            return 1

        return 0


    def _create_parmdb_for_timeslices(self, parmdb_executable, slice_paths,
                                       suffix):
        """
        _create_parmdb_for_timeslices creates a paramdb for each of the
        supplied time slices. The paramdb path = input path + suffix.
        returns 0 on succes 1 on failure:
        """
        for slice_path in slice_paths:
            #Create the paths based on the 'source ms'
            ms_parmdb_path = slice_path + suffix 
            #call parmdb return failure if a single create failed 
            if self._create_parmdb(parmdb_executable, ms_parmdb_path) != 0:
                return 1

        return 0


    def _create_monet_db_connection(self, hostname, database, username, password,
                                    port):
        """
        Create and return a monat db connection. Return None if the creation 
        failed and log the error. Returns the connection if succeed.
        """
        try:
            conn = db.connect(hostname = hostname, database = database,
                                       username = username, password = password,
                                       port = port)
        except db.Error, e:
            self.logger.error("failed to create a monetDB connection: "
                              "{0}".format(str(e)))
            return None

        return conn


    def _get_ra_and_decl_from_ms(self, measurement_set):
        """
        This function uses pyrap to read the ra and declanation from a 
        measurement set (used by exprected_fluxes_in_fov). This is a position 
        in the sky. These values are stored in the field.phase_dir in the first
        row. All exceptions thrown are caught and logged, return None if reading
        failed
        """
        try:
            # open the ms, get the phase direction
            t = pt.table(measurement_set)
            t1 = pt.table(t.getkeyword("FIELD"))
            ra_and_decl = t1.getcell("PHASE_DIR", 0)
            t1.close()
            t.close()
        except Exception, e:
            #catch all exceptions and log
            self.logger.error("Error loading FIELD/PHASE_DIR from "
                              "measurementset {0} : {1}".format(measurement_set,
                                                                str(e)))
            return None

        # Return the ra and decl
        if len(ra_and_decl) != 2:
            self.logger.error("returned PHASE_DIR data did not contain two values")
            return None
        return (ra_and_decl[0], ra_and_decl[1])


    def _create_bbs_sky_model(self, measurement_set, path_output_skymap,
                              monet_db_host, monet_db_port, monet_db_name,
                              monet_db_user, monet_db_password,
                              assoc_theta = None):
        """
        Create a bbs sky model. Based on the measurement (set) suplied
        The skymap is created at the path_output_skymap
        """

        # Create monetdb connection
        conn = self._create_monet_db_connection(monet_db_host, monet_db_name,
                 monet_db_user, monet_db_password, monet_db_port)

        # get position of the target in the sky
        (ra_c, decl_c) = self._get_ra_and_decl_from_ms(measurement_set)

        # Get the Fov: sources in this fov should be included in the skumodel
        fov_radius = self._field_of_view(measurement_set)

        # !!magic constant!! This value is calculated based on communications with Bart Sheers       
        if assoc_theta == None:
            assoc_theta = 90.0 / 3600

        try:
            gsm.expected_fluxes_in_fov(conn, ra_c, decl_c, fov_radius, assoc_theta,
                                path_output_skymap, storespectraplots = False)
        except Exception, e:
            self.logger.error("expected_fluxes_in_fov raise exception: " +
                              str(e))
            return 1

        return 0


if __name__ == "__main__":
    # args contain information regarding to the logging server
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_create_dbs(jobid, jobhost, jobport).run_with_stored_arguments())

