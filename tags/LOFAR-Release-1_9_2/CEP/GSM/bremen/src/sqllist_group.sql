--#GroupFinder
select xtrsrc_id, runcat_id, group_head_id
  from temp_associations
 where kind = 4
   and image_id = [i];

--#GroupUpdate
update temp_associations
   set group_head_id = {0}
 where kind = 4
   and image_id = [i]
   and runcat_id in ({1});

update runningcatalog
   set group_head_id = {0},
       last_update_date = current_timestamp
 where runcatid in ({1});

--#GroupFill
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select xtrsrc_id, runcat_id, distance_arcsec, 5, r
  from temp_associations
 where kind = 4
    and image_id = [i]
;
