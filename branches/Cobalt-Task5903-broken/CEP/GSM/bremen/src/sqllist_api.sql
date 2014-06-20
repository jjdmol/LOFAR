--#APIimage
select i.imagename,
       count(*) as source_count,
       min(f_peak) as min_flux,
       count(r.runcatid) as new_source_count
  from images i
  inner join extractedsources e on (e.image_id = i.imageid)
  left outer join runningcatalog r on (r.first_xtrsrc_id = e.xtrsrcid and not r.deleted)
 where i.imageid = {0}
group by imagename;
