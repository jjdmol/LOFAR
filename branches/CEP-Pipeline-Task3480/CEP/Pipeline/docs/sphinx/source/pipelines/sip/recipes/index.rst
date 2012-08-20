=================================
Standard Imaging Pipeline recipes
=================================

On this page the three toplevel recipes of the LOFAR Automatic Imaging Pipeline
for MSSS type observations.
The Calibrator pipeline creates an instrument model based on a calibration
observation. 
The instrument model, the calibration solution, is applied to the actual measurments 
in the target pipeline. These Measurement sets are then used by the imaging pipeline
to produce, sky images and a list of sources found in this image.
Each of these steps will get more details in each of the chapters

Calibrator Pipeline
------------------------------------	
.. toctree::
    :maxdepth: 1
	
    sip
    datamapper
    storagemapper
    dppp
    rficonsole
    bbs
    sourcedb
    parmdb
    cimager
    vdsmaker
    vdsreader

Target Pipeline
------------------------------------
.. toctree::
    :maxdepth: 1
	
Image Pipeline
------------------------------------
.. toctree::
    :maxdepth: 1
	
    imager_prepare
	imager_create_dbs
	imager_bbs
	imager_awimager
	imager_sourcefinding
	imager_finalize

