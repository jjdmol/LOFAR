"""Version module.

This module simply stores the version and svn revision numbers, as well
as a changelog. The svn revision number will be updated automatically
whenever there is a change to this file. However, if no change is made
to this file, the revision number will get out of sync. Therefore, one
must update this file with each (significant) update of the code: 
adding to the changelog will naturally do this.
"""

# Version number
__version__ = '1.1'

# Store svn Revision number. For this to work, one also
# needs to do: 
# "svn propset svn:keywords Revision src/Anaamika/implement/PyBDSM/python/_version.py" 
# from the LOFARSOFT directory. Then, the revision number is
# added automatically with each update to this file. The line below does not
# need to be edited by hand.
__revision__ = filter(str.isdigit, "$Revision$")


# Change log
def changelog():
    """
    PyBDSM Changelog.
    -----------------------------------------------------------------------
    
    2012/04/20 - Promoted the adaptive_rms_box parameter to
                 the main options listing and added the 
                 rms_box_bright option so that the user can
                 specify either (or both) of the rms_boxes.
                 Fixed bug in wavelet module so that invalid
                 Gaussians (i.e., those that lie outside of
                 islands in the ch0 image) are not used when
                 making the residual images at each scale. 
                 Improved speed of Gaussian fitting to wavelet
                 images. Fixed bug that resulted in pixels found 
                 to be outside the universe (check is enabled 
                 with the check_outsideuniv option) not being 
                 masked properly. 
    
    2012/04/17 - Fixed bug in psf_vary module that resulted in
                 PSF major and minor axis maps in terms of 
                 sigma instead of FWHM. Added psf_vary option
                 (psf_stype_only) to allow PSF fitting to non-
                 S-type sources (useful if sources are very 
                 distorted).
    
    2012/04/12 - Fixed bug in adaptive scaling code that could 
                 cause incorrect small-scale rms_box size. Added
                 a parameter (adaptive_thresh) that controls the
                 minimum threshold for sources used to set the
                 small-scale rms_box size.
    
    2012/04/02 - Implemented an adaptive scaling scheme for 
                 the rms_box parameter that shrinks the box
                 size near bright sources and expands it far
                 from them (enabled with the adaptive_rms_box
                 option when rms_box=None). This scheme generally 
                 results in improved rms and mean maps when both 
                 strong artifacts and extended sources are present.
                 Fixed bug that prevented plotting of results 
                 during wavelet decomposition when interactive =
                 True.
    
    2012/03/29 - Fixed bug in wavelet module that could cause 
                 incorrect associations of Gaussians. Fixed
                 bug in show_fit that displayed incorrect
                 model and residual images when wavelets were
                 used. 
    
    2012/03/28 - Version 1.1
    
    2012/03/28 - Fixed bug that caused mask to be ignored when
                 determining whether variations in rms and mean
                 maps is significant. Fixed bug that caused
                 internally derived rms_box value to be ignored.
    
    2012/03/27 - Modified calculation of rms_box parameter (when 
                 rms_box option is None) to work better with fields 
                 composed mainly of point sources when strong 
                 artifacts are present. Tweaked flagging on FWHM
                 to prevent over-flagging of Gaussians in small
                 islands. Changed wavelet module to flag Gaussians
                 whose centers fall outside of islands found in 
                 the original image and removed atrous_orig_isl
                 option (as redundant).
    
    2012/03/26 - Modified fitting of large islands to adopt an
                 iterative fitting scheme that limits the number
                 of Gaussians fit simultaneously per iteration to 10.
                 This change speeds up fitting of large islands
                 considerably. The options peak_fit and peak_maxsize
                 control whether iterative fitting is done. Added new 
                 Gaussian flagging condition (flag_maxsize_fwhm) that 
                 flags Gaussians whose sigma contour times factor 
                 extends beyond the island boundary. This flag prevents
                 fitting of Gaussians that extend far beyond the island 
                 boundary.
    
    2012/03/23 - Tweaked settings that affect fitting of Gaussians to
                 improve fitting in general. 
    
    2012/03/19 - Added output of shapelet parameters to FITS tables.
                 Fixed issue with resizing of sources in spectral
                 index module.
    
    2012/03/16 - Fixed bugs in polarisation module that caused incorrect
                 polarization fractions.
    
    2012/03/13 - Improved plotting speed (by factor of ~ 4) in show_fit 
                 when there is a large number of islands. Simplified the
                 spectral index module to make it more user friendly and
                 stable. Added the option to use a "detection" image for
                 island detection (the detection_image option); source
                 properties are still measured from the main input image.
    
    2012/03/01 - Fixed a bug in the polarisation module that could result
                 in incorrect fluxes. Changed logging module to suppress
                 output of ANSI color codes to the log file.
    
    2012/02/27 - Implemented fitting of Gaussians in polarisation module,
                 instead of simple summation of pixel values, to determine
                 polarized fluxes. 
    
    2012/02/17 - In scripts, process_image() will now accept a 
                 dictionary of parameters as input.

    2012/02/10 - Sources that appear only in Stokes Q or U (and 
                 hence not in Stokes I) are now identified and included 
                 in the polarisation module. This identification is done
                 using the polarized intensity (PI) image. show_fit() and 
                 export_image() were updated to allow display and export
                 of the PI image.
                 
    2012/02/06 - Fixed bug in island splitting code that could result in 
                 duplicate Gaussians. 

    2012/02/02 - Improved polarisation module. Polarization quantities are
                 now calculated for Gaussians as well as sources.

    2012/01/27 - Fixed bug in psf_vary module that affected tesselation. 
                 Fixed many small typos in parameter descriptions.

    2012/01/18 - Fixed a bug that resulted in incorrect coordinates when
                 the trim_box option was used with a CASA image. Added 
                 option (blank_zeros) to blank pixels in the input image
                 that are exactly zero.

    2012/01/13 - Fixed minor bugs in the interactive shell and updated 
                 pybdsm.py to support IPython 0.12.
                 
    2011/12/21 - Fixed bug in gaul2srl module due to rare cases in which
                 an island has a negative rms value. Fixed a memory issue
                 in which memory was not released after using show_fit.

    2011/11/28 - Added option to have minpix_isl estimated automatically
                 as 1/3 of the beam area. This estimate should help 
                 exclude false islands that are much smaller than the
                 beam. This estimate is not let to fall below 6 pixels.
    
    2011/11/11 - Fixed bugs in source generation that would lead to
                 masking of all pixels for certain sources during
                 moment analysis. Adjusted calculation of jmax in
                 wavelet module to use island sizes (instead of image size)
                 if opts.atrous_orig_isl is True.

    2011/11/04 - Implemented new island fitting routine (enabled with the
                 peak_fit option) that can speed up fitting of large 
                 islands. Changed plotting of Gaussians in show_fit to 
                 use Ellipse artists to improve plotting speed.

    2011/11/03 - Altered reading of images to correctly handle 4D cubes.
                 Fixed bug in readimage that affected filenames.

    2011/10/26 - Extended psf_vary module to include fitting of stacked
                 PSFs with Gaussians, interpolation of the resulting
                 parameters across the image, and correction of the de-
                 convolved source sizes using the interpolated PSFs.
                 Changed plotting of Gaussians in show_fit() to use the
                 FWHM instead of sigma. Modified error calculation of M
                 sources to be more robust when sources are small. Fixed
                 spelling of "gaussian" in bbs_patches option list.

    2011/10/24 - Many small bug fixes to the psf_vary module. Fixed use of
                 input directory so that input files not in the current
                 directory are handled correctly.

    2011/10/14 - Added residual rms and mean values to sources and source
                 list catalogs. These values can be compared to background
                 rms and mean values as a quick check of fit quality.

    2011/10/13 - Modified deconvolution to allow 1-D Gaussians and sources. 
                 Added FREQ0, EQUINOX, INIMAGE keywords to output fits
                 catalogs. Fixed bug in source position angles. Adjusted
                 column names of output catalogs slightly to be more
                 descriptive.

    2011/10/12 - Added errors to source properties (using a Monte Carlo
                 method for M sources). Fixed bug in output column names.
    
    2011/10/11 - Tweaked autocomplete to support IPython shell commands
                 (e.g., "!more file.txt"). Fixed bug in gaul2srl that 
                 resulted in some very nearby Gaussians being placed into
                 different sources. Added group_tol option so that user
                 can adjust the tolerance of how Gaussians are grouped
                 into sources.
    
    2011/10/05 - Added output of source lists. Changed name of write_gaul 
                 method to write_catalog (more general).
                 
    2011/10/04 - Added option to force source grouping by island 
                 (group_by_isl). Added saving of parameters to a PyBDSM 
                 save file to Op_output.
    
    2011/09/21 - Fixed issue with shapelet centering failing: it now falls
                 back to simple moment when this happens. Fixed issue with
                 plotresults when shapelets are fit.
                 
    2011/09/14 - Placed output column names and units in TC properties of
                 Gaussians. This allows easy standardization of the column
                 names and units.
                 
    2011/09/13 - Fixes to trim_box and resetting of Image objects in 
                 interface.process(). Changed thr1 --> thr2 in fit_iter in
                 guasfit.py, as bright sources are often "overfit" when 
                 using thr1, leading to large negative residuals. 
                 Restricted fitting of Gaussians to wavelet images to be 
                 only in islands found in the original image if 
                 opts.atrous_orig_isl is True. 
    
    2011/09/08 - Version 1.0
    
    2011/09/08 - Versioning system changed to use _version.py.
    
    """
    pass
    