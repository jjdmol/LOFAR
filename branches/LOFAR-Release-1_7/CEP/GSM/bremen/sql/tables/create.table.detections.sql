/**
 * This is a temporary table, used to load
 * the detections from the sources extraction.
 */
CREATE TABLE detections
  (image_id VARCHAR(64) NOT NULL
  ,lra double NOT NULL
  ,ldecl double NOT NULL
  ,lra_err double NOT NULL
  ,ldecl_err double NOT NULL
  ,lf_peak double NULL
  ,lf_peak_err double NULL
  ,lf_int double NULL
  ,lf_int_err double NULL
  ,g_minor double null
  ,g_minor_err double null
  ,g_major double null
  ,g_major_err double null
  ,g_pa double null
  ,g_pa_err double null
  ,ldet_sigma double NOT NULL
  ,healpix_zone int not null
  )
;

