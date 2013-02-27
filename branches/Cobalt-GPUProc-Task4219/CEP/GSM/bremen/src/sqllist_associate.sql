--#Associate point
INSERT INTO temp_associations (xtrsrc_id, xtrsrc_id2, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id, image_id)
SELECT e.xtrsrcid, coalesce(e.xtrsrcid2, e.xtrsrcid), rc.runcatid,
$$get_distance('rc', 'e')$$ AS assoc_distance_arcsec, 1,
$$get_assoc_r('rc', 'e')$$ as assoc_r,
       rc.group_head_id, [i]
  FROM runningcatalog rc
      ,extractedsources e
      ,images i
 WHERE e.image_id = [i]
   and i.imageid = [i]
   and rc.x between e.x - {0} and e.x + {0}
   and rc.y between e.y - {0} and e.y + {0}
   and rc.z between e.z - {0} and e.z + {0}
   and e.source_kind = 0
   and rc.source_kind = 0
   and not rc.deleted
   AND rc.decl_zone BETWEEN CAST(FLOOR(e.decl - i.fov_radius) AS INTEGER)
                        AND CAST(FLOOR(e.decl + i.fov_radius) AS INTEGER)
   and rc.healpix_zone in ({2})
 AND $$get_assoc_r('rc', 'e')$$ < {1};


--#Associate extended
--No "copied" sources included, as there is a "good" distance function.
--first associate per-band
INSERT INTO temp_associations (xtrsrc_id, xtrsrc_id2, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id, image_id)
SELECT e.xtrsrcid, e.xtrsrcid, rc.runcatid,
$$get_distance('rc', 'e')$$ AS assoc_distance_arcsec, 2,
$$get_assoc_r_extended('rc', 'e')$$  as assoc_r,
       rc.group_head_id, [i]
  FROM runningcatalog rc
      ,extractedsources e
      ,images i
 WHERE e.image_id = [i]
   and i.imageid = [i]
   and rc.x between e.x - {0} and e.x + {0}
   and rc.y between e.y - {0} and e.y + {0}
   and rc.z between e.z - {0} and e.z + {0}
   and e.source_kind = 1
   and rc.source_kind = 1
   and rc.band = [b]
   and rc.stokes = '[s]'
   AND rc.decl_zone BETWEEN CAST(FLOOR(e.decl - i.fov_radius) AS INTEGER)
                        AND CAST(FLOOR(e.decl + i.fov_radius) AS INTEGER)
   and rc.healpix_zone in ({2})
   and not rc.deleted
   and e.xtrsrcid2 is null
 AND $$get_assoc_r_extended('rc', 'e')$$ < {1};

--if no match was found for this band, then use cross-band source.
INSERT INTO temp_associations (xtrsrc_id, xtrsrc_id2, runcat_id,
                               distance_arcsec,
                               lr_method, r, group_head_id, image_id)
SELECT e.xtrsrcid, e.xtrsrcid, rc.runcatid,
$$get_distance('rc', 'e')$$ AS assoc_distance_arcsec, 3,
$$get_assoc_r_extended('rc', 'e')$$  as assoc_r,
       rc.group_head_id, [i]
  FROM runningcatalog rc
      ,extractedsources e
      ,images i
 WHERE e.image_id = [i]
   and i.imageid = [i]
   and rc.x between e.x - {0} and e.x + {0}
   and rc.y between e.y - {0} and e.y + {0}
   and rc.z between e.z - {0} and e.z + {0}
   and e.source_kind = 1
   and rc.source_kind = 1
   and rc.band is null
   and rc.stokes is null
   and not rc.deleted
   and e.xtrsrcid2 is null
   AND rc.decl_zone BETWEEN CAST(FLOOR(e.decl - i.fov_radius) AS INTEGER)
                        AND CAST(FLOOR(e.decl + i.fov_radius) AS INTEGER)
   and rc.healpix_zone in ({2})
   and not exists (select ta.runcat_id
                     from temp_associations ta
                    where ta.xtrsrc_id = e.xtrsrcid)
 AND $$get_assoc_r_extended('rc', 'e')$$ < {1};
