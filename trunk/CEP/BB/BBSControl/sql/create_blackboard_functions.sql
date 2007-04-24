-- ------- --
-- CONTROL --
-------------

-- Check if the global or a local controller is started cleanly,
-- or if it is restarted after failure. For the global controller,
-- startup is considered clean if no strategy has been set yet.
-- For the local controller, this check does not suffice, because
-- if the global controller was started before the local controller
-- a strategy could already have been set. Therefore, it is verified
-- that the local controller hasn't completed any commands yet by
-- checking the result table. NOTE: This is also not foolproof, if
-- the first command modifies data/state (because it can be partly
-- executed).
CREATE OR REPLACE FUNCTION blackboard.is_new_run(global BOOL)
RETURNS BOOLEAN AS
$$
    DECLARE
        _strategy blackboard.strategy%ROWTYPE;
    BEGIN
        SELECT *
            INTO _strategy
            FROM blackboard.strategy;

        IF NOT FOUND THEN
            RETURN TRUE;
        ELSIF global THEN
            RETURN FALSE;
        ELSE
            RETURN
                (
                    SELECT COUNT(1)
                        FROM
                        (
                            SELECT COUNT(1)
                                FROM blackboard.result
                                WHERE node = inet_client_addr()
                                LIMIT 1
                        )
                        AS tmp
                ) = 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.command_queue_empty()
RETURNS BOOL AS
$$
    SELECT
        (
            SELECT COUNT(1)
                FROM
                (
                    SELECT 1
                        FROM blackboard.command
                        LIMIT 1
                )
                AS tmp
        ) = 0;
$$
LANGUAGE SQL;


-- (PRIVATE FUNCTION, DO NOT CALL FROM C++)
CREATE OR REPLACE FUNCTION blackboard.get_next_command_id()
RETURNS INTEGER AS
$$
    SELECT id
        FROM blackboard.command
        WHERE id NOT IN
            (
                SELECT command_id
                    FROM blackboard.result
                    WHERE node = inet_client_addr()
            )
        ORDER BY id
        LIMIT 1;
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.get_next_command()
RETURNS blackboard.command AS
$$
    SELECT *
        FROM blackboard.command
        WHERE id = blackboard.get_next_command_id();
$$
LANGUAGE SQL;


-- -------- --
-- STRATEGY --
-- -------- --
CREATE OR REPLACE FUNCTION blackboard.set_strategy
    (data_set TEXT,
    parmdb_local_sky TEXT,
    parmdb_instrument TEXT,
    parmdb_history TEXT,
    stations TEXT,
    input_data TEXT,
    region_of_interest_freq TEXT,
    region_of_interest_time TEXT,
    work_domain_size_freq DOUBLE PRECISION,
    work_domain_size_time DOUBLE PRECISION,
    correlation_time TEXT,
    correlation_size TEXT)
RETURNS BOOLEAN AS
$$
    BEGIN
        IF (SELECT COUNT(*) FROM blackboard.strategy) <> 0 THEN
            RETURN FALSE;
        END IF;

        INSERT
            INTO blackboard.strategy
                ("DataSet", 
                "ParmDB.LocalSky", 
                "ParmDB.Instrument", 
                "ParmDB.History", 
                "Stations", 
                "InputData", 
                "RegionOfInterest.Freq", 
                "RegionOfInterest.Time", 
                "WorkDomainSize.Freq", 
                "WorkDomainSize.Time", 
                "Correlation.Selection", 
                "Correlation.Type")
            VALUES
                (data_set,
                parmdb_local_sky,
                parmdb_instrument,
                parmdb_history,
                stations,
                input_data,
                region_of_interest_freq,
                region_of_interest_time,
                work_domain_size_freq,
                work_domain_size_time,
                correlation_time,
                correlation_size);

        RETURN TRUE;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_strategy()
RETURNS blackboard.strategy AS
$$
    SELECT *
        FROM blackboard.strategy;
$$
LANGUAGE SQL;


-- ------- --
-- COMMAND --
-- ------- --
-- (PRIVATE FUNCTION, DO NOT CALL FROM C++)
CREATE OR REPLACE FUNCTION blackboard.add_command(_type TEXT)
RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
    BEGIN
        _id := nextval('blackboard.command_id_seq');
        INSERT
            INTO blackboard.command(id, "Type")
            VALUES (_id, _type);

        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


-- (PRIVATE FUNCTION, DO NOT CALL FROM C++)
CREATE OR REPLACE FUNCTION blackboard.add_single_step_args
    (_command_id INTEGER,
    args anyelement)
RETURNS VOID AS
$$
    INSERT
        INTO blackboard.single_step_args
            (command_id,
            "Name",
            "Baselines.Station1",
            "Baselines.Station2",
            "Correlation.Selection",
            "Correlation.Type",
            "Sources",
            "InstrumentModel",
            "OutputData")
        VALUES
            ($1,
            $2."Name",
            $2."Baselines.Station1",
            $2."Baselines.Station2",
            $2."Correlation.Selection",
            $2."Correlation.Type",
            $2."Sources",
            $2."InstrumentModel",
            $2."OutputData");
$$
LANGUAGE SQL;


-- (PRIVATE FUNCTION, DO NOT CALL FROM C++)
CREATE OR REPLACE FUNCTION blackboard.get_single_step_args
    (_command_id INTEGER)
RETURNS blackboard.iface_single_step_args AS
$$
    SELECT
        "Name",
        "Baselines.Station1",
        "Baselines.Station2",
        "Correlation.Selection",
        "Correlation.Type",
        "Sources",
        "InstrumentModel",
        "OutputData"
        FROM blackboard.single_step_args
        WHERE command_id = $1;
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.add_predict_command
    (command_args blackboard.iface_predict_args)
RETURNS VOID AS
$$
    DECLARE
        _command_id INTEGER;
    BEGIN
        _command_id := blackboard.add_command('predict');
        PERFORM blackboard.add_single_step_args(_command_id, command_args);
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_predict_args(_command_id INTEGER)
RETURNS blackboard.iface_predict_args
AS
$$
    SELECT * FROM blackboard.get_single_step_args($1);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.add_subtract_command
    (command_args blackboard.iface_subtract_args)
RETURNS VOID AS
$$
    DECLARE
        _command_id INTEGER;
    BEGIN
        _command_id := blackboard.add_command('subtract');
        PERFORM blackboard.add_single_step_args(_command_id, command_args);
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_subtract_args(_command_id INTEGER)
RETURNS blackboard.iface_subtract_args
AS
$$
    SELECT * FROM blackboard.get_single_step_args($1);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.add_correct_command
    (command_args blackboard.iface_correct_args)
RETURNS VOID AS
$$
    DECLARE
        _command_id INTEGER;
    BEGIN
        _command_id := blackboard.add_command('correct');
        PERFORM blackboard.add_single_step_args(_command_id, command_args);
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_correct_args(_command_id INTEGER)
RETURNS blackboard.iface_correct_args
AS
$$
    SELECT * FROM blackboard.get_single_step_args($1);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.add_solve_command
    (command_args blackboard.iface_solve_args)
RETURNS VOID AS
$$
    DECLARE
        _command_id INTEGER;
    BEGIN
        _command_id := blackboard.add_command('solve');
        PERFORM blackboard.add_single_step_args(_command_id, command_args);
        INSERT
            INTO blackboard.solve_args
                (command_id,
                "MaxIter",
                "Epsilon",
                "MinConverged",
                "Parms",
                "ExclParms",
                "DomainSize.Freq",
                "DomainSize.Time")
            VALUES
                (_command_id,
                command_args."Solve.MaxIter",
                command_args."Solve.Epsilon",
                command_args."Solve.MinConverged",
                command_args."Solve.Parms",
                command_args."Solve.ExclParms",
                command_args."Solve.DomainSize.Freq",
                command_args."Solve.DomainSize.Time");
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_solve_args(_command_id INTEGER)
RETURNS blackboard.iface_solve_args AS
$$
    SELECT
            "Name",
            "Baselines.Station1",
            "Baselines.Station2",
            "Correlation.Selection",
            "Correlation.Type",
            "Sources",
            "InstrumentModel",
            "OutputData",
            "MaxIter",
            "Epsilon",
            "MinConverged",
            "Parms",
            "ExclParms",
            "DomainSize.Freq",
            "DomainSize.Time"
        FROM blackboard.single_step_args, blackboard.solve_args
        WHERE blackboard.single_step_args.command_id = $1
        AND blackboard.solve_args.command_id = $1;
$$
LANGUAGE SQL;


-- ------ --
-- RESULT --
-- ------ --
CREATE OR REPLACE FUNCTION blackboard.add_result
    (_command_id INTEGER,
    _result_code INTEGER,
    _message TEXT)
RETURNS VOID AS
$$
    DECLARE
        last_id INTEGER;
    BEGIN
        IF blackboard.command_queue_empty()
            OR _command_id != blackboard.get_next_command_id()
        THEN
            RAISE EXCEPTION
                'Attempt to add result for a command that is not current';
        END IF;

        INSERT INTO blackboard.result(command_id, result_code, message)
            VALUES (_command_id, _result_code, _message);
    END;
$$
LANGUAGE plpgsql;


-- --- --
-- LOG --
-- --- --
CREATE OR REPLACE FUNCTION blackboard.log
    (_command_id INTEGER,
    level INTEGER,
    pid INTEGER,
    scope TEXT,
    line_no INTEGER,
    message TEXT)
RETURNS VOID AS
$$
    INSERT INTO blackboard.log(command_id, level, pid, scope, line_no, message)
        VALUES ($1, $2, $3, $4, $5, $6);
$$
LANGUAGE SQL;
