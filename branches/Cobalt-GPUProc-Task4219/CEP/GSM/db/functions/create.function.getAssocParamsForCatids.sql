create function getAssocParamsForCatids(icatsrcid1 INT
                                       ,icatsrcid2 INT
                                       ) RETURNS TABLE (assoc_distance_arcsec DOUBLE
                                                       ,assoc_r DOUBLE
                                                       ,assoc_log_lr DOUBLE
                                                       )

begin

  return table (
    select 3600 * DEGREES(2 * ASIN(SQRT( (c1.x - c2.x) * (c1.x - c2.x)
                                   + (c1.y - c2.y) * (c1.y - c2.y)
                                   + (c1.z - c2.z) * (c1.z - c2.z)
                                   )
                               / 2
                              )
                     ) AS assoc_distance_arcsec
          ,3600 * SQRT( (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl))) * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl)))
                 / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err)
               + (c1.decl - c2.decl) * (c1.decl - c2.decl)  
                 / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err)
               ) AS assoc_r
          ,LOG10(EXP((( (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl))) * (c1.ra * COS(RADIANS(c1.decl)) - c2.ra * COS(RADIANS(c2.decl))) 
                        / (c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err)
                       + (c1.decl - c2.decl) * (c1.decl - c2.decl) 
                        / (c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err)
                      )
                     ) / 2
                    )
                 /
                 (2 * PI() * SQRT(c1.ra_err * c1.ra_err + c2.ra_err * c2.ra_err) * SQRT(c1.decl_err * c1.decl_err + c2.decl_err * c2.decl_err) * 4.02439375E-06)
                ) AS assoc_log_lr
  from catalogedsources c1
      ,catalogedsources c2 
 where c1.catsrcid = icatsrcid1
   and c2.catsrcid = icatsrcid2
  )
  ;

END
;

