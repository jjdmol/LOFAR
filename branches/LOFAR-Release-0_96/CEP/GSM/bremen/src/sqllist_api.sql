--#APIimage
select i.imagename, count(*) as source_count, min(f_peak) as min_flux
  from images i,
       extractedsources e
 where i.imageid = {0}
   and e.image_id = i.imageid
group by imagename;
