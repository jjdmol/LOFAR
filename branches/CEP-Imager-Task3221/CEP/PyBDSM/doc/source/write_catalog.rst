.. _write_catalog:

***************************************************
``write_catalog``: writing source catalogs
***************************************************

The properties of the fitted Gaussians, sources, and shapelets may be written in a number of formats to a file using the ``write_catalog`` task.  See below (:ref:`output_cols`) for a detailed description of the output columns.

.. note::

    The output catalogs always use the J2000 equinox. If the input image does not have an equinox of J2000, the coordinates of sources will be precessed to J2000.

The task parameters are as follows:

.. parsed-literal::

    WRITE_CATALOG: Write the Gaussian, source, or shapelet list to a file.
    ================================================================================
    :term:`outfile` ............... None : Output file name. None => file is named     
                                   automatically                               
    :term:`bbs_patches` ........... None : For BBS format, type of patch to use: None => no
                                   patches. 'single' => all Gaussians in one patch.
                                   'gaussian' => each Gaussian gets its own patch.
                                   'source' => all Gaussians belonging to a single
                                   source are grouped into one patch           
    :term:`catalog_type` ......... 'gaul': Type of catalog to write:  'gaul' - Gaussian 
                                   list, 'srl' - source list (formed by grouping
                                   Gaussians), 'shap' - shapelet list (not yet
                                   supported)
    :term:`clobber` .............. False : Overwrite existing file?                    
    :term:`format` ................ 'bbs': Format of output Gaussian list: 'bbs', 'ds9',
                                   'fits', 'star', 'kvis', or 'ascii'          
    :term:`incl_wavelet` .......... True : Include Gaussians from wavelet decomposition (if
                                   any)?                                       
    :term:`srcroot` ............... None : Root name for entries in the output catalog. None
                                   => use image file name
                                   
Each of the parameters is described in detail below.

.. glossary::

    outfile
        This parameter is a string (default is ``None``) that sets the name of the output file. If ``None``, the file is named automatically.
    
    bbs_patches
        This parameter is a string (default is ``None``) that sets the type of patch to use in BBS-formatted catalogs. When the Gaussian catalogue is written as a BBS-readable sky file, this
        determines whether all Gaussians are in a single patch (``'single'``), there are no
        patches (``None``), all Gaussians for a given source are in a separate patch (``'source'``), or
        each Gaussian gets its own patch (``'gaussian'``).
        
        If you wish to have patches defined by island, then set
        ``group_by_isl = True`` before fitting to force all
        Gaussians in an island to be in a single source. Then set
        ``bbs_patches = 'source'`` when writing the catalog.

    catalog_type
        This parameter is a string (default is ``'gaul'``) that sets the type of catalog to write:  ``'gaul'`` - Gaussian list, ``'srl'`` - source list
        (formed by grouping Gaussians), ``'shap'`` - shapelet list (``'fits'`` format only)
        
        .. note::
        
            The choice of ``'srl'`` or ``'gaul'`` depends on whether you want all the source structure in your catalog or not. For example, if you are making a sky model for use as a model in calibration, you want to include all the source structure in your model, so you would use a Gaussian list (``'gaul'``), which writes each Gaussian. On the other hand, if you want to compare to other source catalogs, you want instead the total source fluxes, so use source lists (``'srl'``). For example, say you have a source that is unresolved in WENSS, but is resolved in your image into two nearby Gaussians that are grouped into a single source. In this case, you want to compare the sum of the Gaussians to the WENSS flux, and hence should use a source list.
        
    clobber
        This parameter is a Boolean (default is ``False``) that determines whether existing files are overwritten or not.
        
    format
        This parameter is a string (default is ``'bbs'``) that sets the format of the output catalog. The following formats are supported:

        * ``'bbs'`` - BlackBoard Selfcal sky model format (Gaussian list only)
        
        * ``'ds9'`` - ds9 region format
        
        * ``'fits'`` - FITS catalog format, readable by many software packages, including IDL, TOPCAT, Python, fv, Aladin, etc.
        
        * ``'star'`` - AIPS STAR format (Gaussian list only)
        
        * ``'kvis'`` - kvis format (Gaussian list only)
        
        * ``'ascii'`` - simple text file
        
        Catalogues with the ``'fits'`` and ``'ascii'`` formats include all available
        information (see :ref:`output_cols` for column definitions). The
        other formats include only a subset of the full information.
        
    incl_wavelet
        This parameter is a Boolean (default is ``True``) that determines whether Gaussians fit to wavelet images are included in the output.
        
    srcroot
        This parameter is a string (default is ``None``) that sets the root for source names in the output catalog.
        

.. _output_cols:

Definition of output columns
----------------------------
The information included in the Gaussian and source catalogs varies by format and can include the following quantities.

.. note::
    For ACSII and FITS formats, the reference frequency (in Hz) and equinox are stored in the header of the catalog. The header in ASCII catalogs is the first few lines of the catalog. For FITS catalogs, this information is stored in the comments as well as in the FREQ0 and EQUINOX keywords in the primary header.

* **Gaus_id:** a unique number that identifies the Gaussian, starting from zero

* **Source_id:** a unique number that identifies the Source, starting from zero

* **Isl_id:** a unique number that identifies the Island, starting from zero

* **Wave_id:** the wavelet scale from which the source was extracted, starting from zero (for the ch0 image)

* **RA:** the J2000 right ascension of the source, in degrees

* **E_RA:** the error on the right ascension of the source, in degrees

* **DEC:** the J2000 declination of the source, in degrees

* **E_DEC:** the 1-:math:`\sigma` error on the declination of the source, in degrees

* **Total_flux:** the total, integrated Stokes I flux of the source at the reference frequency, in Jy

* **E_Total_flux:** the 1-:math:`\sigma` error on the total flux of the source, in Jy

* **Peak_flux:** the peak Stokes I flux of the source, in Jy/beam

* **E_Peak_flux:** the 1-:math:`\sigma` error on the peak flux of the source, in Jy/beam

* **RA_max:** the J2000 right ascension of the maximum of the source, in degrees (``'srl'`` catalogs only)

* **E_RA_max:** the 1-:math:`\sigma` error on the right ascension of the maximum of the source, in degrees (``'srl'`` catalogs only)

* **DEC_max:** the J2000 declination of the maximum of the source, in degrees (``'srl'`` catalogs only)

* **E_DEC_max:** the 1-:math:`\sigma` error on the declination of the maximum of the source, in degrees (``'srl'`` catalogs only)

* **Xposn:** the x image coordinate of the source, in pixels

* **E_Xposn:** the 1-:math:`\sigma` error on the x image coordinate of the source, in pixels

* **Yposn:** the y image coordinate of the source, in pixels

* **E_Yposn:** the 1-:math:`\sigma` error on the y image coordinate of the source, in pixels

* **Xposn_max:** the x image coordinate of the maximum of the source, in pixels (``'srl'`` catalogs only)

* **E_Xposn_max:** the 1-:math:`\sigma` error on the x image coordinate of the maximum of the source, in pixels (``'srl'`` catalogs only)

* **Yposn_max:** the y image coordinate of the maximum of the source, in pixels (``'srl'`` catalogs only)

* **E_Yposn_max:** the 1-:math:`\sigma` error on the y image coordinate of the maximum of the source, in pixels (``'srl'`` catalogs only)

* **Maj:** the FWHM of the major axis of the source, in degrees

* **E_Maj:** the 1-:math:`\sigma` error on the FWHM of the major axis of the source, in degrees

* **Min:** the FWHM of the minor axis of the source, in degrees

* **E_Min:** the 1-:math:`\sigma` error on the FWHM of the minor axis of the source, in degrees

* **PA:** the position angle of the major axis of the source measured east of north, in degrees

* **E_PA:** the 1-:math:`\sigma` error on the position angle of the major axis of the source, in degrees

* **DC_Maj:** the FWHM of the deconvolved major axis of the source, in degrees

* **E_DC_Maj:** the 1-:math:`\sigma` error on the FWHM of the deconvolved major axis of the source, in degrees

* **DC_Min:** the FWHM of the deconvolved minor axis of the source, in degrees

* **E_DC_Min:** the 1-:math:`\sigma` error on the FWHM of the deconvolved minor axis of the source, in degrees

* **DC_PA:** the position angle of the deconvolved major axis of the source measured east of north, in degrees

* **E_DC_PA:** the 1-:math:`\sigma` error on the position angle of the deconvolved major axis of the source, in degrees

* **Isl_rms:** the average background rms value of the island, in Jy/beam

* **Isl_mean:** the averge background mean value of the island, in Jy/beam

* **Resid_Isl_rms:** the average residual background rms value of the island, in Jy/beam

* **Resid_Isl_mean:** the averge residual background mean value of the island, in Jy/beam

* **S_Code:** a code that defines the source structure. 
    * 'S' = a single-Gaussian source that is the only source in the island
    * 'C' = a single-Gaussian source in an island with other sources
    * 'M' = a multi-Gaussian source 

* **Spec_Indx:** the spectral index of the source

* **E_Spec_Indx:** the 1-:math:`\sigma` error on the spectral index of the source

* **Total_Q:** the total, integrated Stokes Q flux of the source at the reference frequency, in Jy

* **E_Total_Q:** the 1-:math:`\sigma` error on the total Stokes Q flux of the source at the reference frequency, in Jy

* **Total_U:** the total, integrated Stokes U flux of the source at the reference frequency, in Jy

* **E_Total_U:** the 1-:math:`\sigma` error on the total Stokes U flux of the source at the reference frequency, in Jy

* **Total_V:** the total, integrated Stokes V flux of the source at the reference frequency, in Jy

* **E_Total_V:** the 1-:math:`\sigma` error on the total Stokes V flux of the source at the reference frequency, in Jy

* **Linear_Pol_frac:** the linear polarization fraction of the source

* **Elow_Linear_Pol_frac:** the 1-:math:`\sigma` error on the linear polarization fraction of the source

* **Ehigh_Linear_Pol_frac:** the 1-:math:`\sigma` error on the linear polarization fraction of the source

* **Circ_Pol_Frac:** the circular polarization fraction of the source

* **Elow_Circ_Pol_Frac:** the 1-:math:`\sigma` error on the circular polarization fraction of the source

* **Ehigh_Circ_Pol_Frac:** the 1-:math:`\sigma` error on the circular polarization fraction of the source

* **Total_Pol_Frac:** the total polarization fraction of the source

* **Elow_Total_Pol_Frac:** the 1-:math:`\sigma` error on the total polarization fraction of the source

* **Ehigh_Total_Pol_Frac:** the 1-:math:`\sigma` error on the total polarization fraction of the source

* **Linear_Pol_Ang:** the linear polarization angle, measured east of north, in degrees

* **E_Linear_Pol_Ang:** the 1-:math:`\sigma` error on the linear polarization angle, in degrees


The shapelet catalog contains the following additional columns:

* **shapelet_basis:** the basis coordinate system: 'c' for cartesian, 's' for spherical

* **shapelet_beta:** the :math:`\beta` parameter of the shapelet decomposition

* **shapelet_nmax:** the maximum order of the shapelet

* **shapelet_cf:** a (flattened) array of the shapelet coefficients
