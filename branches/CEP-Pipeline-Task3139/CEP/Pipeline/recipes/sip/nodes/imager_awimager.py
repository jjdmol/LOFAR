# LOFAR AUTOMATIC IMAGING PIPELINE
# awimager
# The awimager recipe creates based an image of the field of view. Based on  
# nine concatenated and measurementsets each spanning 10 subbands
# The recipe contains two parts: The call to awimager
# and secondairy some functionality that calculates settings (for awimager)
# based on the information present in the measurement set
# The calculated parameters are:
#        1: The cellsize
#        2: The npixels in a each of the two dimension of the image
#        3. What columns use to determine the maximum baseline
#        4. The number of projection planes
# Wouter Klijn 2012
# klijn@astron.nl
# -----------------------------------------------------------------------------
from __future__ import with_statement
import sys
import shutil
import os.path
import math

from lofarpipe.support.lofarnode import LOFARnodeTCP
from lofarpipe.support.pipelinelogging import CatchLog4CPlus
from lofarpipe.support.pipelinelogging import log_time
from lofarpipe.support.utilities import patch_parset
from lofarpipe.support.utilities import get_parset
from lofarpipe.support.utilities import read_initscript
from lofarpipe.support.utilities import catch_segfaults
from lofarpipe.support.lofarexceptions import PipelineException
import pyrap.tables as pt                   #@UnresolvedImport
from subprocess import CalledProcessError
from lofarpipe.support.utilities import create_directory
import pyrap.images as pim                   #@UnresolvedImport
from lofarpipe.support.parset import Parset
import lofar.parmdb                          #@UnresolvedImport
import numpy as np

class imager_awimager(LOFARnodeTCP):
    def run(self, executable, init_script, parset, working_directory,
            output_image, concatenated_measurement_set, sourcedb_path, mask_patch_size):
        self.logger.info("Start imager_awimager  run: client")
        log4CPlusName = "imager_awimager"
        with log_time(self.logger):
            size_converter = 1.0 #TODO debugging tool scale the image and cellsize to allow quicker running of the awimager
            # Calculate awimager parameters that depend on measurement set                 
            cell_size, npix, w_max, w_proj_planes = \
                self._calc_par_from_measurement(concatenated_measurement_set, parset, size_converter)



            # Get the target image location from the mapfile for the parset.
            # Create target dir
            # if it not exists
            image_path_head = os.path.dirname(output_image)
            create_directory(image_path_head)
            self.logger.debug("Created directory to place awimager output"
                              " files: {0}".format(image_path_head))

            mask_file_path = self._create_mask(npix, cell_size, output_image,
                         concatenated_measurement_set, init_script, executable,
                         working_directory, log4CPlusName, sourcedb_path,
                          mask_patch_size, image_path_head)
            # The max support should always be a minimum of 1024 (Ger van Diepen)
            maxsupport = max(1024, npix)

            # Update the parset with calculated parameters, and output image
            patch_dictionary = {'uselogger': 'True', # enables log4cpluscd log
                               'ms': str(concatenated_measurement_set),
                               'cellsize': str(cell_size),
                               'npix': str(npix),
                               'wmax': str(w_max),
                               'wprojplanes':str(w_proj_planes),
                               'image':str(output_image),
                               'maxsupport':str(npix),
                               #'mask':str(mask_file_path),  #TODO REINTRODUCE MASK, excluded to speed up in this debug stage
                               }

            # save the parset at the target dir for the image            
            temp_parset_filename = patch_parset(parset, patch_dictionary)
            calculated_parset_path = os.path.join(image_path_head, "parset.par")
            shutil.copy(temp_parset_filename, calculated_parset_path)
            os.unlink(temp_parset_filename)
            self.logger.debug("Wrote parset for awimager run: {0}".format(
                                                    calculated_parset_path))

            # The command and parameters to be run
            cmd = [executable, calculated_parset_path]
            try:
                environment = read_initscript(self.logger, init_script)
                with CatchLog4CPlus(working_directory,
                        self.logger.name + "." + os.path.basename(log4CPlusName),
                        os.path.basename(executable)
                ) as logger:
                    catch_segfaults(cmd, working_directory, environment,
                                            logger)

            # Thrown by catch_segfault
            except CalledProcessError, e:
                self.logger.error(str(e))
                return 1

            except Exception, e:
                self.logger.error(str(e))
                return 1

        # TODO Append static .restored: This might change but prob. not
        self.outputs["image"] = output_image + ".restored"
        return 0

    def _create_mask(self, npix, cell_size, output_image,
                     concatenated_measurement_set, init_script, executable,
                     working_directory, log4CPlusName, sourcedb_path,
                     mask_patch_size, image_path_image_cycle):
        """
        _create_mask creates a casa image containing an mask blocking out the
        sources in the provided sourcedb.
        It expects the ms for which the mask will be created. enviroment 
        parameters for running within the catchsegfault framework and
        finaly the size of the mask_pach.
        To create a mask, first a empty measurement set is created using
        awimager: ready to be filled with mask data        
        """
        #Create an empty mask using awimager
        # Create the parset used to make a mask
        mask_file_path = output_image + ".mask"

        mask_patch_dictionary = {"npix":str(npix),
                                 "cellsize":str(cell_size),
                                 "image":str(mask_file_path),
                                 "ms":str(concatenated_measurement_set),
                                 "operation":"empty",
                                 "stokes":"'I'"
                                 }
        mask_parset = Parset.fromDict(mask_patch_dictionary)
        mask_parset_path = os.path.join(image_path_image_cycle, "mask.par")
        mask_parset.writeFile(mask_parset_path)
        self.logger.debug("Write parset for awimager mask creation: {0}".format(
                                                      mask_parset_path))

        # The command and parameters to be run
        cmd = [executable, mask_parset_path]
        self.logger.info(" ".join(cmd))
        try:
            environment = read_initscript(self.logger, init_script)
            with CatchLog4CPlus(working_directory,
                    self.logger.name + "." + os.path.basename(log4CPlusName),
                    os.path.basename(executable)
            ) as logger:
                catch_segfaults(cmd, working_directory, environment,
                                        logger)
        # Thrown by catch_segfault
        except CalledProcessError, e:
            self.logger.error(str(e))
            return 1
        except Exception, e:
            self.logger.error(str(e))
            return 1

        self.logger.debug("Started mask creation using mask_patch_size:"
                          " {0}".format(mask_patch_size))
        # create the actual mask
        self._msss_mask(mask_file_path, sourcedb_path, mask_patch_size)
        self.logger.debug("Fished mask creation")
        return mask_file_path

    def _msss_mask(self, mask_file_path, sourcedb_path, mask_patch_size = 1.0):
        """
        Fill a mask based on skymodel
        Usage: ./msss_mask.py mask-file skymodel
        inputs:wenss-2048-15.mask skymodel.dat
        Bugs: fdg@mpa-garching.mpg.de
              pipeline implementation klijn@astron.nl
        version 0.3
        
         Edited by JDS, 2012-03-16:
         * Properly convert maj/minor axes to half length
         * Handle empty fields in sky model by setting them to 0
         * Fix off-by-one error at mask boundary
        
         FIXED BUG
         * if a source is outside the mask, the script ignores it
         * if a source is on the border, the script draws only the inner part
         * can handle skymodels with different headers
        
         KNOWN BUG
         * not works with single line skymodels, workaround: add a fake source outside the field
         * mask patched display large amounts of aliasing. A possible sollution would
           be normalizing to pixel centre. ( int(normalize_x * npix) / npix + (0.5 /npix)) 
           ideally the patch would increment in pixel radiuses
             
         Version 0.3  (Wouter Klijn, klijn@astron.nl)
         * Usage of sourcedb instead of txt document as 'source' of sources
           This allows input from different source sources
         Version 0.31  (Wouter Klijn, klijn@astron.nl)  
         * Adaptable patch size (patch size needs specification)
         * Patch size and geometry is broken: needs some astronomer magic to fix it, problem with afine transformation prol.
        """
        pad = 500. # increment in maj/minor axes [arcsec]

        # open mask
        mask = pim.image(mask_file_path, overwrite = True)
        mask_data = mask.getdata()
        xlen, ylen = mask.shape()[2:]
        freq, stokes, null, null = mask.toworld([0, 0, 0, 0]) #@UnusedVariable


        #Open the sourcedb:
        table = pt.table(sourcedb_path + "::SOURCES")
        pdb = lofar.parmdb.parmdb(sourcedb_path)

        # Get the data of interest
        source_list = table.getcol("SOURCENAME")
        source_type_list = table.getcol("SOURCETYPE")
        all_values_dict = pdb.getDefValues()  # All date in the format valuetype:sourcename

        # Loop the sources
        for source, source_type in zip(source_list, source_type_list):
            if source_type == 1:
                type_string = "Gaussian"
            else:
                type_string = "Point"
            self.logger.info("processing: {0} ({1})".format(source, type_string))

            # Get de ra and dec (already in radians)
            ra = all_values_dict["Ra:" + source][0, 0]
            dec = all_values_dict["Dec:" + source][0, 0]
            if source_type == 1:
                # Get the raw values from the db
                maj_raw = all_values_dict["MajorAxis:" + source][0, 0]
                min_raw = all_values_dict["MinorAxis:" + source][0, 0]
                pa_raw = all_values_dict["Orientation:" + source][0, 0]
                #convert to radians (conversion is copy paste JDS)
                maj = (((maj_raw + pad)) / 3600.) * np.pi / 180. # major radius (+pad) in rad
                minor = (((min_raw + pad)) / 3600.) * np.pi / 180. # minor radius (+pad) in rad
                pa = pa_raw * np.pi / 180.
                if maj == 0 or minor == 0: # wenss writes always 'GAUSSIAN' even for point sources -> set to wenss beam+pad
                    maj = ((54. + pad) / 3600.) * np.pi / 180.
                    minor = ((54. + pad) / 3600.) * np.pi / 180.
            elif source_type == 0: # set to wenss beam+pad
                maj = (((54. + pad) / 2.) / 3600.) * np.pi / 180.
                minor = (((54. + pad) / 2.) / 3600.) * np.pi / 180.
                pa = 0.
            else:
                self.logger.info("WARNING: unknown source source_type ({0}), ignoring it.".format(source_type))
                continue

            #print "Maj = ", maj*180*3600/np.pi, " - Min = ", minor*180*3600/np.pi # DEBUG

            # define a small square around the source to look for it
            null, null, y1, x1 = mask.topixel([freq, stokes, dec - maj, ra - maj / np.cos(dec - maj)])
            null, null, y2, x2 = mask.topixel([freq, stokes, dec + maj, ra + maj / np.cos(dec + maj)])
            xmin = np.int(np.floor(np.min([x1, x2])))
            xmax = np.int(np.ceil(np.max([x1, x2])))
            ymin = np.int(np.floor(np.min([y1, y2])))
            ymax = np.int(np.ceil(np.max([y1, y2])))

            if xmin > xlen or ymin > ylen or xmax < 0 or ymax < 0:
                self.logger.info("WARNING: source {0} falls outside the mask, ignoring it.".format(source))
                continue

            if xmax > xlen or ymax > ylen or xmin < 0 or ymin < 0:
                self.logger.info("WARNING: source {0} falls across map edge.".format(source))
                pass


            for x in xrange(xmin, xmax):
                for y in xrange(ymin, ymax):
                    # skip pixels outside the mask field
                    if x >= xlen or y >= ylen or x < 0 or y < 0:
                        continue
                    # get pixel ra and dec in rad
                    null, null, pix_dec, pix_ra = mask.toworld([0, 0, y, x])

                    X = (pix_ra - ra) * np.sin(pa) + (pix_dec - dec) * np.cos(pa); # Translate and rotate coords.
                    Y = -(pix_ra - ra) * np.cos(pa) + (pix_dec - dec) * np.sin(pa); # to align with ellipse
                    if (((X ** 2) / (maj ** 2)) +
                        ((Y ** 2) / (minor ** 2))) < mask_patch_size:
                        mask_data[0, 0, y, x] = 1
        null = null
        mask.putdata(mask_data)
        table.close()

    def _nearest_ceiled_power2(self, value):
        '''
        Return int value of  the nearest Ceiled power of 2 for the 
        suplied argument
        
        '''
#        TODO: This needs to be adapted to provide a size that is power1
#        after cropping
        return int(pow(2, math.ceil(math.log(value, 2))))

    def _field_of_view_and_station_diameter(self, measurement_set):
        """
        _field_of_view calculates the fov, which is dependend on the
        station type, location and mode:
        For details see:        
        (1) http://www.astron.nl/radio-observatory/astronomers/lofar-imaging-capabilities-sensitivity/lofar-imaging-capabilities/lofar
        
        """
        # Open the ms
        t = pt.table(measurement_set)

        # Get antenna name and observation mode
        antenna = pt.table(t.getkeyword("ANTENNA"))
        antenna_name = antenna.getcell('NAME', 0)
        antenna.close()

        observation = pt.table(t.getkeyword("OBSERVATION"))
        antenna_set = observation.getcell('LOFAR_ANTENNA_SET', 0)
        observation.close()

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
            self.logger.error('Unknown antenna type for antenna: {0} , {1}'.format(\
                              antenna_name, antenna_set))
            raise PipelineException("Unknown antenna type encountered in Measurement set")

        #Get the wavelength
        spectral_window_table = pt.table(t.getkeyword("SPECTRAL_WINDOW"))
        freq = float(spectral_window_table.getcell("REF_FREQUENCY", 0))
        wave_length = pt.taql('CALC C()') / freq
        spectral_window_table.close()

        # Now calculate the FOV see ref (1)
        # alpha_one is a magic parameter: The value 1.3 is representative for a 
        # WSRT dish, where it depends on the dish illumination
        alpha_one = 1.3

        #alpha_one is in radians so transform to degrees for output
        fwhm = alpha_one * (wave_length / station_diameter) * (180 / math.pi)
        fov = fwhm / 2.0
        t.close()

        return fov, station_diameter

    def _calc_par_from_measurement(self, measurement_set, parset, size_converter):
        """
        calculate and format some parameters that are determined runtime based
        on values in the measurement set:
        1: The cellsize
        2: The npixels in a each of the two dimension of the image
        3. What columns use to determine the maximum baseline
        4. The number of projection planes (if > 512 the ask George heald 
        
        """
        baseline_limit = get_parset(parset).getInt('maxbaseline')

        arc_sec_in_degree = 3600
        arc_sec_in_rad = (180.0 / math.pi) * arc_sec_in_degree

        # Calculate the cell_size         
        max_baseline = pt.taql('CALC sqrt(max([select sumsqr(UVW[:2]) from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory]))'.format(\
            measurement_set, baseline_limit *
            baseline_limit))[0]  #ask ger van diepen for details if ness.
        self.logger.debug("Calculated maximum baseline: {0}".format(
                                                            max_baseline))
        t = pt.table(measurement_set)
        t1 = pt.table(t.getkeyword("SPECTRAL_WINDOW"))
        freq = t1.getcell("REF_FREQUENCY", 0)
        waveLength = pt.taql('CALC C()') / freq
        t1.close()

        cell_size = (1.0 / 3) * (waveLength / float(max_baseline)) * arc_sec_in_rad
        self.logger.debug("Calculated cellsize baseline: {0}".format(
                                                            cell_size))

        # Calculate the number of pixels in x and y dim
        #    fov and diameter depending on the antenna name
        fov, station_diameter = self._field_of_view_and_station_diameter(measurement_set)
        self.logger.debug("Calculated fov and station diameter baseline:"
                          " {0} , {1}".format(fov, station_diameter))

        npix = (arc_sec_in_degree * fov) / cell_size
        npix = self._nearest_ceiled_power2(npix)

        # Get the max w with baseline < 10000
        w_max = pt.taql('CALC max([select UVW[2] from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory])'.format(
            measurement_set, baseline_limit * baseline_limit))[0]

        # Calculate number of projection planes
        w_proj_planes = min(257, math.floor((max_baseline * waveLength) /
                                             (station_diameter ** 2)))

        w_proj_planes = int(round(w_proj_planes))
        self.logger.debug("Calculated w_max and the number pf projection plances:"
                          " {0} , {1}".format(w_max, w_proj_planes))

        if w_proj_planes > 511:
            raise Exception("The number of projections planes for the current" +
                            "measurement set is to large.")  #FIXME: Ask george 
        # Do debugging size conversion ( to decrease image size for fater testing)
        if npix <= 256: #Do not make small images smaller
            size_converter = 1
        elif npix == 512: #only increase one size step for 512 npix
            size_converter = min(2, size_converter)
        cell_size_formatted = str(int(round(cell_size * size_converter))) + 'arcsec'
        npix = int(float(npix) / size_converter)
        return cell_size_formatted, npix, str(w_max), str(w_proj_planes)


if __name__ == "__main__":
    jobid, jobhost, jobport = sys.argv[1:4]
    sys.exit(imager_awimager(jobid, jobhost, jobport).run_with_stored_arguments())
