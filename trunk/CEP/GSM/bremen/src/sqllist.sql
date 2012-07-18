--#get frequency
select freqbandid
  from frequencybands
 where freq_low < {0} and freq_high > {0};

--#get last image_id
select max(imageid) from images;

--#insert image
insert into images (ds_id, tau, band, imagename, status,
                    centr_ra, centr_decl, svn_version)
select 0, 1, {1}, '{0}' as imagename, 0,
       0.0, 0.0, {2}


--#insert_extractedsources
insert into extractedsources (image_id, zone, ra, decl, ra_err, decl_err,
                              x, y, z, det_sigma,
                              f_peak, f_peak_err, f_int, f_int_err,
                              source_kind,
                              g_minor, g_minor_err, g_major, g_major_err,
                              g_pa, g_pa_err)
select {0}, cast(floor(ldecl) as integer) as zone, lra, ldecl, lra_err, ldecl_err,
       cos(radians(ldecl))*cos(radians(lra)),
       cos(radians(ldecl))*sin(radians(lra)),
       sin(radians(ldecl)), 3.0, lf_peak, lf_peak_err, lf_int, lf_int_err,
       case when g_major is null or ldecl_err > g_major or g_pa_err = 0.0 or g_major_err = 0.0 or g_minor_err = 0.0 then 0
            else 1 end,
       g_minor, g_minor_err, g_major, g_major_err,
       g_pa, g_pa_err
from detections
where lf_int_err > 0
  and lf_int > 0
  and lf_peak_err > 0;


--#insert dummysources
insert into extractedsources (image_id, zone, ra, decl, ra_err, decl_err,
                              x, y, z, det_sigma,
                              f_peak, f_peak_err, f_int, f_int_err,
                              source_kind,
                              g_minor, g_minor_err, g_major, g_major_err,
                              g_pa, g_pa_err, xtrsrcid2)
select image_id, zone, ra - 360.0, decl, ra_err, decl_err,
       x, y, z, det_sigma,
       f_peak, f_peak_err, f_int, f_int_err,
       source_kind,
       g_minor, g_minor_err, g_major, g_major_err,
       g_pa, g_pa_err, xtrsrcid
  from extractedsources
 where image_id = {0}
   and ra > 360 - 1/cos(radians(decl))
   and ra > 180
union
select image_id, zone, ra + 360.0, decl, ra_err, decl_err,
       x, y, z, det_sigma,
       f_peak, f_peak_err, f_int, f_int_err,
       source_kind,
       g_minor, g_minor_err, g_major, g_major_err,
       g_pa, g_pa_err, xtrsrcid
  from extractedsources
 where image_id = {0}
   and ra < 1/cos(radians(decl))
   and ra < 180;

--#update flux_fraction
--for point sources only
update temp_associations
   set flux_fraction = (select sum(e.f_int)
                          from extractedsources e,
                               temp_associations ta
                         where e.xtrsrcid = ta.xtrsrc_id
                           and ta.runcat_id = temp_associations.runcat_id)
 where kind = 3
   and lr_method < 3;

update temp_associations
   set flux_fraction = (select e.f_int/ta.flux_fraction
                          from extractedsources e,
                               temp_associations ta
                         where e.xtrsrcid = ta.xtrsrc_id
                           and ta.xtrsrc_id = temp_associations.xtrsrc_id
                           and ta.runcat_id = temp_associations.runcat_id)
 where kind = 3
   and lr_method < 3;

--#add 1 to 1
--exclude cross-band associations here. They will need a new record in runningcatalog.
--this new record will be added in the end, together with adding completely new objects.
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select ta.xtrsrc_id, ta.runcat_id, ta.distance_arcsec, ta.lr_method, ta.r
  from temp_associations ta
 where kind = 1
union
select ta.xtrsrc_id, r.parent_runcat_id, ta.distance_arcsec, 3, ta.r
  from temp_associations ta,
       runningcatalog r
 where ta.kind = 1
   and ta.lr_method = 2
   and r.runcatid = ta.runcat_id;


--#add 1 to N
--Warning! There can be associations with cross-band sources.
--here we add point-sources associations and
--associations with existing band record for extended sources
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select ta.xtrsrc_id, ta.runcat_id, ta.distance_arcsec, ta.lr_method, ta.r
  from temp_associations ta
 where ta.kind = 2
   and ta.distance_arcsec = (select min(tb.distance_arcsec)
                               from temp_associations tb
                              where tb.xtrsrc_id = ta.xtrsrc_id
                                and tb.kind = 2
                            )
union
select ta.xtrsrc_id, r.parent_runcat_id, ta.distance_arcsec, ta.lr_method, ta.r
  from temp_associations ta,
       runningcatalog r
 where ta.kind = 2
   and r.runcatid = ta.runcat_id
   and r.band is not null
   and ta.distance_arcsec = (select min(tb.distance_arcsec)
                               from temp_associations tb
                              where tb.xtrsrc_id = ta.xtrsrc_id
                                and tb.kind = 2
                            );

--#add N to 1
--insert new sources
--point sources
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl'])$$,
                           x, y, z, source_kind)
select e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl'])$$,
       x, y, z, source_kind
  from extractedsources e,
       temp_associations ta
 where ta.xtrsrc_id = e.xtrsrcid
   and ta.kind = 3
   and ta.lr_method = 1
   and ta.xtrsrc_id not in (select tx.min_id
                              from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                      from temp_associations tb
                                     where tb.kind = 3
                                  group by tb.runcat_id) tx);

--extended sources - per-band match
insert into runningcatalog(band, stokes, parent_runcat_id,
                           first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind)
select r.band, r.stokes, r.parent_runcat_id,
       e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       e.x, e.y, e.z, e.source_kind
  from extractedsources e,
       temp_associations ta,
       runningcatalog r
 where ta.xtrsrc_id = e.xtrsrcid
   and ta.kind = 3
   and ta.lr_method = 2
   and r.runcatid = ta.runcat_id
   and ta.xtrsrc_id not in (select tx.min_id
                              from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                      from temp_associations tb
                                     where tb.kind = 3
                                  group by tb.runcat_id) tx);

--extended sources - cross-band match
insert into runningcatalog(band, stokes, parent_runcat_id,
                           first_xtrsrc_id, datapoints, decl_zone,
                           $$get_column_insert(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
                           x, y, z, source_kind)
select i.band, i.stokes, ta.runcat_id,
       e.xtrsrcid, 1, zone,
       $$get_column_insert_values(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$,
       x, y, z, source_kind
  from extractedsources e,
       temp_associations ta,
       images i
 where ta.xtrsrc_id = e.xtrsrcid
   and ta.kind = 3
   and ta.lr_method = 3
   and i.imageid = e.image_id;

insert into runningcatalog_fluxes(runcat_id, band, stokes, datapoints,
                                  $$get_column_insert(['f_peak', 'f_int'])$$)
select r.runcatid, r.band, r.stokes, 1,
       $$get_column_insert_values(['f_peak', 'f_int'])$$
  from extractedsources e,
       temp_associations ta,
       runningcatalog r
 where ta.xtrsrc_id = e.xtrsrcid
   and r.first_xtrsrc_id = e.xtrsrcid
   and r.parent_runcat_id is not null
   and ta.kind = 3
   and ta.lr_method = 3;


--copy associations from old to new source
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r, weight)
--point sources
select a.xtrsrc_id, r.runcatid, $$get_distance('r', 'e')$$, 4, 0.0, a.weight*ta.flux_fraction
  from assocxtrsources a,
       extractedsources e,
       temp_associations ta,
       runningcatalog r
 where a.runcat_id = ta.runcat_id
   and e.xtrsrcid = a.xtrsrc_id
   and r.first_xtrsrc_id = ta.xtrsrc_id
   and not r.deleted
   and ta.kind = 3;

--insert new associations to new sources
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select ta.xtrsrc_id, r.runcatid, 0.0, ta.lr_method, 0.0
  from temp_associations ta,
       runningcatalog r
 where r.first_xtrsrc_id = ta.xtrsrc_id
   and (r.band is null or r.band = {0})
   and ta.runcat_id <> r.runcatid --not to the old sources(!!!)
   and ta.kind = 3;

--insert new associations to old sources
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select e.xtrsrcid, ta.runcat_id,
       $$get_distance('r', 'e')$$ as distance_arcsec,
       ta.lr_method, 0.0 as distance
  from extractedsources e,
       temp_associations ta,
       runningcatalog r
 where ta.kind = 3
   and ta.xtrsrc_id = e.xtrsrcid
   and not r.deleted
   and r.runcatid = ta.runcat_id
   and ta.xtrsrc_id in (select tx.min_id
                          from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                  from temp_associations tb
                                 where tb.kind = 3
                              group by tb.runcat_id) tx)
union
select e.xtrsrcid, r.parent_runcat_id,
       $$get_distance('r', 'e')$$ as distance_arcsec,
       ta.lr_method, 0.0 as distance
  from extractedsources e,
       temp_associations ta,
       runningcatalog r
 where ta.kind = 3
   and ta.xtrsrc_id = e.xtrsrcid
   and not r.deleted
   and r.source_kind = 1
   and r.runcatid = ta.runcat_id
   and ta.lr_method = 2
union
select e.xtrsrcid, r.runcatid,
       $$get_distance('r', 'e')$$ as distance_arcsec,
       ta.lr_method, 0.0 as distance
  from extractedsources e,
       temp_associations ta,
       runningcatalog r
 where ta.kind = 3
   and ta.xtrsrc_id = e.xtrsrcid
   and not r.deleted
   and r.source_kind = 1
   and r.runcatid = ta.runcat_id
   and ta.lr_method = 3;

--update all old associations with flux_fraction
UPDATE assocxtrsources
SET    weight = weight * (SELECT ta.flux_fraction
                          FROM   temp_associations ta,assocxtrsources a,runningcatalog r
                          WHERE  a.runcat_id = assocxtrsources.runcat_id
                                 AND a.xtrsrc_id <> assocxtrsources.xtrsrc_id
                                 AND ta.runcat_id = a.runcat_id
                                 AND ta.xtrsrc_id = a.xtrsrc_id
                                 AND r.first_xtrsrc_id <> ta.xtrsrc_id
                                 AND r.runcatid = ta.runcat_id
                                 AND ta.lr_method < 3
                                 AND ta.kind = 3)
WHERE EXISTS (SELECT ta.flux_fraction
                FROM temp_associations ta,assocxtrsources a,runningcatalog r
               WHERE  a.runcat_id = assocxtrsources.runcat_id
                      AND a.xtrsrc_id <> assocxtrsources.xtrsrc_id
                      AND ta.runcat_id = a.runcat_id
                      AND ta.xtrsrc_id = a.xtrsrc_id
                      AND r.first_xtrsrc_id <> ta.xtrsrc_id
                      AND r.runcatid = ta.runcat_id
                      AND ta.lr_method < 3
                      AND ta.kind = 3);

--update old fluxes with new weights
update runningcatalog_fluxes
   set $$get_column_update_total(['f_peak', 'f_int'], True)$$
 where exists (select 1
                 from extractedsources e,
                      temp_associations ta
                where e.xtrsrcid = ta.xtrsrc_id
                  and ta.runcat_id = runningcatalog_fluxes.runcat_id
                  and ta.kind = 3)
   and band <> {0};

update runningcatalog_fluxes
   set $$get_column_update_second(['f_peak', 'f_int'])$$
 where exists (select 1
                 from extractedsources e,
                      temp_associations ta
                where e.xtrsrcid = ta.xtrsrc_id
                  and ta.runcat_id = runningcatalog_fluxes.runcat_id
                  and ta.kind = 3)
   and band <> {0};

--insert old fluxes for new sources
insert into runningcatalog_fluxes(runcat_id, band, datapoints,
                                  wm_f_peak, wm_f_peak_err,
                                  avg_wf_peak, avg_weight_f_peak,
                                  wm_f_int, wm_f_int_err,
                                  avg_wf_int, avg_weight_f_int)
select r.runcatid, f.band, f.datapoints,
       ta.flux_fraction*wm_f_peak, wm_f_peak_err,
       ta.flux_fraction*avg_wf_peak, avg_weight_f_peak,
       ta.flux_fraction*wm_f_int, wm_f_int_err,
       ta.flux_fraction*avg_wf_int, avg_weight_f_int
  from runningcatalog_fluxes f,
       runningcatalog r,
       temp_associations ta
 where ta.runcat_id = f.runcat_id
   and ta.xtrsrc_id = r.first_xtrsrc_id
   and f.band <> {0}
   and ta.kind = 3 ;
