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

---------------------------------------------------------------------------------------------------------------------

DROP TRIGGER IF EXISTS trigger_delete_conflict_reasons_after_resource_claim_update ON resource_allocation.resource_claim CASCADE;
DROP FUNCTION IF EXISTS resource_allocation.delete_conflict_reasons_after_resource_claim_update();

CREATE OR REPLACE FUNCTION resource_allocation.delete_conflict_reasons_after_resource_claim_update()
  RETURNS trigger AS
$BODY$
BEGIN
  IF OLD.status_id = 2 AND NEW.status_id <> 2 THEN   --new status is not conflict
    DELETE FROM resource_allocation.resource_claim_conflict_reason rccr WHERE rccr.resource_claim_id = NEW.id;
  END IF;
RETURN NEW;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION resource_allocation.delete_conflict_reasons_after_resource_claim_update()
  OWNER TO resourceassignment;
COMMENT ON FUNCTION resource_allocation.delete_conflict_reasons_after_resource_claim_update()
  IS 'function which is called by resource_claim table update trigger, which deletes resource_claim_conflict_reasons when the claim status is updated to !conflict.';

CREATE TRIGGER trigger_delete_conflict_reasons_after_resource_claim_update
  AFTER UPDATE
  ON resource_allocation.resource_claim
  FOR EACH ROW
  EXECUTE PROCEDURE resource_allocation.delete_conflict_reasons_after_resource_claim_update();

---------------------------------------------------------------------------------------------------------------------

DROP TRIGGER IF EXISTS trigger_before_insert_conflict_reason_do_resource_claim_status_check ON resource_allocation.resource_claim_conflict_reason CASCADE;
DROP FUNCTION IF EXISTS resource_allocation.before_insert_conflict_reason_do_resource_claim_status_check();

CREATE OR REPLACE FUNCTION resource_allocation.before_insert_conflict_reason_do_resource_claim_status_check()
  RETURNS trigger AS
$BODY$
BEGIN
  -- check if referred resource_claim is in conflict status, else raise
  IF (SELECT COUNT(id) FROM resource_allocation.resource_claim rc WHERE rc.id = NEW.resource_claim_id AND rc.status_id = 2) = 0 THEN
    RAISE EXCEPTION 'resource_claim has no conflict status';
  END IF;
RETURN NEW;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION resource_allocation.before_insert_conflict_reason_do_resource_claim_status_check()
  OWNER TO resourceassignment;
COMMENT ON FUNCTION resource_allocation.before_insert_conflict_reason_do_resource_claim_status_check()
  IS 'check if referred resource_claim is in conflict status, else raise';

CREATE TRIGGER trigger_before_insert_conflict_reason_do_resource_claim_status_check
  BEFORE INSERT
  ON resource_allocation.resource_claim_conflict_reason
  FOR EACH ROW
  EXECUTE PROCEDURE resource_allocation.before_insert_conflict_reason_do_resource_claim_status_check();

---------------------------------------------------------------------------------------------------------------------

DROP TRIGGER IF EXISTS trigger_delete_conflict_reasons_after_task_update ON resource_allocation.task CASCADE;
DROP FUNCTION IF EXISTS resource_allocation.delete_conflict_reasons_after_task_update();

CREATE OR REPLACE FUNCTION resource_allocation.delete_conflict_reasons_after_task_update()
  RETURNS trigger AS
$BODY$
BEGIN
  IF OLD.status_id = 335 AND NEW.status_id <> 335 THEN   --new status is not conflict
    DELETE FROM resource_allocation.task_conflict_reason tcr WHERE tcr.task_id = NEW.id;
  END IF;
RETURN NEW;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION resource_allocation.delete_conflict_reasons_after_task_update()
  OWNER TO resourceassignment;
COMMENT ON FUNCTION resource_allocation.delete_conflict_reasons_after_task_update()
  IS 'function which is called by task table update trigger, which deletes task_conflict_reasons when the task status is updated to !conflict.';

CREATE TRIGGER trigger_delete_conflict_reasons_after_task_update
  AFTER UPDATE
  ON resource_allocation.task
  FOR EACH ROW
  EXECUTE PROCEDURE resource_allocation.delete_conflict_reasons_after_task_update();

---------------------------------------------------------------------------------------------------------------------

DROP TRIGGER IF EXISTS trigger_before_insert_conflict_reason_do_task_status_check ON resource_allocation.task_conflict_reason CASCADE;
DROP FUNCTION IF EXISTS resource_allocation.before_insert_conflict_reason_do_task_status_check();

CREATE OR REPLACE FUNCTION resource_allocation.before_insert_conflict_reason_do_task_status_check()
  RETURNS trigger AS
$BODY$
BEGIN
  -- check if referred task is in conflict status, else raise
  IF (SELECT COUNT(id) FROM resource_allocation.task task WHERE task.id = NEW.task_id AND task.status_id = 335) = 0 THEN
    RAISE EXCEPTION 'task has no conflict status';
  END IF;
RETURN NEW;
END;
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION resource_allocation.before_insert_conflict_reason_do_task_status_check()
  OWNER TO resourceassignment;
COMMENT ON FUNCTION resource_allocation.before_insert_conflict_reason_do_task_status_check()
  IS 'check if referred task is in conflict status, else raise';

CREATE TRIGGER trigger_before_insert_conflict_reason_do_task_status_check
  BEFORE INSERT
  ON resource_allocation.task_conflict_reason
  FOR EACH ROW
  EXECUTE PROCEDURE resource_allocation.before_insert_conflict_reason_do_task_status_check();

---------------------------------------------------------------------------------------------------------------------
