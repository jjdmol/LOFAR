-- Function: fill_temp_assoc_kind()

-- DROP FUNCTION fill_temp_assoc_kind();

CREATE OR REPLACE FUNCTION fill_temp_assoc_kind()
  RETURNS void AS
$BODY$begin
update temp_associations
   set xtr_count = (select count(xtrsrc_id2)
                      from temp_associations as ta
                     where ta.xtrsrc_id2 = temp_associations.xtrsrc_id2);

update temp_associations
   set run_count = (select count(runcat_id)
                      from temp_associations as ta
                     where ta.runcat_id = temp_associations.runcat_id);

update temp_associations
   set kind = 1
 where xtr_count = 1
   and run_count = 1;

update temp_associations
   set kind = 2
 where xtr_count > 1
   and run_count = 1;

update temp_associations
   set kind = 3
 where xtr_count = 1
   and run_count > 1;

update temp_associations
   set kind = 4
 where xtr_count > 1
   and run_count > 1;

--complete groups
update temp_associations
   set kind = 4
 where kind <> 4
   and exists (select kind from temp_associations ta
                where ta.runcat_id = temp_associations.runcat_id
                  and kind = 4);

update temp_associations
   set kind = 4
 where kind <> 4
   and exists (select kind from temp_associations ta
                where ta.xtrsrc_id2 = temp_associations.xtrsrc_id2
                  and kind = 4);

update temp_associations
   set group_head_id = runcat_id
 where kind = 4
   and group_head_id is null;

--extended sources merging
update temp_associations
   set kind = 5
 where lr_method = 3
   and kind = 2
   and not exists (select r.band
                     from runningcatalog r,
                          temp_associations ta,
                          runningcatalog r2
                    where r.parent_runcat_id = temp_associations.runcat_id
                      and ta.xtrsrc_id2 = temp_associations.xtrsrc_id2
                      and ta.runcat_id <> temp_associations.runcat_id
                      and r2.parent_runcat_id = ta.runcat_id
                      and r.band = r2.band
                      and r.stokes = r2.stokes
                      and ta.kind = 2
                      and ta.lr_method = 3
                );
end;
$BODY$
LANGUAGE plpgsql VOLATILE
COST 100;
ALTER FUNCTION fill_temp_assoc_kind()
OWNER TO postgres;
