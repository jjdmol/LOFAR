--#deduct runningcatalog
update runningcatalog
   set datapoints = runningcatalog.datapoints - 1,
       $$get_column_deduct('ra', 'e.ra','e.ra_err')$$,
       $$get_column_deduct('decl', 'e.decl','e.decl_err')$$
  from assocxtrsources a,
       extractedsources e
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog.runcatid = a.runcat_id
   and runningcatalog.group_head_id is null
   and runningcatalog.source_kind = 0
   and e.image_id = {0};

--#deduct runningcatalog non-zero
update runningcatalog
   set $$get_column_deduct_nonzero('ra', 'e.ra','e.ra_err')$$,
       $$get_column_deduct_nonzero('decl', 'e.decl','e.decl_err')$$
  from assocxtrsources a,
       extractedsources e
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog.runcatid = a.runcat_id
   and runningcatalog.group_head_id is null
   and runningcatalog.source_kind = 0
   and runningcatalog.datapoints <> 0
   and e.image_id = {0};


--#deduct runningcatalog extended
update runningcatalog
   set datapoints = datapoints - y.dp,
       $$get_column_deduct2(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$
  from (select r.runcatid, count(*) as dp,
               $$get_column_from(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$
          from assocxtrsources a,
               extractedsources e,
               runningcatalog r
         where a.xtrsrc_id = e.xtrsrcid
           and r.runcatid = a.runcat_id
           and r.group_head_id is null
           and r.source_kind <> 0
           and e.image_id = {0}
        group by runcatid) as y
where y.runcatid = runningcatalog.runcatid;

--#deduct runningcatalog extended non-zero
update runningcatalog
   set $$get_column_deduct2_nonzero(['ra', 'decl', 'g_minor', 'g_major','g_pa'])$$
  from assocxtrsources a,
       extractedsources e
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog.runcatid = a.runcat_id
   and runningcatalog.group_head_id is null
   and runningcatalog.source_kind <> 0
   and runningcatalog.datapoints <> 0
   and e.image_id = {0};

--#deduct runningcatalog_fluxes
update runningcatalog_fluxes
   set datapoints = datapoints - y.dp,
       $$get_column_deduct2(['f_peak', 'f_int'])$$
  from (select f.runcat_id, f.band, f.stokes, count(*) as dp,
               $$get_column_from(['f_peak', 'f_int'])$$
        from assocxtrsources a,
             extractedsources e,
             images i,
             runningcatalog_fluxes f
       where a.xtrsrc_id = e.xtrsrcid
         and f.runcat_id = a.runcat_id
         and e.image_id = {0}
         and e.image_id = i.imageid
         and a.lr_method <> 5 --not a group association
         and f.band = i.band
         and f.stokes = i.stokes
         group by f.runcat_id, f.stokes, f.band) as y
where y.runcat_id = runningcatalog_fluxes.runcat_id
  and y.band = runningcatalog_fluxes.band
  and y.stokes = runningcatalog_fluxes.stokes;

--#deduct runningcatalog_fluxes non-zero
update runningcatalog_fluxes
   set $$get_column_deduct2_nonzero(['f_peak', 'f_int'])$$
  from assocxtrsources a,
       extractedsources e,
       images i
 where a.xtrsrc_id = e.xtrsrcid
   and runningcatalog_fluxes.runcat_id = a.runcat_id
   and runningcatalog_fluxes.stokes = i.stokes
   and runningcatalog_fluxes.band = i.band
   and i.imageid = e.image_id
   and runningcatalog_fluxes.datapoints <> 0
   and e.image_id = {0};


--#deduct cleanup
delete from assocxtrsources
 where exists (select 1 from extractedsources e
                where e.image_id = {0}
                  and e.xtrsrcid = assocxtrsources.xtrsrc_id);

--#deduct remove extractedsources
delete from extractedsources
 where image_id = {0}
   and not exists (select 1 from assocxtrsources a
                    where a.xtrsrc_id = extractedsources.xtrsrcid);
