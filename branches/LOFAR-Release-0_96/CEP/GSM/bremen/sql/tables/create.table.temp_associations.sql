drop table temp_associations;
create table temp_associations(
   xtrsrc_id int not null
  ,runcat_id int not null
  ,weight double null
  ,distance_arcsec double null
  ,lr_method int null default 0
  ,r double null
  ,lr double null
  ,xtr_count int null
  ,run_count int null
  ,kind int null -- 1: 1-1, 2: 1-n; 3: n-1; 4: n-n
  ,group_head_id int null
);

