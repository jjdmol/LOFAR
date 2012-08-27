.. _new:

**********
What's New
**********

Version 1.3 (2012/07/03):
    
    * Fixed a bug in the calculation of positional errors for Gaussians.
    
    * Adjusted ``rms_box`` algorithm to check for negative rms values (due to interpolation with cubic spline). If negative values are found, either the box size is increased or the interpolation is done with ``order=1`` (bilinear) instead.

    * Output now includes the residual image produced using only wavelet Gaussians (if any) when ``atrous_do=True`` and ``output_all=True``. 
    
    * Improved organization of files when ``output_all=True``. 
    
    * Added logging of simple statistics (mean, std. dev, skew, and kurtosis) of the residual images.

    * Included image rotation (if any) in beam definition. Rotation angle can vary across the image (defined by image WCS).

    * Added Sagecal output format for Gaussian catalogs.

    * Added check for newer versions of the PyBDSM software ``tar.gz`` file available on ftp.strw.leidenuniv.nl.

    * Added total island flux (from sum of pixels) to ``gaul`` and ``srl`` catalogs.

Version 1.2 (2012/06/06):
        
    * Added option to output flux densities for every channel found by the spectral index module. 
    
    * Added option to spectral index module to allow use of flux densities that do not meet the desired SNR.

    * Implemented an adaptive scaling scheme for the ``rms_box`` parameter that shrinks the box size near bright sources and expands it far from them (enabled with the ``adaptive_rms_box`` option when ``rms_box`` is None). This scheme generally results in improved rms and mean maps when both strong artifacts and extended sources are present.

    * Improved speed of Gaussian fitting to wavelet images.

    * Added option to calculate fluxes within a specified aperture radius in pixels (set with the ``aperture`` option). Aperture fluxes, if measured, are output in the ``srl`` format catalogs.

Version 1.1 (2012/03/28):

    * Tweaked settings that affect fitting of Gaussians to improve fitting in general.
    
    * Modified calculation of the ``rms_box`` parameter (when the ``rms_box`` option is None) to work better with fields composed mainly of point sources when strong artifacts are present. 
    
    * Modified fitting of large islands to adopt an iterative fitting scheme that limits the number of Gaussians fit simultaneously per iteration to 10. This change speeds up fitting of large islands considerably. 
    
    * Added the option to use a "detection" image for island detection (the ``detection_image`` option); source properties are still measured from the main input image. This option is particularly useful with images made with LOFAR's AWImager, as the uncorrected, flat-noise image (the ``*.restored`` image) is better for source detection than the corrected image (the ``*.restored.corr`` image). 
            
    * Modified the polarization module so that sources that appear only in Stokes Q or U (and hence not in Stokes I) are now identified. This identification is done using the polarized intensity (PI) image.
    
    * Improved the plotting speed (by a factor of many) in ``show_fit`` when there are a large number of islands present.
    
    * Simplified the spectral index module to make it more user friendly and stable.
    
    * Altered reading of images to correctly handle 4D cubes.
    
    * Extended the ``psf_vary`` module to include fitting of stacked PSFs with Gaussians, interpolation of the resulting parameters across the image, and correction of the deconvolved source sizes using the interpolated PSFs.
    
    * Added residual rms and mean values to source catalogs. These values can be compared to background rms and mean values as a quick check of fit quality.
    
    * Added output of shapelet parameters as FITS tables.
    
    * Fixed many minor bugs.

See the changelog (accessible from the interactive shell using ``help changelog``) for details of all changes since the last version.