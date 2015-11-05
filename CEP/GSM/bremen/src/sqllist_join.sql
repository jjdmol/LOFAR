--#Join extended
--Kk1,l2 + Lm3,n4 + X5 = (KLX)k1,l2,m3,n4,o5
--switch all bound pieces to 1
update runningcatalog
   set parent_runcat_id = (select min(tt.runcat_id)
                          from temp_associations ta,
                               temp_associations tt
                         where ta.runcat_id = runningcatalog.parent_runcat_id
                           and ta.xtrsrc_id = tt.xtrsrc_id
                           and ta.kind = 5),
       last_update_date = current_timestamp
 where exists (select ta.kind
                 from temp_associations ta
                where ta.runcat_id = runningcatalog.parent_runcat_id
                  and ta.kind = 5)
   and parent_runcat_id is not null;

--reassign all sources to the merged ones
update assocxtrsources
   set runcat_id = (select min(tt.runcat_id)
                      from temp_associations ta,
                           temp_associations tt
                     where ta.runcat_id = assocxtrsources.runcat_id
                       and ta.xtrsrc_id = tt.xtrsrc_id
                       and ta.kind = 5)
 where exists (select ta.kind
                 from temp_associations ta
                where ta.runcat_id = assocxtrsources.runcat_id
                  and ta.kind = 5);

--mark obsolete sources as deleted
update runningcatalog
   set deleted = true,
       last_update_date = current_timestamp
 where exists (select ta.kind
                 from temp_associations ta
                where ta.runcat_id = runningcatalog.runcatid
                  and ta.kind = 5)
   and not exists (select a.lr_method
                     from assocxtrsources a
                    where runningcatalog.runcatid = a.runcat_id);

update runningcatalog
   set source_kind = 4,
       last_update_date = current_timestamp
 where exists (select ta.kind
                 from temp_associations ta
                where ta.runcat_id = runningcatalog.runcatid
                  and ta.kind = 5)
   and exists (select a.lr_method
                 from assocxtrsources a
                where runningcatalog.runcatid = a.runcat_id);

insert into runningcatalog(band, stokes, first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind, parent_runcat_id, healpix_zone
                           )
select i.band, i.stokes, e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       e.x, e.y, e.z, 1, ta.runcat_id, e.healpix_zone
  from extractedsources e,
       images i,
       temp_associations ta,
       runningcatalog r
 where e.image_id = {0}
   and i.imageid = e.image_id
   and e.source_kind = 1
   and e.xtrsrcid = ta.xtrsrc_id
   and ta.kind = 5 --there is only one association
   and ta.lr_method = 3 --this indicates that there is a cross-band association
   and r.runcatid = ta.runcat_id
   and not r.deleted
order by e.xtrsrcid;

--add associations to new sources
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method,
                            lr, r)
select xtrsrc_id, r.runcatid, 0, 4, 0, 0
  from temp_associations ta,
       runningcatalog r
 where ta.kind = 5
   and r.parent_runcat_id = ta.runcat_id
   and r.first_xtrsrc_id = ta.xtrsrc_id
union
select xtrsrc_id, r.parent_runcat_id, 0, 4, 0, 0
  from temp_associations ta,
       runningcatalog r
 where ta.kind = 5
   and r.parent_runcat_id = ta.runcat_id
   and r.first_xtrsrc_id = ta.xtrsrc_id;

--insert fluxes for new sources
insert into runningcatalog_fluxes(runcat_id, band, datapoints,
                                  $$get_column_insert(['f_peak', 'f_int'])$$)
select r.runcatid, i.band, 1,
       $$get_column_insert_values(['f_peak', 'f_int'])$$
  from extractedsources e,
       images i,
       runningcatalog r,
       temp_associations ta
 where e.image_id = {0}
   and i.imageid = e.image_id
   and r.first_xtrsrc_id = e.xtrsrcid
   and r.band is not null --this is not a cross-band source
   and ta.runcat_id = r.parent_runcat_id
   and ta.xtrsrc_id = e.xtrsrcid
   and ta.kind = 5;

update runningcatalog
   set datapoints = (select count(*)
                       from assocxtrsources a
                      where a.runcat_id = runningcatalog.runcatid),
       $$get_column_update_total(['ra', 'decl', 'g_minor', 'g_major', 'g_pa'])$$
 where not deleted
   and source_kind = 4;

update runningcatalog
   set $$get_column_update_second(['ra', 'decl', 'g_minor', 'g_major', 'g_pa'])$$,
       last_update_date = current_timestamp,
       source_kind = 1
 where not deleted
   and source_kind = 4;
