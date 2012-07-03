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
       runningcatalog_fluxes f
 WHERE a1.xtrsrc_id = e1.xtrsrcid
   AND a1.runcat_id = r1.runcatid
   AND a2.xtrsrc_id = e1.xtrsrcid
   AND a2.runcat_id = r2.runcatid
   AND r2.parent_runcat_id = r1.runcatid
   AND f.runcat_id = r2.runcatid
   AND f.band = r2.band;
