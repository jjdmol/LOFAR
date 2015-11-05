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
  ,wm_ra DOUBLE NOT NULL
  ,wm_decl DOUBLE NOT NULL
  ,wm_ra_err DOUBLE NOT NULL
  ,wm_decl_err DOUBLE NOT NULL
  ,avg_wra DOUBLE NULL
  ,avg_wdecl DOUBLE NULL
  ,avg_weight_ra DOUBLE NULL
  ,avg_weight_decl DOUBLE NULL
  ,source_kind smallint not null default 0 -- 0-Point; 1-Gaussian, 2-Group head, 3-Dummy;
  ,is_group BOOLEAN NOT NULL DEFAULT FALSE -- to be used for groups
  ,group_head_id INT NULL --reference to the group head
  ,deleted BOOLEAN NOT NULL DEFAULT FALSE -- deletion flag
  ,x DOUBLE NOT NULL
  ,y DOUBLE NOT NULL
  ,z DOUBLE NOT NULL
  --,beam_semimaj DOUBLE NULL
  --,beam_semimin DOUBLE NULL
  --,beam_pa DOUBLE NULL
  --,stokes CHAR(1) NOT NULL DEFAULT 'I'
  )
;

