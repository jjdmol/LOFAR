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
    def run(self, executable, environment, parset, working_directory,
            output_image, concatenated_measurement_set, sourcedb_path,
             mask_patch_size):
        """       
        :param executable: Path to awimager executable
        :param environment: environment for catch_segfaults (executable runner)
        :param parset: parameters for the awimager, 
        :param working_directory: directory the place temporary files
        :param output_image: location and filesname to story the output images
          the multiple images are appended with type extentions
        :param concatenated_measurement_set: Input measurement set
        :param sourcedb_path: Path the the sourcedb used to create the image 
          mask
        :param mask_patch_size: Scaling of the patch around the source in the 
          mask  
        :rtype: self.outputs["image"] The path to the output image
        
        """
        self.logger.info("Start imager_awimager node run:")
        log4_cplus_name = "imager_awimager"
        self.environment.update(environment)
        
        with log_time(self.logger):
            # ****************************************************************
            # 1. Calculate awimager parameters that depend on measurement set
            cell_size, npix, w_max, w_proj_planes = \
                self._calc_par_from_measurement(concatenated_measurement_set,
                                                 parset)

            # ****************************************************************
            # 2. Get the target image location from the mapfile for the parset.
            # Create target dir if it not exists
            image_path_head = os.path.dirname(output_image)
            create_directory(image_path_head)
            self.logger.debug("Created directory to place awimager output"
                              " files: {0}".format(image_path_head))

            # ****************************************************************
            # 3. Create the mask
            mask_file_path = self._create_mask(npix, cell_size, output_image,
                         concatenated_measurement_set, executable,
                         working_directory, log4_cplus_name, sourcedb_path,
                          mask_patch_size, image_path_head)

            # ******************************************************************
            # 4. Update the parset with calculated parameters, and output image
            patch_dictionary = {'uselogger': 'True', # enables log4cpluscd log
                               'ms': str(concatenated_measurement_set),
                               'cellsize': str(cell_size),
                               'npix': str(npix),
                               'wmax': str(w_max),
                               'wprojplanes':str(w_proj_planes),
                               'image':str(output_image),
                               'maxsupport':str(npix),
                               #'mask':str(mask_file_path),  #TODO REINTRODUCE 
                               # MASK, excluded to speed up in this debug stage
                               }

            # save the parset at the target dir for the image            
            temp_parset_filename = patch_parset(parset, patch_dictionary)
            calculated_parset_path = os.path.join(image_path_head,
                                                   "parset.par")
            # Copy tmp file to the final location
            shutil.copy(temp_parset_filename, calculated_parset_path)
            self.logger.debug("Wrote parset for awimager run: {0}".format(
                                                    calculated_parset_path))

            # *****************************************************************
            # 5. Run the awimager with the updated parameterset
            cmd = [executable, calculated_parset_path]
            try:
                with CatchLog4CPlus(working_directory,
                        self.logger.name + "." +
                        os.path.basename(log4_cplus_name),
                        os.path.basename(executable)
                ) as logger:
                    catch_segfaults(cmd, working_directory, self.environment,
                                            logger)

            # Thrown by catch_segfault
            except CalledProcessError, exception:
                self.logger.error(str(exception))
                return 1

            except Exception, exception:
                self.logger.error(str(exception))
                return 1

        # *********************************************************************
        # 6. Return output
        # Append static .restored: This might change but prob. not
        # The actual output image has this extention always, default of awimager
        self.outputs["image"] = output_image + ".restored"
        return 0

    def _calc_par_from_measurement(self, measurement_set, parset):
        """
        (1) calculate and format some parameters that are determined runtime. 
        Based  on values in the measurementset and input parameter (set):
        
        a. <string> The cellsize 
        b. <int> The npixels in a each of the two dimension of the image
        c. <string> The largest baseline in the ms smaller then the maxbaseline
        d. <string> The number of projection planes
                
        The calculation of these parameters is done in three steps:
        
        1. Calculate intermediate results based on the ms. 
        2. The calculation of the actual target values using intermediate result
        3. Scaling of cellsize and npix to allow for user input of the npix
        
        """
        # *********************************************************************
        # 1. Get partial solutions from the parameter set
        # Get the parset and a number of raw parameters from this parset
        parset_object = get_parset(parset)
        baseline_limit = parset_object.getInt('maxbaseline')
        # npix round up to nearest pow 2
        parset_npix = self._nearest_ceiled_power2(parset_object.getInt('npix'))

        # Get the longest baseline      
        sqrt_max_baseline = pt.taql(
                        'CALC sqrt(max([select sumsqr(UVW[:2]) from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory]))'.format(\
            measurement_set, baseline_limit *
            baseline_limit))[0]  #ask ger van diepen for details if ness.

        #Calculate the wave_length
        table_ms = pt.table(measurement_set)
        table_spectral_window = pt.table(table_ms.getkeyword("SPECTRAL_WINDOW"))
        freq = table_spectral_window.getcell("REF_FREQUENCY", 0)
        table_spectral_window.close()
        wave_length = pt.taql('CALC C()') / freq

        #Calculate the cell_size from the ms
        arc_sec_in_degree = 3600
        arc_sec_in_rad = (180.0 / math.pi) * arc_sec_in_degree
        cell_size = (1.0 / 3) * (wave_length / float(sqrt_max_baseline))\
             * arc_sec_in_rad

        # Calculate the number of pixels in x and y dim
        #    fov and diameter depending on the antenna name
        fov, station_diameter = self._get_fov_and_station_diameter(
                                                            measurement_set)

        # ********************************************************************
        # 2. Calculate the ms based output variables
        # 'optimal' npix based on measurement set calculations
        npix = (arc_sec_in_degree * fov) / cell_size
        npix = self._nearest_ceiled_power2(npix)

        # Get the max w with baseline < 10000
        w_max = pt.taql('CALC max([select UVW[2] from ' + \
            '{0} where sumsqr(UVW[:2]) <{1} giving as memory])'.format(
            measurement_set, baseline_limit * baseline_limit))[0]

        # Calculate number of projection planes
        w_proj_planes = min(257, math.floor((sqrt_max_baseline * wave_length) /
                                             (station_diameter ** 2)))

        w_proj_planes = int(round(w_proj_planes))
        self.logger.debug(
                    "Calculated w_max and the number pf projection plances:"
                    " {0} , {1}".format(w_max, w_proj_planes))

        # MAximum number of proj planes set to 1024: George Heald, Ger van 
        # Diepen if this exception occurs
        maxsupport = max(1024, npix)
        if w_proj_planes > maxsupport:
            raise Exception("The number of projections planes for the current" +
                            "measurement set is to large.")

        # *********************************************************************
        # 3. if the npix from the parset is different to the ms calculations,
        # calculate a sizeconverter value  (to be applied to the cellsize)
        size_converter = 1
        if npix != parset_npix:
            size_converter = npix / parset_npix
            npix = parset_npix

        if npix < 256:
            self.logger.warn("Using a image size smaller then 256x256:"
                " This leads to problematic imaging in some instances!!")

        cell_size_formatted = str(
                        int(round(cell_size * size_converter))) + 'arcsec'
        npix = int(npix)
        self.logger.info("Using the folowing image"
            " properties: npix: {0}, cell_size: {1}".format(
              npix, cell_size_formatted))
        return cell_size_formatted, npix, str(w_max), str(w_proj_planes)

    def _get_fov_and_station_diameter(self, measurement_set):
        """
        _field_of_view calculates the fov, which is dependend on the
        station type, location and mode:
        For details see:        
        (1) http://www.astron.nl/radio-observatory/astronomers/lofar-imaging-capabilities-sensitivity/lofar-imaging-capabilities/lofar
        
        """
        # Open the ms
        table_ms = pt.table(measurement_set)

        # Get antenna name and observation mode
        antenna = pt.table(table_ms.getkeyword("ANTENNA"))
        antenna_name = antenna.getcell('NAME', 0)
        antenna.close()

        observation = pt.table(table_ms.getkeyword("OBSERVATION"))
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
            self.logger.error(
                    'Unknown antenna type for antenna: {0} , {1}'.format(\
                              antenna_name, antenna_set))
            raise PipelineException(
                    "Unknown antenna type encountered in Measurement set")

        #Get the wavelength
        spectral_window_table = pt.table(table_ms.getkeyword("SPECTRAL_WINDOW"))
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
        table_ms.close()

        return fov, station_diameter

    def _create_mask(self, npix, cell_size, output_image,
                     concatenated_measurement_set, executable,
                     working_directory, log4_cplus_name, sourcedb_path,
                     mask_patch_size, image_path_directory):
        """
        (3) create a casa image containing an mask blocking out the
        sources in the provided sourcedb.
        
        It expects:
        
        a. the ms for which the mask will be created, it is used to de
           termine some image details: (eg. pointing)
        b. parameters for running within the catchsegfault framework
        c. and the size of the mask_pach.
           To create a mask, first a empty measurement set is created using
           awimager: ready to be filled with mask data 
           
        This function is a wrapper around some functionality written by:
        fdg@mpa-garching.mpg.de
        
        steps: 
        1. Create a parset with image paramters used by:
        2. awimager run. Creating an empty casa image.
        3. Fill the casa image with mask data
           
        """
        # ********************************************************************
        # 1. Create the parset used to make a mask
        mask_file_path = output_image + ".mask"

        mask_patch_dictionary = {"npix":str(npix),
                                 "cellsize":str(cell_size),
                                 "image":str(mask_file_path),
                                 "ms":str(concatenated_measurement_set),
                                 "operation":"empty",
                                 "stokes":"'I'"
                                 }
        mask_parset = Parset.fromDict(mask_patch_dictionary)
        mask_parset_path = os.path.join(image_path_directory, "mask.par")
        mask_parset.writeFile(mask_parset_path)
        self.logger.debug("Write parset for awimager mask creation: {0}".format(
                                                      mask_parset_path))

        # *********************************************************************
        # 2. Create an empty mask using awimager
        cmd = [executable, mask_parset_path]
        self.logger.info(" ".join(cmd))
        try:
            with CatchLog4CPlus(working_directory,
                    self.logger.name + "." + os.path.basename(log4_cplus_name),
                    os.path.basename(executable)
            ) as logger:
                catch_segfaults(cmd, working_directory, self.environment,
                                        logger)
        # Thrown by catch_segfault
        except CalledProcessError, exception:
            self.logger.error(str(exception))
            return 1
        except Exception, exception:
            self.logger.error(str(exception))
            return 1

        # ********************************************************************
        # 3. create the actual mask
        self.logger.debug("Started mask creation using mask_patch_size:"
                          " {0}".format(mask_patch_size))

        self._msss_mask(mask_file_path, sourcedb_path, mask_patch_size)
        self.logger.debug("Fished mask creation")
        return mask_file_path

    def _msss_mask(self, mask_file_path, sourcedb_path, mask_patch_size=1.0):
        """
        Fill casa image with a mask based on skymodel(sourcedb)
        Bugs: fdg@mpa-garching.mpg.de
        
        pipeline implementation klijn@astron.nl
        version 0.32
        
        Edited by JDS, 2012-03-16:
         - Properly convert maj/minor axes to half length
         - Handle empty fields in sky model by setting them to 0
         - Fix off-by-one error at mask boundary
        
        FIXED BUG
         - if a source is outside the mask, the script ignores it
         - if a source is on the border, the script draws only the inner part
         - can handle skymodels with different headers
        
        KNOWN BUG
         - not works with single line skymodels, workaround: add a fake
           source outside the field
         - mask patched display large amounts of aliasing. A possible 
           sollution would
           be normalizing to pixel centre. ( int(normalize_x * npix) /
           npix + (0.5 /npix)) 
           ideally the patch would increment in pixel radiuses
             
        Version 0.3  (Wouter Klijn, klijn@astron.nl)
         - Usage of sourcedb instead of txt document as 'source' of sources
           This allows input from different source sources
        Version 0.31  (Wouter Klijn, klijn@astron.nl)  
         - Adaptable patch size (patch size needs specification)
         - Patch size and geometry is broken: needs some astronomer magic to
           fix it, problem with afine transformation prol.
        Version 0.32 (Wouter Klijn, klijn@astron.nl)
         - Renaming of variable names to python convention        
        """
        pad = 500. # increment in maj/minor axes [arcsec]

        # open mask
        mask = pim.image(mask_file_path, overwrite=True)
        mask_data = mask.getdata()
        xlen, ylen = mask.shape()[2:]
        freq, stokes, null, null = mask.toworld([0, 0, 0, 0]) #@UnusedVariable


        #Open the sourcedb:
        table = pt.table(sourcedb_path + "::SOURCES")
        pdb = lofar.parmdb.parmdb(sourcedb_path)

        # Get the data of interest
        source_list = table.getcol("SOURCENAME")
        source_type_list = table.getcol("SOURCETYPE")
        # All date in the format valuetype:sourcename
        all_values_dict = pdb.getDefValues()

        # Loop the sources
        for source, source_type in zip(source_list, source_type_list):
            if source_type == 1:
                type_string = "Gaussian"
            else:
                type_string = "Point"
            self.logger.info("processing: {0} ({1})".format(source,
                                                             type_string))

            # Get de right_ascension and declination (already in radians)
            right_ascension = all_values_dict["Ra:" + source][0, 0]
            declination = all_values_dict["Dec:" + source][0, 0]
            if source_type == 1:
                # Get the raw values from the db
                maj_raw = all_values_dict["MajorAxis:" + source][0, 0]
                min_raw = all_values_dict["MinorAxis:" + source][0, 0]
                pa_raw = all_values_dict["Orientation:" + source][0, 0]
                #convert to radians (conversion is copy paste JDS)
                # major radius (+pad) in rad
                maj = (((maj_raw + pad)) / 3600.) * np.pi / 180.
                # minor radius (+pad) in rad
                minor = (((min_raw + pad)) / 3600.) * np.pi / 180.
                pix_asc = pa_raw * np.pi / 180.
                # wenss writes always 'GAUSSIAN' even for point sources 
                #-> set to wenss beam+pad
                if maj == 0 or minor == 0:
                    maj = ((54. + pad) / 3600.) * np.pi / 180.
                    minor = ((54. + pad) / 3600.) * np.pi / 180.
            elif source_type == 0: # set to wenss beam+pad
                maj = (((54. + pad) / 2.) / 3600.) * np.pi / 180.
                minor = (((54. + pad) / 2.) / 3600.) * np.pi / 180.
                pix_asc = 0.
            else:
                self.logger.info(
                    "WARNING: unknown source source_type ({0}),"
                    "ignoring: ".format(source_type))
                continue

            # define a small square around the source to look for it
            null, null, border_y1, border_x1 = mask.topixel(
                    [freq, stokes, declination - maj,
                      right_ascension - maj / np.cos(declination - maj)])
            null, null, border_y2, border_x2 = mask.topixel(
                    [freq, stokes, declination + maj,
                     right_ascension + maj / np.cos(declination + maj)])
            xmin = np.int(np.floor(np.min([border_x1, border_x2])))
            xmax = np.int(np.ceil(np.max([border_x1, border_x2])))
            ymin = np.int(np.floor(np.min([border_y1, border_y2])))
            ymax = np.int(np.ceil(np.max([border_y1, border_y2])))

            if xmin > xlen or ymin > ylen or xmax < 0 or ymax < 0:
                self.logger.info(
                    "WARNING: source {0} falls outside the mask,"
                    " ignoring: ".format(source))
                continue

            if xmax > xlen or ymax > ylen or xmin < 0 or ymin < 0:
                self.logger.info(
                    "WARNING: source {0} falls across map edge".format(source))

            for pixel_x in xrange(xmin, xmax):
                for pixel_y in xrange(ymin, ymax):
                    # skip pixels outside the mask field
                    if pixel_x >= xlen or pixel_y >= ylen or\
                       pixel_x < 0 or pixel_y < 0:
                        continue
                    # get pixel right_ascension and declination in rad
                    null, null, pix_dec, pix_ra = mask.toworld(
                                                    [0, 0, pixel_y, pixel_x])
                    # Translate and rotate coords.
                    translated_pixel_x = (pix_ra - right_ascension) * np.sin(
                        pix_asc) + (pix_dec - declination) * np.cos(pix_asc)
                    # to align with ellipse
                    translate_pixel_y = -(pix_ra - right_ascension) * np.cos(
                        pix_asc) + (pix_dec - declination) * np.sin(pix_asc)
                    if (((translated_pixel_x ** 2) / (maj ** 2)) +
                        ((translate_pixel_y ** 2) / (minor ** 2))) < \
                                                         mask_patch_size:
                        mask_data[0, 0, pixel_y, pixel_x] = 1
        null = null
        mask.putdata(mask_data)
        table.close()

    # some helper functions
    def _nearest_ceiled_power2(self, value):
        """
        Return int value of  the nearest Ceiled power of 2 for the 
        suplied argument
        
        """
        return int(pow(2, math.ceil(math.log(value, 2))))


if __name__ == "__main__":
    _JOBID, _JOBHOST, _JOBPORT = sys.argv[1:4]
    sys.exit(imager_awimager(
                    _JOBID, _JOBHOST, _JOBPORT).run_with_stored_arguments())
