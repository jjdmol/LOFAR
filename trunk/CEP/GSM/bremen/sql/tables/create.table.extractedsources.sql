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

CREATE SEQUENCE "seq_extractedsources" AS INTEGER;

CREATE TABLE extractedsources
  (xtrsrcid INT DEFAULT NEXT VALUE FOR "seq_extractedsources"
  ,xtrsrcid2 int null --reference to the original source for a copied sources.
  ,image_id INT NOT NULL
  ,zone INT NOT NULL
  ,healpix_zone int not null
  ,ra double NOT NULL
  ,decl double NOT NULL
  ,ra_err double NOT NULL
  ,decl_err double NOT NULL
  ,x double NOT NULL
  ,y double NOT NULL
  ,z double NOT NULL
  ,det_sigma double NOT NULL
  ,source_kind smallint not null default 0 -- 0-Point; 1-Gaussian;
  ,g_major double NULL
  ,g_major_err double NULL
  ,g_minor double NULL
  ,g_minor_err double NULL
  ,g_pa double NULL
  ,g_pa_err double NULL
  ,f_peak double NULL
  ,f_peak_err double NULL
  ,f_int double NULL
  ,f_int_err double NULL
  ,PRIMARY KEY (xtrsrcid)
  --,FOREIGN KEY (image_id) REFERENCES images (imageid)
  )
;

