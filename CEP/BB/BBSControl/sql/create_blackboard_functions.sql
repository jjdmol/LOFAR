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
            RETURN (SELECT COUNT(*) FROM blackboard.result WHERE node = inet_client_addr()) = 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;


-- ------- --
-- COMMAND --
-- ------- --
CREATE OR REPLACE FUNCTION blackboard.add_command()
RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
    BEGIN
        _id := nextval('blackboard.command_id_seq');
        INSERT
            INTO blackboard.command(id, "Type")
            VALUES (_id, 'EXEC_STEP');

        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_next_command(_current_command_id INTEGER)
RETURNS blackboard.command AS
$$
    SELECT *
        FROM blackboard.command
        WHERE id > $1
        ORDER BY id;
$$
LANGUAGE SQL;


-- ---- --
-- STEP --
-- ---- --
CREATE TYPE blackboard.iface_step AS
(
    "Name"                  TEXT,
    "Operation"             TEXT,
    "Baselines.Station1"    TEXT,
    "Baselines.Station2"    TEXT,
    "Correlation.Selection" TEXT,
    "Correlation.Type"      TEXT,
    "Sources"               TEXT,
    "InstrumentModel"       TEXT,
    "OutputData"            TEXT
);


CREATE TYPE blackboard.iface_solve_arguments AS
(
    "MaxIter"               INTEGER,
    "Epsilon"               DOUBLE PRECISION,
    "MinConverged"          DOUBLE PRECISION,
    "Parms"                 TEXT,
    "ExclParms"             TEXT,
    "DomainSize.Freq"       DOUBLE PRECISION,
    "DomainSize.Time"       DOUBLE PRECISION
);


-- This function is a 'combination' of get_next_command and
-- get_next_step. Therefore, it has to return a combination of
-- a command_id and an iface_step. We could define a separate
-- iface composite type for this, but then we would have two
-- interface definitions for a step. Instead, we just return a
-- row from the step table. Of course, this is not very clean.
-- Therefore, it is recommended to use get_next_command and
-- get_step sequentially.
CREATE OR REPLACE FUNCTION blackboard.get_next_step(_current_command_id INTEGER)
RETURNS blackboard.step AS
$$
    SELECT blackboard.step.*
        FROM blackboard.command, blackboard.step
        WHERE blackboard.command.id > $1
        AND blackboard.command.id = blackboard.step.command_id
        ORDER BY blackboard.command.id;
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.get_step(_command_id INTEGER)
RETURNS blackboard.iface_step AS
$$
    SELECT
        "Name",
        "Operation",
        "Baselines.Station1",
        "Baselines.Station2",
        "Correlation.Selection",
        "Correlation.Type",
        "Sources",
        "InstrumentModel",
        "OutputData"
        FROM blackboard.step
        WHERE blackboard.step.command_id = $1; 
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.get_solve_arguments(_command_id INTEGER)
RETURNS blackboard.iface_solve_arguments AS
$$
    DECLARE
        step blackboard.step%ROWTYPE;
        arguments blackboard.iface_solve_arguments;
    BEGIN
        SELECT *
            INTO step
            FROM blackboard.step
            WHERE command_id = _command_id;

        IF NOT FOUND OR step."Operation" != 'SOLVE' THEN
            RAISE EXCEPTION 'Work order % either is not a solve step or it is not a step at all.', $1;
        END IF;

        SELECT
            "MaxIter",
            "Epsilon",
            "MinConverged",
            "Parms",
            "ExclParms",
            "DomainSize.Freq",
            "DomainSize.Time"
            INTO arguments
            FROM blackboard.solve_arguments
            WHERE step_id = step.id;

        RETURN arguments;
    END;
$$
LANGUAGE plpgsql;


-- (PRIVATE FUNCTION, DO NOT CALL FROM C++)
CREATE OR REPLACE FUNCTION blackboard.add_step
    (name TEXT,
    operation TEXT,
    baselines_station1 TEXT,
    baselines_station2 TEXT,
    correlation_selection TEXT,
    correlation_type TEXT,
    sources TEXT,
    instrument_model TEXT,
    output_data TEXT)
RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
        _command_id INTEGER;
    BEGIN
        _command_id := blackboard.add_command();
        _id := nextval('blackboard.step_id_seq');
        
         INSERT
            INTO blackboard.step
                (id,
                command_id,
                "Name",
                "Operation",
                "Baselines.Station1",
                "Baselines.Station2",
                "Correlation.Selection",
                "Correlation.Type",
                "Sources",
                "InstrumentModel",
                "OutputData")
            VALUES
                (_id,
                _command_id,
                name,
                operation,
                baselines_station1,
                baselines_station2,
                correlation_selection,
                correlation_type,
                sources,
                instrument_model,
                output_data);
                
        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.add_predict_step
    (name TEXT,
    baselines_station1 TEXT,
    baselines_station2 TEXT,
    correlation_selection TEXT,
    correlation_type TEXT,
    sources TEXT,
    instrument_model TEXT,
    output_data TEXT)
RETURNS INTEGER AS
$$
    SELECT blackboard.add_step($1, 'PREDICT', $2, $3, $4, $5, $6, $7, $8);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.add_subtract_step
    (name TEXT,
    baselines_station1 TEXT,
    baselines_station2 TEXT,
    correlation_selection TEXT,
    correlation_type TEXT,
    sources TEXT,
    instrument_model TEXT,
    output_data TEXT)
RETURNS INTEGER AS
$$
    SELECT blackboard.add_step($1, 'SUBTRACT', $2, $3, $4, $5, $6, $7, $8);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.add_correct_step
    (name TEXT,
    baselines_station1 TEXT,
    baselines_station2 TEXT,
    correlation_selection TEXT,
    correlation_type TEXT,
    sources TEXT,
    instrument_model TEXT,
    output_data TEXT)
RETURNS INTEGER AS
$$
    SELECT blackboard.add_step($1, 'CORRECT', $2, $3, $4, $5, $6, $7, $8);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.add_solve_step
    (name TEXT,
    baselines_station1 TEXT,
    baselines_station2 TEXT,
    correlation_selection TEXT,
    correlation_type TEXT,
    sources TEXT,
    instrument_model TEXT,
    output_data TEXT,
    max_iter INTEGER,
    epsilon DOUBLE PRECISION,
    min_converged DOUBLE PRECISION,
    parms TEXT,
    excl_parms TEXT,
    domain_size_freq DOUBLE PRECISION,
    domain_size_time DOUBLE PRECISION)
RETURNS INTEGER AS
$$
    DECLARE
        _step_id INTEGER;
    BEGIN
        _step_id := blackboard.add_step
            (name,
            'SOLVE',
            baselines_station1,
            baselines_station2,
            correlation_selection,
            correlation_type,
            sources,
            instrument_model,
            output_data);
        
        INSERT
            INTO blackboard.solve_arguments
                (step_id,
                "MaxIter",
                "Epsilon",
                "MinConverged",
                "Parms",
                "ExclParms",
                "DomainSize.Freq",
                "DomainSize.Time")
            VALUES
                (_step_id,
                max_iter,
                epsilon,
                min_converged,
                parms,
                excl_parms,
                domain_size_freq,
                domain_size_time);

        RETURN _step_id;
    END;
$$
LANGUAGE plpgsql;


-- -------------- --
-- COMMAND RESULT --
-- -------------- --
CREATE OR REPLACE FUNCTION blackboard.set_result(command_id INTEGER, result_code INTEGER, message TEXT)
RETURNS VOID AS
$$
    INSERT INTO blackboard.result(command_id, result_code, message)
        VALUES ($1, $2, $3);
$$
LANGUAGE SQL;


-- --- --
-- LOG --
-- --- --
CREATE OR REPLACE FUNCTION blackboard.log(command_id INTEGER, level INTEGER, pid INTEGER, scope TEXT, line_no INTEGER, message TEXT)
RETURNS VOID AS
$$
    INSERT INTO blackboard.log(command_id, level, pid, scope, line_no, message)
        VALUES ($1, $2, $3, $4, $5, $6);
$$
LANGUAGE SQL;
