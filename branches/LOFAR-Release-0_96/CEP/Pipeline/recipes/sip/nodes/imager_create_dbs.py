# LOFAR AUTOMATIC IMAGING PIPELINE
# imager_create_dbs (node)
# Wouter Klijn 2012
# klijn@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys
import subprocess
import math
import shutil
import pyrap.tables as pt                                                       #@UnresolvedImport
import os

from subprocess import CalledProcessError
from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import log_process_output
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import catch_segfaults
import monetdb.sql as db
import lofar.gsm.gsmutils as gsm                                                #@UnresolvedImport

#TODO: A better place for this template
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
    """
    create dbs
    The create dbs recipe is responcible for settings up database
    for subsequenty imaging steps. It creates two databases in three steps
    1. sourcedb. 
      On the first major imaging cycle filled by the gsm. sourcefinding in the 
      in the later steps sources found are append to the current list
      Current patch of the sky. It is filled with an initial started set of
      sources created by  the Global Sky Model (GSM). 
      The GSM does not create a sourceDB. It creates a text file which is con-
      sumed by makesourcedb resulting in a sourceDB (casa table)
      There is a single sourcedb for a measurement set
   2. parmdb
      Each individual timeslice needs a place to collect parameters: This is
      done in the paramdb. 
    """
    def run(self, concatenated_measurement_set, sourcedb_target_path,
            monet_db_hostname, monet_db_port, monet_db_name, monet_db_user,
            monet_db_password, assoc_theta, parmdb_executable, slice_paths,
            parmdb_suffix, init_script, working_directory, makesourcedb_path,
            source_list_path_extern):

        self.logger.info("Starting imager_create_dbs Node")

        # If a (local) sourcelist is received use it else
        # construct one
        if source_list_path_extern == "":
            #create a temporary file to contain the skymap
            source_list = sourcedb_target_path + ".temp"
            if self._fill_soucelist_based_on_gsm_sky_model(
                    concatenated_measurement_set,
                    source_list, monet_db_hostname, monet_db_port,
                    monet_db_name, monet_db_user, monet_db_password, assoc_theta):
                self.logger.error("failed creating skymodel")
                return 1
            append = False
        else:
            source_list = source_list_path_extern
            append = True

        # convert it to a sourcedb (casa table)
        if self._create_source_db(source_list, sourcedb_target_path,
                                  init_script, working_directory,
                                  makesourcedb_path, append):
            self.logger.error("failed creating sourcedb")
            return 1

        self.outputs["sourcedb"] = sourcedb_target_path

        if self._create_parmdb_for_timeslices(parmdb_executable, slice_paths,
                                           parmdb_suffix):
            self.logger.error("failed creating paramdb for slices")
            return 1


        return 0

    def _create_source_db(self, source_list, sourcedb_target_path, init_script,
                          working_directory, executable, append = False):
        """
        _create_source_db consumes a skymap text file and produces a source db
        (pyraptable) 
        """
        #remove existing sourcedb if not appending
        if (append == False) and os.path.isdir(sourcedb_target_path):
            shutil.rmtree(sourcedb_target_path)
            self.logger.debug("Removed existing sky model: {0}".format(
                                            sourcedb_target_path))

        # The command and parameters to be run
        cmd = [executable, "in={0}".format(source_list),
               "out={0}".format(sourcedb_target_path),
               "format=<", # format according to Ger van Diepen
               "append=true"] # Always set append flag: no effect on non exist db

        try:
            environment = read_initscript(self.logger, init_script)
            with CatchLog4CPlus(working_directory,
                 self.logger.name + "." + os.path.basename("makesourcedb"),
                 os.path.basename(executable)
            ) as logger:
                    catch_segfaults(cmd, working_directory, environment,
                                            logger, cleanup = None)

        except Exception, e:
            self.logger.error("Execution of external failed:")
            self.logger.error(" ".join(cmd))
            self.logger.error("exception details:")
            self.logger.error(str(e))
            return 1

        return 0


    def _field_of_view(self, measurement_set, alpha_one = None):
        """
        _field_of_view calculates the fov, which is dependend on the
        station type, location and mode:
        For details see:        
        (1) http://www.astron.nl/radio-observatory/astronomers/lofar-imaging-capabilities-sensitivity/lofar-imaging-capabilities/lofar
        
        """
        # Open the ms
        try:
            t = pt.table(measurement_set)

            # Get antenna name and observation mode
            antenna = pt.table(t.getkeyword("ANTENNA"))
            antenna_name = antenna.getcell('NAME', 0)

            observation = pt.table(t.getkeyword("OBSERVATION"))
            antenna_set = observation.getcell('LOFAR_ANTENNA_SET', 0)
            observation.close

            #static parameters for the station diameters ref (1)     
            hba_core_diameter = 30.8
            hba_remote_diameter = 41.1
            lba_inner = 32.3
            lba_outer = 81.3

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
                self.logger.error('Unknown antenna type for antenna: {0} , {1}'.format(
                                  antenna_name, antenna_set))
                raise Exception("Unknown antenna type encountered in Measurement set")

            #Get the wavelength
            spectral_window_table = pt.table(t.getkeyword("SPECTRAL_WINDOW"))
            freq = float(spectral_window_table.getcell("REF_FREQUENCY", 0))
            wave_length = pt.taql('CALC C()')[0] / freq

            # Now calculate the FOV see ref (1)
            # alpha_one is a magic parameter: The value 1.3 is representative for a 
            # WSRT dish, where it depends on the dish illumination
            # For LOFAR it will depend on the final tapering of the station.
            # For the LBA probably no tapering will be applied. In that case it
            # is expected that the value of a1 will turn out to be between 1.2 
            # and 1.4. For reference, the value for the LOFAR Initial Test 
            # Station (ITS) was 1.42. 
            alpha_one = 1.3

            #alpha_one is in radians so transform to degrees for output
            fwhm = alpha_one * (wave_length / station_diameter) * (180 / math.pi)
            fov = fwhm / 2.0
        finally:
            antenna.close()
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
        parmdbms = []
        for slice_path in slice_paths:
            #Create the paths based on the 'source ms'
            ms_parmdb_path = slice_path + suffix
            parmdbms.append(ms_parmdb_path)
            #call parmdb return failure if a single create failed 
            if self._create_parmdb(parmdb_executable, ms_parmdb_path) != 0:
                return 1
        self.outputs["parmdbms"] = parmdbms
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
            self.logger.error("Failed to create a monetDB connection: "
                              "{0}".format(str(e)))
            raise e

        return conn


    def _get_ra_and_decl_from_ms(self, measurement_set):
        """
        This function uses pyrap to read the ra and declanation from a 
        measurement set (used by expected_fluxes_in_fov). This is a position 
        in the sky. These values are stored in the field.phase_dir in the first
        row. All exceptions thrown are caught and logged, return None if reading
        failed
        """
        try:
            # open the ms, get the phase direction
            t = pt.table(measurement_set)
            t1 = pt.table(t.getkeyword("FIELD"))
            ra_and_decl = t1.getcell("PHASE_DIR", 0)[0]

        except Exception, e:
            #catch all exceptions and log
            self.logger.error("Error loading FIELD/PHASE_DIR from "
                              "measurementset {0} : {1}".format(measurement_set,
                                                                str(e)))
            raise e

        finally:
            t1.close()
            t.close()

        # Return the ra and decl
        if len(ra_and_decl) != 2:
            self.logger.error("returned PHASE_DIR data did not contain two values")
            return None

        return (ra_and_decl[0], ra_and_decl[1])


    def _fill_soucelist_based_on_gsm_sky_model(self, measurement_set, sourcelist,
                              monet_db_host, monet_db_port, monet_db_name,
                              monet_db_user, monet_db_password,
                              assoc_theta = None):
        """
        Create a bbs sky model. Based on the measurement (set) suplied
        The skymap is created at the sourcelist
        """
        # Create monetdb connection
        conn = self._create_monet_db_connection(monet_db_host, monet_db_name,
                 monet_db_user, monet_db_password, monet_db_port)
        self.logger.debug("Connected to monet db at: {0}:{1}  {2}".format(
                monet_db_host, monet_db_port, monet_db_name))

        # get position of the target in the sky
        (ra_c, decl_c) = self._get_ra_and_decl_from_ms(measurement_set)
        self.logger.debug("ra and dec from measurement set: {0}, {1}".format(
                    ra_c, decl_c))

        # Get the Fov: sources in this fov should be included in the skumodel
        fov_radius = self._field_of_view(measurement_set)
        self.logger.debug("Using the folowing calculated field of view: {0}".format(
                    fov_radius))

        # !!magic constant!! This value is calculated based on        
        # communications with Bart Sheers
        if assoc_theta == None:
            assoc_theta = 90.0 / 3600
        try:
            # Transform the ra and decl to rad
            ra_c = float(ra_c) * (180 / math.pi)
            if ra_c < 0:  #gsm utils break when using negative ra_c ergo add 360
                ra_c += 360.0
            decl_c = float(decl_c) * (180 / math.pi)

            gsm.expected_fluxes_in_fov(conn, ra_c ,
                        decl_c, float(fov_radius),
                        float(assoc_theta), sourcelist,
                        storespectraplots = False)
        except Exception, e:
            self.logger.error("expected_fluxes_in_fov raise exception: " +
                              str(e))
            return 1

        return 0


if __name__ == "__main__":
    # args contain information regarding to the logging server
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_create_dbs(jobid, jobhost, jobport).run_with_stored_arguments())
