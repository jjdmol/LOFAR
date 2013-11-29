--#PG insert new bands
insert into runningcatalog_fluxes(runcat_id, band, datapoints,
                                  wm_f_peak, wm_f_peak_err,
                                  avg_wf_peak, avg_weight_f_peak,
                                  wm_f_int, wm_f_int_err,
                                  avg_wf_int, avg_weight_f_int)
select a.runcat_id, {1}, 1,
       e.f_peak, e.f_peak_err,
       e.f_peak/(e.f_peak_err*e.f_peak_err), 1/(e.f_peak_err*e.f_peak_err),
       e.f_int, e.f_int_err,
       e.f_int/(e.f_int_err*e.f_int_err), 1/(e.f_int_err*e.f_int_err)
  from extractedsources e,
       assocxtrsources a,
       temp_associations ta
 where a.xtrsrc_id = e.xtrsrcid
   and ta.xtrsrc_id = a.xtrsrc_id
   and ta.runcat_id = a.runcat_id
   and ta.kind <> 4
   and e.image_id = {0}
   and not exists (select f.datapoints
                     from runningcatalog_fluxes f
                    where f.runcat_id = a.runcat_id
                      and f.band = {1}
                      and f.stokes = 'I');

--#PG update runningcatalog
update runningcatalog
   set datapoints = runningcatalog.datapoints + 1,
       $$get_column_update_pg('ra', 'e.ra','e.ra_err')$$,
       $$get_column_update_pg('decl', 'e.decl','e.decl_err')$$
  from assocxtrsources a,
       extractedsources e
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog.runcatid = a.runcat_id
   and runningcatalog.group_head_id is null
   and e.image_id = {0};

--#PG update runningcatalog_fluxes
update runningcatalog_fluxes
   set datapoints = runningcatalog_fluxes.datapoints + 1,
       $$get_column_update_pg('f_peak', 'e.f_peak','e.f_peak_err')$$,
       $$get_column_update_pg('f_int', 'e.f_int','e.f_int_err')$$
  from assocxtrsources a,
       extractedsources e,
       runningcatalog r
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog_fluxes.runcat_id = a.runcat_id
   and r.runcatid = a.runcat_id
   and r.group_head_id is null
   and runningcatalog_fluxes.band = {1}
   and e.image_id = {0};
