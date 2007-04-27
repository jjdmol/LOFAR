CREATE OR REPLACE FUNCTION blackboard.notify() RETURNS TRIGGER AS
$$
    BEGIN
        EXECUTE 'NOTIFY ' || quote_ident(TG_NAME);
        RETURN NULL;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.notify_result() RETURNS TRIGGER AS
$$
    DECLARE
        cmd_type    TEXT;
    BEGIN
        cmd_type := "Type" FROM blackboard.command WHERE id = NEW.command_id;
        
        IF cmd_type ~ 'initialize|finalize|nextchunk' THEN
--            RAISE NOTICE 'NOTIFY %', quote_ident(TG_NAME) || '_' || cmd_type;
            EXECUTE 'NOTIFY ' || quote_ident(TG_NAME) || '_' || cmd_type;    
        END IF;

        RETURN NULL;
    END;
$$
LANGUAGE plpgsql;

CREATE TRIGGER insert_command
    AFTER INSERT ON blackboard.command
    FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify();

CREATE TRIGGER insert_result
    AFTER INSERT ON blackboard.result
    FOR EACH ROW EXECUTE PROCEDURE blackboard.notify_result();
