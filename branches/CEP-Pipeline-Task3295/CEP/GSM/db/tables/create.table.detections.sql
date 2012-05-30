/**
 * This is a temporary table, used to load
 * the detections from the sources extraction.
 */
CREATE TABLE detections 
  (lra DOUBLE NOT NULL 
  ,ldecl DOUBLE NOT NULL 
  ,lra_err DOUBLE NOT NULL 
  ,ldecl_err DOUBLE NOT NULL 
  ,lf_peak DOUBLE NULL 
  ,lf_peak_err DOUBLE NULL 
  ,lf_int DOUBLE NULL 
  ,lf_int_err DOUBLE NULL 
  ,ldet_sigma DOUBLE NOT NULL
  )
;

