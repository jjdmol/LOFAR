--add triggers and trigger functions to radb (note, there are also the notification triggers in the add_notifications.sql file)

DROP TRIGGER IF EXISTS trigger_delete_resource_claims_for_approved_task ON resource_allocation.task CASCADE;
DROP FUNCTION IF EXISTS resource_allocation.delete_resource_claims_for_approved_task();

CREATE OR REPLACE FUNCTION resource_allocation.delete_resource_claims_for_approved_task()
  RETURNS trigger AS
$BODY$
BEGIN
  IF NEW.status_id <> OLD.status_id AND NEW.status_id = 300 THEN
    DELETE FROM resource_allocation.resource_claim rc WHERE rc.task_id = NEW.id;
  END IF;
RETURN NEW;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION resource_allocation.delete_resource_claims_for_approved_task()
  OWNER TO resourceassignment;
COMMENT ON FUNCTION resource_allocation.delete_resource_claims_for_approved_task()
  IS 'function which is called by task table update trigger, which deletes all the tasks resource claims.';

CREATE TRIGGER trigger_delete_resource_claims_for_approved_task
  AFTER UPDATE
  ON resource_allocation.task
  FOR EACH ROW
  EXECUTE PROCEDURE resource_allocation.delete_resource_claims_for_approved_task();
COMMENT ON TRIGGER trigger_delete_resource_claims_for_approved_task ON resource_allocation.task
  IS 'task table update trigger, calls the resource_allocation.delete_resource_claims_for_approved_task() function.';

