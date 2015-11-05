--DROP TABLE runningcatalog;
/* This table contains the unique sources that were detected
 * during an observation.
 * TODO: The resolution element (from images table) is not implemented yet
 * Extractedsources not in this table are appended when there is no positional match
 * or when a source was detected in a higher resolution image.
 *
 * We maintain weighted averages for the sources (see ch4, Bevington)
 * wm_ := weighted mean
 *
 * wm_ra := avg_wra/avg_weight_ra
 * wm_decl := avg_wdecl/avg_weight_decl
 * wm_ra_err := 1/(N * avg_weight_ra)
 * wm_decl_err := 1/(N * avg_weight_decl)
 * avg_wra := avg(ra/ra_err^2)
 * avg_wdecl := avg(decl/decl_err^2)
 * avg_weight_ra := avg(1/ra_err^2)
 * avg_weight_decl := avg(1/decl_err^2)
 */

CREATE SEQUENCE "seq_runningcatalog" AS INTEGER;

CREATE TABLE runningcatalog
  (runcatid INT DEFAULT NEXT VALUE FOR "seq_runningcatalog"
  ,first_xtrsrc_id int not null -- id of the first observation
  ,ds_id INT NULL
  ,band INT NULL        -- not null for group members ONLY
  ,stokes CHAR(1) NULL  -- not null for group members ONLY
  ,datapoints INT NOT NULL
  ,decl_zone INT NULL

  ,wm_ra double NOT NULL
  ,wm_ra_err double NOT NULL
  ,avg_wra double NULL
  ,avg_weight_ra double NULL

  ,wm_decl double NOT NULL
  ,wm_decl_err double NOT NULL
  ,avg_wdecl double NULL
  ,avg_weight_decl double NULL

  ,source_kind smallint not null default 0 -- 0-Point; 1-Gaussian, 2-Group head, 3-Dummy, 4-ToBe Updated;
  ,parent_runcat_id int null --reference to the non-banded extended source

  ,wm_g_minor double NULL
  ,wm_g_minor_err double NULL
  ,avg_wg_minor double NULL
  ,avg_weight_g_minor double NULL

  ,wm_g_major double NULL
  ,wm_g_major_err double NULL
  ,avg_wg_major double NULL
  ,avg_weight_g_major double NULL

  ,wm_g_pa double NULL
  ,wm_g_pa_err double NULL
  ,avg_wg_pa double NULL
  ,avg_weight_g_pa double NULL

  ,is_group BOOLEAN NOT NULL DEFAULT FALSE -- to be used for groups
  ,group_head_id INT NULL --reference to the group head
  ,deleted BOOLEAN NOT NULL DEFAULT FALSE -- deletion flag
  ,last_update_date timestamp not null default current_timestamp()

  ,x double NOT NULL
  ,y double NOT NULL
  ,z double NOT NULL
  ,last_spectra_update_date timestamp null --last update of spectral indices
  ,spectral_power integer null
  ,spectral_index_0 double  null
  ,spectral_index_1 double  null
  ,spectral_index_2 double  null
  ,spectral_index_3 double  null
  ,spectral_index_4 double  null

  --,beam_semimaj double NULL
  --,beam_semimin double NULL
  --,beam_pa double NULL
  --,stokes CHAR(1) NOT NULL DEFAULT 'I'
  )
;

