=================================
Standard Imaging Pipeline recipes
=================================

On this page the three toplevel recipes of the LOFAR Automatic Imaging Pipeline
for MSSS type observations.
The Calibrator pipeline creates an instrument model based on a calibration
observation. 
The instrument model, the calibration solution, is applied to the actual measurements 
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
	
    missing recipe<imager_bbs>



Imager Pipeline
------------------------------------

.. autoclass:: msss_imager_pipeline.msss_imager_pipeline
    
**Recipes of the Imager Pipeline**

.. toctree::
    :maxdepth: 1

    1. imager_prepare <imager_prepare>
    2. imager_create_dbs <imager_create_dbs>
	3. imager_bbs <imager_bbs>
	4. imager_awimager <imager_awimager>
	5. imager_source_finding <imager_source_finding>
	6. imager_finalize <imager_finalize>
	7. get_metadata
	
	
	