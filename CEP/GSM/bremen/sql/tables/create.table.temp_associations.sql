--drop table temp_associations;
create table temp_associations(
   xtrsrc_id int not null
  ,xtrsrc_id2 int null --reference to the original source (for near-360-degrees copies), otherwise same as xtrsrc_id
  ,runcat_id int not null
  ,distance_arcsec double null
  ,lr_method int null default 1 -- 1: point-point, 2: extended-extended, 3: extended-extended (cross-band)
  ,r double null
  ,lr double null
  ,xtr_count int null
  ,run_count int null
  ,kind int null -- 1: 1-1, 2: 1-n; 3: n-1; 4: n-n; 5: merge extended
  ,group_head_id int null
  ,flux_fraction double null

  ,PRIMARY KEY (xtrsrc_id, runcat_id)

);

