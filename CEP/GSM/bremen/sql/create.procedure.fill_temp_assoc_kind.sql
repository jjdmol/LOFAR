drop procedure fill_temp_assoc_kind;
create procedure fill_temp_assoc_kind()
begin
update temp_associations
   set xtr_count = (select count(xtrsrc_id)
                      from temp_associations as ta
                     where ta.xtrsrc_id = temp_associations.xtrsrc_id);

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
                where ta.xtrsrc_id = temp_associations.xtrsrc_id
                  and kind = 4);

update temp_associations
   set group_head_id = runcat_id
 where kind = 4
   and group_head_id is null;
end;
