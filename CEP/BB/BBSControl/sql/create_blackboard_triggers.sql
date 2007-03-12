CREATE FUNCTION blackboard.check_strategy_constraint() RETURNS TRIGGER AS
$$
    BEGIN
        IF (SELECT COUNT(*) FROM blackboard.strategy) <> 0 THEN
            RAISE EXCEPTION 'Only one strategy can exist at any time.';
        END IF;
        
        RETURN NEW;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.notify_client() RETURNS TRIGGER AS
$$
    BEGIN
        EXECUTE 'NOTIFY ' || quote_ident(TG_NAME);
        RETURN NULL;
    END;
$$
LANGUAGE plpgsql;


CREATE TRIGGER insert_strategy BEFORE INSERT ON blackboard.strategy FOR EACH ROW EXECUTE PROCEDURE blackboard.check_strategy_constraint();

CREATE TRIGGER insert_command AFTER INSERT ON blackboard.command FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();
CREATE TRIGGER insert_result AFTER INSERT ON blackboard.result FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();
