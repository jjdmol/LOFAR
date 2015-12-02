CREATE VIEW v_catalog_info AS
SELECT a1.xtrsrc_id,
       a2.runcat_id,
       a1.runcat_id AS parent_id,
       r2.band,
       r1.datapoints,
       r2.datapoints AS per_band_datapoints,
       f.datapoints AS flux_datapoints,
       e1.image_id
  FROM extractedsources e1,
       assocxtrsources a1,
       runningcatalog r1,
       assocxtrsources a2,
       runningcatalog r2,
       runningcatalog_fluxes f,
       images i
 WHERE a1.xtrsrc_id = e1.xtrsrcid
   AND a1.runcat_id = r1.runcatid
   AND a2.xtrsrc_id = e1.xtrsrcid
   AND a2.runcat_id = r2.runcatid
   AND r2.parent_runcat_id = r1.runcatid
   AND f.runcat_id = r2.runcatid
   AND f.band = r2.band
   and f.stokes = r2.stokes
   and not r1.deleted
   and not r2.deleted
   and r1.group_head_id is null
   and e1.image_id = i.imageid
   and i.status = 1
union
select a1.xtrsrc_id,
       a1.runcat_id,
       r1.runcatid as parent_id,
       i.band,
       r1.datapoints,
       f.datapoints as per_band_datapoints,
       f.datapoints as flux_datapoints,
       e.image_id
  from assocxtrsources a1,
       runningcatalog r1,
       runningcatalog_fluxes f,
       extractedsources e,
       images i
 where e.image_id = i.imageid
   and e.xtrsrcid = a1.xtrsrc_id
   and a1.runcat_id = r1.runcatid
   and f.runcat_id = r1.runcatid
   and f.band = i.band
   and f.stokes = i.stokes
   and not r1.deleted
   and r1.source_kind = 0
   and r1.group_head_id is null
   and i.status = 1
