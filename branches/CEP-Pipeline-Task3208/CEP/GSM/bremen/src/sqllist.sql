--#insert_extractedsources
insert into extractedsources (image_id, zone, ra, decl, ra_err, decl_err,
                              x, y, z, det_sigma,
                              f_peak, f_peak_err, f_int, f_int_err,
                              source_kind,
                              g_minor, g_minor_err, g_major, g_major_err,
                              g_pa, g_pa_err)
select %s, cast(floor(ldecl) as integer) as zone, lra, ldecl, lra_err, ldecl_err,
       cos(radians(ldecl))*cos(radians(lra)),
       cos(radians(ldecl))*sin(radians(lra)),
       sin(radians(ldecl)), 3.0, lf_peak, lf_peak_err, lf_int, lf_int_err,
       case when g_major is null or ldecl_err > g_major then 0
            else 1 end,
       g_minor, g_minor_err, g_major, g_major_err,
       g_pa, g_pa_err
from detections;

--#Associate extended
INSERT INTO temp_associations (xtrsrc_id, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id)
SELECT e.xtrsrcid, rc.runcatid,
$$get_distance('rc', 'e')$$ AS assoc_distance_arcsec, 2,
$$get_assoc_r('rc', 'e')$$  as assoc_r,
       rc.group_head_id
  FROM runningcatalog rc
      ,extractedsources e
      ,images im0
 WHERE e.image_id = {0}
   AND e.image_id = im0.imageid
   and rc.x between e.x - {1} and e.x + {1}
   and rc.y between e.y - {1} and e.y + {1}
   and rc.z between e.z - {1} and e.z + {1}
   and e.source_kind = 1
   and rc.source_kind = 1
   and rc.band = im0.band
   and rc.stokes = im0.stokes
 AND $$get_assoc_r('rc', 'e')$$ < {2};

--if no match was found for this band, then use cross-band source.
INSERT INTO temp_associations (xtrsrc_id, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id)
SELECT e.xtrsrcid, rc.runcatid,
$$get_distance('rc', 'e')$$ AS assoc_distance_arcsec, 3,
$$get_assoc_r('rc', 'e')$$  as assoc_r,
       rc.group_head_id
  FROM runningcatalog rc
      ,extractedsources e
      ,images im0
 WHERE e.image_id = {0}
   AND e.image_id = im0.imageid
   and rc.x between e.x - {1} and e.x + {1}
   and rc.y between e.y - {1} and e.y + {1}
   and rc.z between e.z - {1} and e.z + {1}
   and e.source_kind = 1
   and rc.source_kind = 1
   and rc.band is null
   and rc.stokes is null
   and not exists (select ta.runcat_id
                     from temp_associations ta
                    where ta.xtrsrc_id = e.xtrsrcid)
 AND $$get_assoc_r('rc', 'e')$$ < {2};


--#add 1 to 1
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select ta.xtrsrc_id, ta.runcat_id, ta.distance_arcsec, ta.lr_method, ta.r
  from temp_associations ta
 where kind = 1;


--#add 1 to N
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select ta.xtrsrc_id, ta.runcat_id, ta.distance_arcsec, ta.lr_method, ta.r
  from temp_associations ta
 where kind = 2
   and ta.distance_arcsec = (select min(tb.distance_arcsec)
                               from temp_associations tb
                              where tb.xtrsrc_id = ta.xtrsrc_id
                                and tb.kind = 2
                            );


--#add N to 1
--insert new sources
--point sources
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
$$get_column_insert_list(['ra', 'decl'])$$,
                           x, y, z, source_kind)
select e.xtrsrcid, 1, zone,
$$get_column_insert_values_list(['ra', 'decl'])$$,
       x, y, z, source_kind
  from extractedsources e,
       temp_associations ta
 where ta.xtrsrc_id = e.xtrsrcid
   and ta.kind = 3
   and e.source_kind = 0
   and ta.xtrsrc_id not in (select tx.min_id
                              from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                      from temp_associations tb
                                     where tb.kind = 3
                                  group by tb.runcat_id) tx);
--same for extended sources (update positions)
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
$$get_column_insert_list(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind)
select e.xtrsrcid, 1, zone,
$$get_column_insert_values_list(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       x, y, z, source_kind
  from extractedsources e,
       temp_associations ta
 where ta.xtrsrc_id = e.xtrsrcid
   and ta.kind = 3
   and e.source_kind = 1
   and ta.xtrsrc_id not in (select tx.min_id
                              from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                      from temp_associations tb
                                     where tb.kind = 3
                                  group by tb.runcat_id) tx);

--copy fluxes from old to new sources
insert into runningcatalog_fluxes(runcat_id, band, datapoints,
                               $$get_column_insert_list(['f_peak', 'f_int'])$$)
select r.runcatid, f.band, f.datapoints,
       $$get_column_insert_list(['f_peak', 'f_int'])$$
  from runningcatalog_fluxes f,
       temp_associations ta,
       runningcatalog r
 where ta.kind = 3
   and ta.runcat_id = f.runcat_id
   and r.first_xtrsrc_id = ta.xtrsrc_id
   and ta.xtrsrc_id not in (select tx.min_id
                              from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                      from temp_associations tb
                                     where tb.kind = 3
                                  group by tb.runcat_id) tx);

--copy associations from old to new source
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select a.xtrsrc_id, r.runcatid, $$get_distance('r', 'e')$$, 2, 0.0
  from assocxtrsources a,
       extractedsources e,
       temp_associations ta,
       runningcatalog r
 where a.runcat_id = ta.runcat_id
   and e.xtrsrcid = a.xtrsrc_id
   and r.first_xtrsrc_id = ta.xtrsrc_id
   and ta.kind = 3;

--insert new associations to new sources
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select ta.xtrsrc_id, r.runcatid, 0.0, 1, 0.0
  from temp_associations ta,
       runningcatalog r
 where r.first_xtrsrc_id = ta.xtrsrc_id
   and ta.kind = 3;


--insert new associations to old sources
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select e.xtrsrcid, ta.runcat_id, $$get_distance('r', 'e')$$,  2, 0.0
  from extractedsources e,
       temp_associations ta,
       runningcatalog r
 where ta.kind = 3
   and ta.xtrsrc_id = e.xtrsrcid
   and r.runcatid = ta.runcat_id
   and ta.xtrsrc_id in (select tx.min_id
                              from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                      from temp_associations tb
                                     where tb.kind = 3
                                  group by tb.runcat_id) tx);


--#Insert new sources
--point sources
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert_list(['ra', 'decl'])$$,
                           x, y, z, source_kind)
select e.xtrsrcid, 1, zone,
       $$get_column_insert_values_list(['ra', 'decl'])$$,
       x, y, z, 0
  from extractedsources e
 where image_id = {0}
   and source_kind = 0
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid)
order by e.xtrsrcid;


--insert new band for extended sources
insert into runningcatalog(band, stokes, first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert_list(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind, parent_source
                           )
select i.band, i.stokes, e.xtrsrcid, 1, zone,
       $$get_column_insert_values_list(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       x, y, z, 1, ta.runcat_id
  from extractedsources e,
       images i,
       temp_associations ta
 where e.image_id = {0}
   and i.imageid = e.image_id
   and e.source_kind = 1
   and e.xtrsrcid = ta.xtrsrc_id
   and ta.lr_method = 3
order by e.xtrsrcid;

--insert totally new extended sources
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert_list(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind, parent_source
                           )
select e.xtrsrcid, 1, zone,
       $$get_column_insert_values_list(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       x, y, z, 1, null
  from extractedsources e
 where image_id = {0}
   and source_kind = 1
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid)
order by e.xtrsrcid;

insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method,
                            lr, r)
select r.first_xtrsrc_id, r.runcatid, 0.0, 0, 0.0, 0.0
  from runningcatalog r,
       extractedsources e
 where e.image_id = {0}
   and e.xtrsrcid = r.first_xtrsrc_id
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid);

insert into runningcatalog_fluxes(runcat_id, band, datapoints,
                                  $$get_column_insert_list(['f_peak', 'f_int'])$$)
select r.runcatid, i.band, 1,
       $$get_column_insert_values_list(['f_peak', 'f_int'])$$
  from extractedsources e,
       images i,
       runningcatalog r
 where e.image_id = {0}
   and i.imageid = e.image_id
   and r.first_xtrsrc_id = e.xtrsrcid
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid);
