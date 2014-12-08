/**
 * This table contains all the extracted sources during an observation.
 * Maybe source is not the right description, because measurements
 * may be made that were erronous and do not represent a source.
 *
 * This table is empty BEFORE an observation.
 * DURING an observation new sources are inserted into this table
 * AFTER an observation this table is dumped and transported to the 
 * catalog database
 *
 * xtrsrcid         Every inserted source/measurement gets a unique id.
 * image_id         The reference id to the image from which this sources
 *                  was extracted.
 * zone             The zone number in which the source declination resides.
 *                  The width of the zones is determined by the "zoneheight" 
 *                  parameter defined in the zoneheight table.
 * ra               Right ascension of the measurement [in degrees]
 * decl             Declination of the measurement [in degrees]
 * ra_err           The 1sigma error of the ra measurement [in arcsec]
 * decl_err         The 1sigma error of the declination measurement [in arcsec]
 * x, y, z:         Cartesian coordinate representation of (ra,decl)
 * margin           Used for association procedures to take into 
 *                  account sources that lie close to ra=0 & ra=360 meridian.
 *                  True: source is close to ra=0 meridian
 *                  False: source is far away enough from the ra=0 meridian
 *                  TODO: This is not implemented yet.
 * det_sigma:       The sigma level of the detection,
 *                  20*(I_peak/det_sigma) gives the rms of the detection.
 * semimajor        Semi-major axis that was used for gauss fitting
 *                  [in arcsec]
 * semiminor        Semi-minor axis that was used for gauss fitting
 *                  [in arcsec]
 * pa               Position Angle that was used for gauss fitting 
 *                  [from north through local east, in degrees]
 *  f_peak            peak flux values (refer to images table for Stokes param)
 *  f_int           integrated flux values
 *  err             1sigma errors
 *                  Fluxes and flux errors are in Jy
 * 
 */

CREATE SEQUENCE "seq_extractedsources" AS INTEGER 
  START WITH 1
  INCREMENT BY 1
;

CREATE TABLE extractedsources 
  (xtrsrcid INT DEFAULT NEXT VALUE FOR "seq_extractedsources"
  ,image_id INT NOT NULL
  ,zone INT NOT NULL
  ,ra DOUBLE NOT NULL
  ,decl DOUBLE NOT NULL
  ,ra_err DOUBLE NOT NULL
  ,decl_err DOUBLE NOT NULL
  ,x DOUBLE NOT NULL
  ,y DOUBLE NOT NULL
  ,z DOUBLE NOT NULL
  ,margin BOOLEAN NOT NULL DEFAULT 0
  ,det_sigma DOUBLE NOT NULL
  ,g_major DOUBLE NULL
  ,g_minor DOUBLE NULL
  ,g_pa DOUBLE NULL
  ,f_peak DOUBLE NULL
  ,f_peak_err DOUBLE NULL
  ,f_int DOUBLE NULL
  ,f_int_err DOUBLE NULL
  ,PRIMARY KEY (xtrsrcid)
  ,FOREIGN KEY (image_id) REFERENCES images (imageid)
  )
;

