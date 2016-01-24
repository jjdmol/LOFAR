--#Insert new sources
--point sources
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl'])$$,
                           x, y, z, source_kind, healpix_zone)
select e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl'])$$,
       x, y, z, 0, healpix_zone
  from extractedsources e
 where image_id = [i]
   and source_kind = 0
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid
                      and x.image_id = [i])
   and e.xtrsrcid2 is null
order by e.xtrsrcid;


--insert new band for extended sources
insert into runningcatalog(band, stokes, first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind, parent_runcat_id, healpix_zone
                           )
select i.band, i.stokes, e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       x, y, z, 1, ta.runcat_id, healpix_zone
  from extractedsources e,
       images i,
       temp_associations ta
 where e.image_id = [i]
   and ta.image_id = [i]
   and i.imageid = e.image_id
   and e.source_kind = 1
   and e.xtrsrcid = ta.xtrsrc_id
   and ta.kind = 1 -- 1-to-1 or N-to-1 association
   and ta.lr_method = 3 --this indicates that there is a cross-band association
order by e.xtrsrcid;

--insert new band in the case of 1-to-N association, when the nearest source is the
--cross-band source.
insert into runningcatalog(band, stokes, first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind, parent_runcat_id, healpix_zone
                           )
select i.band, i.stokes, e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       x, y, z, 1, ta.runcat_id, healpix_zone
  from extractedsources e,
       images i,
       temp_associations ta
 where e.image_id = [i]
   and ta.image_id = [i]
   and i.imageid = e.image_id
   and e.source_kind = 1
   and e.xtrsrcid = ta.xtrsrc_id
   and ta.kind = 2 --there is only one association
   and ta.lr_method = 3 --this indicates that there is a cross-band association
   and ta.distance_arcsec = (select min(tb.distance_arcsec)
                               from temp_associations tb
                              where tb.xtrsrc_id = ta.xtrsrc_id
                                and tb.kind = 2
                                and tb.image_id = [i]
                            )
order by e.xtrsrcid;


--insert totally new extended sources
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind, parent_runcat_id, healpix_zone
                           )
select e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       x, y, z, 1, null, healpix_zone
  from extractedsources e
 where image_id = [i]
   and source_kind = 1
   and xtrsrcid2 is null
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid
                      and x.image_id = [i])
order by e.xtrsrcid;

insert into runningcatalog(band, stokes, first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind, parent_runcat_id, healpix_zone
                           )
select i.band, i.stokes, e.xtrsrcid, 1, zone,
       $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$, --can copy from runningcatalog
       e.x, e.y, e.z, 1, r.runcatid, e.healpix_zone
  from extractedsources e,
       images i,
       runningcatalog r
 where e.image_id = [i]
   and i.imageid = [i]
   and e.source_kind = 1
   and r.source_kind = 1
   and e.xtrsrcid = r.first_xtrsrc_id
   and e.xtrsrcid2 is null
   and not r.deleted
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid
                      and x.image_id = [i])
order by e.xtrsrcid;

--associate new extended sources and new point sources
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method,
                            lr, r)
select r.first_xtrsrc_id, r.runcatid, 0.0, 0, 0.0, 0.0
  from runningcatalog r,
       extractedsources e
 where e.image_id = [i]
   and e.xtrsrcid = r.first_xtrsrc_id
   and not exists (select x.xtrsrc_id
                     from temp_associations x
                    where x.xtrsrc_id = e.xtrsrcid
                      and x.image_id = [i])
union
select r.first_xtrsrc_id, r.runcatid, 0.0, 0, 0.0, 0.0
  from runningcatalog r,
       extractedsources e,
       temp_associations ta
 where e.image_id = [i]
   and ta.image_id = [i]
   and e.xtrsrcid = r.first_xtrsrc_id
   and ta.xtrsrc_id = e.xtrsrcid
   and not r.deleted
   and ta.runcat_id = r.parent_runcat_id --there is a link to the cross-band source
   and r.band is not null --this is not a cross-band source
;

--insert fluxes for new sources (if needed)
insert into runningcatalog_fluxes(runcat_id, band, datapoints,
                                  $$get_column_insert(['f_peak', 'f_int'])$$)
select r.runcatid, i.band, r.datapoints,
       $$get_column_insert_values(['f_peak', 'f_int'])$$
  from extractedsources e,
       images i,
       runningcatalog r
 where e.image_id = [i]
   and i.imageid = e.image_id
   and r.first_xtrsrc_id = e.xtrsrcid
   and not r.deleted
   and (r.band = i.band or r.source_kind = 0) --this is not a cross-band source
   and not exists (select f.band
                     from runningcatalog_fluxes f
                    where f.runcat_id = r.runcatid
                      and f.band = i.band
                      and f.stokes = i.stokes)
   ;
