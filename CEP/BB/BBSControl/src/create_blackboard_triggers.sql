CREATE OR REPLACE FUNCTION blackboard.notify_client() RETURNS TRIGGER AS
$$
    BEGIN
        EXECUTE 'NOTIFY ' || quote_ident(TG_NAME);
        RETURN NULL;
    END;
$$
LANGUAGE plpgsql;


CREATE TRIGGER trigger_work_order AFTER INSERT ON blackboard.work_order FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();
