--#insert new bands for point sources
insert into runningcatalog_fluxes(runcat_id, band, datapoints,
                                  wm_f_peak, wm_f_peak_err,
                                  avg_wf_peak, avg_weight_f_peak,
                                  wm_f_int, wm_f_int_err,
                                  avg_wf_int, avg_weight_f_int)
select a.runcat_id, [b], 1,
       e.f_peak, e.f_peak_err,
       e.f_peak/(e.f_peak_err*e.f_peak_err), 1/(e.f_peak_err*e.f_peak_err),
       e.f_int, e.f_int_err,
       e.f_int/(e.f_int_err*e.f_int_err), 1/(e.f_int_err*e.f_int_err)
  from extractedsources e,
       assocxtrsources a,
       temp_associations ta,
       images i
 where a.xtrsrc_id = e.xtrsrcid
   and ta.xtrsrc_id = a.xtrsrc_id
   and ta.runcat_id = a.runcat_id
   and ta.image_id = [i]
   and ta.kind <> 4
   and i.imageid = e.image_id
   and not exists (select f.band
                     from runningcatalog_fluxes f
                    where f.runcat_id = a.runcat_id
                      and f.band = i.band
                      and f.stokes = i.stokes)
   and ta.lr_method = 1
   and e.image_id = [i];


--#update runningcatalog
update runningcatalog
   set datapoints = runningcatalog.datapoints + 1,
       $$get_column_update('ra', 'e.ra','e.ra_err')$$,
       $$get_column_update('decl', 'e.decl','e.decl_err')$$
  from assocxtrsources a,
       extractedsources e
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog.runcatid = a.runcat_id
   and runningcatalog.group_head_id is null
   and runningcatalog.source_kind = 0
   and e.image_id = [i];

--#update runningcatalog extended
update runningcatalog
   set datapoints = datapoints + y.dp,
       $$get_column_update2(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$
  from (select r.runcatid, count(*) as dp,
               $$get_column_from(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$
          from assocxtrsources a,
               extractedsources e,
               runningcatalog r
         where a.xtrsrc_id = e.xtrsrcid
           and r.runcatid = a.runcat_id
           and r.group_head_id is null
           and r.source_kind <> 0
           and e.image_id = [i]
        group by runcatid) as y
where y.runcatid = runningcatalog.runcatid;


--#update runningcatalog XYZ
update runningcatalog
   set x = cos(radians(runningcatalog.wm_decl))*cos(radians(runningcatalog.wm_ra)),
       y = cos(radians(runningcatalog.wm_decl))*sin(radians(runningcatalog.wm_ra)),
       z = sin(radians(runningcatalog.wm_decl)),
       last_update_date = current_timestamp
 where exists (select e.image_id
  from assocxtrsources a,
       extractedsources e
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog.runcatid = a.runcat_id
   and runningcatalog.group_head_id is null
   and e.image_id = [i]);


--#update runningcatalog_fluxes
update runningcatalog_fluxes
   set datapoints = datapoints + y.dp,
       $$get_column_update2(['f_peak', 'f_int'])$$
  from (select f.runcat_id, f.band, f.stokes, count(*) as dp,
               $$get_column_from(['f_peak', 'f_int'])$$
        from assocxtrsources a,
             extractedsources e,
             images i,
             runningcatalog_fluxes f
       where a.xtrsrc_id = e.xtrsrcid
         and f.runcat_id = a.runcat_id
         and e.image_id = [i]
         and i.imageid = [i]
         and a.lr_method <> 5 --not a group association
         and f.band = i.band
         and f.stokes = i.stokes
         group by f.runcat_id, f.stokes, f.band) as y
where y.runcat_id = runningcatalog_fluxes.runcat_id
  and y.band = runningcatalog_fluxes.band
  and y.stokes = runningcatalog_fluxes.stokes;
