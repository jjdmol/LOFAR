--DROP TABLE temprunningcatalog;
/* This table contains the unique sources that were detected
 * during an observation.
 * TODO: The resolution element (from images table) is not implemented yet
 * Extractedsources not in this table are appended when there is no positional match
 * or when a source was detected in a higher resolution image.
 */
--DROP TABLE tempbasesources;
CREATE TABLE temprunningcatalog 
  (xtrsrc_id INT NOT NULL
  ,assoc_xtrsrc_id INT NOT NULL
  ,ds_id INT NOT NULL
  ,band INT NOT NULL
  ,datapoints INT NOT NULL
  ,zone INT NOT NULL
  ,wm_ra DOUBLE NOT NULL
  ,wm_decl DOUBLE NOT NULL
  ,wm_ra_err DOUBLE NOT NULL
  ,wm_decl_err DOUBLE NOT NULL
  ,avg_wra DOUBLE NOT NULL
  ,avg_wdecl DOUBLE NOT NULL
  ,avg_weight_ra DOUBLE NOT NULL
  ,avg_weight_decl DOUBLE NOT NULL
  ,x DOUBLE NOT NULL
  ,y DOUBLE NOT NULL
  ,z DOUBLE NOT NULL
  ,margin BOOLEAN NOT NULL DEFAULT 0
  ,beam_semimaj DOUBLE NULL
  ,beam_semimin DOUBLE NULL
  ,beam_pa DOUBLE NULL
  ,stokes CHAR(1) NOT NULL DEFAULT 'I'
  ,avg_f_peak DOUBLE NULL
  ,avg_f_peak_sq DOUBLE NULL
  ,avg_weight_f_peak DOUBLE NULL
  ,avg_weighted_f_peak DOUBLE NULL
  ,avg_weighted_f_peak_sq DOUBLE NULL
  )
;

