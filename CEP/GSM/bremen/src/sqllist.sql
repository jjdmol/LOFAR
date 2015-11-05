--#insert_extractedsources
insert into extractedsources (image_id, zone, ra, decl, ra_err, decl_err,
                              x, y, z, det_sigma, f_peak, f_peak_err, source_kind)
select %s, cast(ldecl as integer) as zone, lra, ldecl, lra_err, ldecl_err,
       cos(radians(ldecl))*cos(radians(lra)),
       cos(radians(ldecl))*sin(radians(lra)),
       sin(radians(ldecl)), 3.0, lf_peak, lf_peak_err, 0
  from detections;


--#runcat_cursor
select a.runcat_id, count(*) as xcount, sum(e.ra), sum(e.decl)
  from assocxtrsources a,
       extractedsources e
 where a.xtrsrc_id = e.xtrsrcid
   and e.image_id = {0}
group by runcat_id;


--#update runningcat cursor
select a.runcat_id, 1/(e.ra_err*e.ra_err), 1/(e.decl_err*e.decl_err),
       e.ra/(e.ra_err*e.ra_err), e.decl/(e.decl_err*e.decl_err)
  from assocxtrsources a,
       extractedsources e,
       runningcatalog r
 where a.xtrsrc_id = e.xtrsrcid
   and r.runcatid = a.runcat_id
   and r.group_head_id is null
   and e.image_id = {0};

--#update runningcatalog
update runningcatalog
   set datapoints = datapoints + 1,
       $$_get_column_update('ra', '{1}', '{3}')$$,
       $$_get_column_update('decl', '{2}', '{4}')$$
 where runcatid = {0};


--#add 1 to 1
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select ta.xtrsrc_id, ta.runcat_id, ta.distance_arcsec, ta.lr_method, ta.r
  from temp_associations ta
 where kind = 1;


--#flux cursor
select a.runcatid, f.datapoints, f.avg_f_peak
  from runningcatalog a
  left outer join runningcatalog_fluxes f on (f.runcat_id = a.runcatid
                                          and f.band = {1})
 where a.runcatid = {0};


--#insert flux
insert into runningcatalog_fluxes(runcat_id, band, datapoints, avg_f_peak, avg_weight_f_peak)
select a.runcat_id, {0}, 1, e.f_peak, 1/(e.f_peak_err*e.f_peak_err)
  from extractedsources e,
       assocxtrsources a
 where a.runcat_id = {1}
   and a.xtrsrc_id = e.xtrsrcid
   and e.image_id = {2};


--#update flux
update runningcatalog_fluxes
   set datapoints = datapoints + 1
 where runcat_id = %s
   and band = %s;

--#select flux for update
select count(*) as datapoints, sum(f_peak/(f_peak_err*f_peak_err)), sum(1/(f_peak_err*f_peak_err))
  from extractedsources e,
       assocxtrsources a
 where a.runcat_id = {0}
   and a.xtrsrc_id = e.xtrsrcid
   and e.band = {1};


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
insert into runningcatalog(first_xtrsrc_id, datapoints, decl_zone,
                           wm_ra, wm_decl, wm_ra_err, wm_decl_err,
                           avg_wra, avg_wdecl, avg_weight_ra, avg_weight_decl,
                           x, y, z, source_kind)
select e.xtrsrcid, 1, zone, ra, decl, ra_err, decl_err,
       ra/(ra_err*ra_err), decl/(decl_err*decl_err), 1/(ra_err*ra_err), 1/(decl_err*decl_err),
       x, y, z, source_kind
  from extractedsources e,
       temp_associations ta
 where ta.xtrsrc_id = e.xtrsrcid
   and ta.kind = 3
   and ta.xtrsrc_id not in (select tx.min_id
                              from (select tb.runcat_id, min(tb.xtrsrc_id) as min_id
                                      from temp_associations tb
                                     where tb.kind = 3
                                  group by tb.runcat_id) tx);

--copy fluxes from old to new sources
insert into runningcatalog_fluxes(runcat_id, band, datapoints, avg_f_peak, avg_weight_f_peak)
select r.runcatid, f.band, f.datapoints, f.avg_f_peak, f.avg_weight_f_peak
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
select a.xtrsrc_id, r.runcatid, $$_get_distance('r', 'e')$$, 2, 0.0
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
select e.xtrsrcid, ta.runcat_id, $$_get_distance('r', 'e')$$,  2, 0.0
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


--#GroupFinder
select xtrsrc_id, runcat_id, group_head_id
  from temp_associations
 where kind = 4;

--#GroupUpdate
update temp_associations
   set group_head_id = {0}
 where kind = 4
   and runcat_id in ({1});

update runningcatalog
   set group_head_id = {0}
 where runcatid in ({1});

--#GroupFill
insert into assocxtrsources(xtrsrc_id, runcat_id, distance_arcsec, lr_method, r)
select xtrsrc_id, runcat_id, distance_arcsec, 4, r
  from temp_associations
 where kind = 4;
