CREATE OR REPLACE FUNCTION blackboard.notify_client() RETURNS TRIGGER AS
$$
    BEGIN
        EXECUTE 'NOTIFY ' || quote_ident(TG_NAME);
        RETURN NULL;
    END;
$$
LANGUAGE plpgsql;

CREATE TRIGGER modify_worker_register
    AFTER INSERT OR UPDATE OR DELETE ON blackboard.worker
    FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();

CREATE TRIGGER insert_command
    AFTER INSERT ON blackboard.command
    FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();

CREATE TRIGGER insert_result
    AFTER INSERT ON blackboard.result
    FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();

