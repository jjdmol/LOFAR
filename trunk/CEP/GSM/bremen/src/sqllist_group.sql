--#GroupCycle
select distinct group_head_id
  from temp_associations
 where kind = 4
   and image_id = [i];

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

--#GroupUpdate runcat
update runningcatalog
   set group_head_id = {0},
       last_update_date = current_timestamp
 where exists (select 1 from temp_associations ta
                where ta.runcat_id = runningcatalog.runcatid
                  and kind = 4
                  and group_head_id = {0});

--#GroupFill
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select xtrsrc_id, runcat_id, distance_arcsec, 5, r
  from temp_associations
 where kind = 4
    and image_id = [i]
;

--#Solution insert
INSERT INTO temp_associations (xtrsrc_id, xtrsrc_id2, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id, image_id)
SELECT e.xtrsrcid, coalesce(e.xtrsrcid2, e.xtrsrcid), rc.runcatid,
$$get_distance('rc', 'e')$$ AS assoc_distance_arcsec, 99,
$$get_assoc_r('rc', 'e')$$ as assoc_r,
       null, [i]
  FROM runningcatalog rc
      ,extractedsources e
      ,images i
 where image_id = [i]
   and e.xtrsrcid = {0}
   and rc.runcatid = {1};
