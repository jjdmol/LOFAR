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
  ,source_kind smallint not null default 0 -- 0-Point; 1-Gaussian;
  ,zone INT NOT NULL
  ,wm_ra double NOT NULL
  ,wm_decl double NOT NULL
  ,wm_ra_err double NOT NULL
  ,wm_decl_err double NOT NULL
  ,avg_wra double NOT NULL
  ,avg_wdecl double NOT NULL
  ,avg_weight_ra double NOT NULL
  ,avg_weight_decl double NOT NULL
  ,x double NOT NULL
  ,y double NOT NULL
  ,z double NOT NULL
  --,margin BOOLEAN NOT NULL DEFAULT 0
  ,beam_semimaj double NULL
  ,beam_semimin double NULL
  ,beam_pa double NULL
  ,stokes CHAR(1) NOT NULL DEFAULT 'I'
  ,avg_f_peak double NULL
  ,avg_f_peak_sq double NULL
  ,avg_weight_f_peak double NULL
  ,avg_weighted_f_peak double NULL
  ,avg_weighted_f_peak_sq double NULL
  )
;

