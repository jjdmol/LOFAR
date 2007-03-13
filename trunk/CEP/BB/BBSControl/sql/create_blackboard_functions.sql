-- -------- --
-- STRATEGY --
-- -------- --
-- Function: blackboard.set_strategy
-- Full signature:
-- blackboard.set_strategy("DataSet" TEXT, "ParmDB.LocalSky" TEXT, "ParmDB.Instrument" TEXT, "ParmDB.History" TEXT, "Stations" TEXT, "InputData" TEXT, "WorkDomainSize.Freq" DOUBLE PRECISION, "WorkDomainSize.Time" DOUBLE PRECISION, "Correlation.Selection" TEXT, "Correlation.Type" TEXT)
CREATE OR REPLACE FUNCTION blackboard.set_strategy(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, DOUBLE PRECISION, DOUBLE PRECISION, TEXT, TEXT)
RETURNS BOOLEAN AS
$$
    BEGIN
        IF (SELECT COUNT(*) FROM blackboard.strategy) <> 0 THEN
            RETURN FALSE;
        END IF;

        INSERT INTO blackboard.strategy(
            "DataSet", 
            "ParmDB.LocalSky", 
            "ParmDB.Instrument", 
            "ParmDB.History", 
            "Stations", 
            "InputData", 
            "WorkDomainSize.Freq", 
            "WorkDomainSize.Time", 
            "Correlation.Selection", 
            "Correlation.Type")
        VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10);

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


CREATE OR REPLACE FUNCTION blackboard.is_clean_startup(_node INET)
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
        ELSIF _node IS NULL THEN
            RETURN FALSE;
        ELSE
            RETURN (SELECT COUNT(*) FROM blackboard.result WHERE node = _node) = 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;


-- ---------- --
-- WORK ORDER --
-- ---------- --
CREATE OR REPLACE FUNCTION blackboard.add_command()
RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
    BEGIN
        _id := nextval('blackboard.command_id_seq');
        INSERT
            INTO blackboard.command(id)
            VALUES (_id);

        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_next_command(current_command_id INTEGER)
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


-- Function: blackboard.get_next_step
-- Full signature:
-- blackboard.get_next_step(_current_command_id INTEGER)
--
-- This function is a 'combination' of get_next_command and
-- get_next_step. Therefore, it has to return a combination of
-- a command_id and an iface_step. We could define a separate
-- iface composite type for this, but then we would have two
-- interface definitions for a step. Instead, we just return a
-- row from the step table. Of course, this is not very clean.
-- Therefore, it is recommended to use get_next_command and
-- get_step sequentially.
CREATE OR REPLACE FUNCTION blackboard.get_next_step(INTEGER)
RETURNS blackboard.step AS
$$
    SELECT blackboard.step.*
        FROM blackboard.command, blackboard.step
        WHERE blackboard.command.id > $1
        AND blackboard.command.id = blackboard.step.command_id
        ORDER BY blackboard.command.id;
$$
LANGUAGE SQL;


-- Function: blackboard.get_step
-- Full signature:
-- blackboard.get_step(_command_id INTEGER)
CREATE OR REPLACE FUNCTION blackboard.get_step(INTEGER)
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


-- Function: blackboard.get_solve_arguments
-- Full signature:
-- blackboard.get_solve_arguments(_command_id INTEGER)
CREATE OR REPLACE FUNCTION blackboard.get_solve_arguments(INTEGER)
RETURNS blackboard.iface_solve_arguments AS
$$
    DECLARE
        step blackboard.step%ROWTYPE;
        arguments blackboard.iface_solve_arguments;
    BEGIN
        SELECT *
            INTO step
            FROM blackboard.step
            WHERE command_id = $1;

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


-- Function: blackboard.add_step (PRIVATE FUNCTION, DO NOT CALL FROM C++)
-- Full signature:
-- blackboard.add_step("Name" TEXT, "Operation" TEXT, "Baselines.Station1" TEXT, "Baselines.Station2" TEXT, "Correlation.Selection" TEXT, "Correlation.Type" TEXT, "Sources" TEXT, "InstrumentModel" TEXT, "OutputData" TEXT)
CREATE OR REPLACE FUNCTION blackboard.add_step(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT)
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
            VALUES (_id, _command_id, $1, $2, $3, $4, $5, $6, $7, $8, $9);
                
        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


-- Function: blackboard.add_predict_step
-- Full signature:
-- blackboard.add_predict_step("Name" TEXT, "Baselines.Station1" TEXT, "Baselines.Station2" TEXT, "Correlation.Selection" TEXT, "Correlation.Type" TEXT, "Sources" TEXT, "InstrumentModel" TEXT, "OutputData")
CREATE OR REPLACE FUNCTION blackboard.add_predict_step(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT)
RETURNS INTEGER AS
$$
    SELECT blackboard.add_step($1, 'PREDICT', $2, $3, $4, $5, $6, $7, $8);
$$
LANGUAGE SQL;


-- Function: blackboard.add_subtract_step
-- Full signature:
-- blackboard.add_subtract_step("Name" TEXT, "Baselines.Station1" TEXT, "Baselines.Station2" TEXT, "Correlation.Selection" TEXT, "Correlation.Type" TEXT, "Sources" TEXT, "InstrumentModel" TEXT, "OutputData")
CREATE OR REPLACE FUNCTION blackboard.add_subtract_step(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT)
RETURNS INTEGER AS
$$
    SELECT blackboard.add_step($1, 'SUBTRACT', $2, $3, $4, $5, $6, $7, $8);
$$
LANGUAGE SQL;


-- Function: blackboard.add_correct_step
-- Full signature:
-- blackboard.add_correct_step("Name" TEXT, "Baselines.Station1" TEXT, "Baselines.Station2" TEXT, "Correlation.Selection" TEXT, "Correlation.Type" TEXT, "Sources" TEXT, "InstrumentModel" TEXT, "OutputData")
CREATE OR REPLACE FUNCTION blackboard.add_correct_step(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT)
RETURNS INTEGER AS
$$
    SELECT blackboard.add_step($1, 'CORRECT', $2, $3, $4, $5, $6, $7, $8);
$$
LANGUAGE SQL;


-- Function: blackboard.add_solve_step
-- Full signature:
-- blackboard.add_solve_step("Name" TEXT, "Baselines.Station1" TEXT, "Baselines.Station2" TEXT, "Correlation.Selection" TEXT, "Correlation.Type" TEXT, "Sources" TEXT, "InstrumentModel" TEXT, "OutputData" TEXT, "MaxIter" INTEGER, "Epsilon" DOUBLE PRECISION, "MinConverged" DOUBLE PRECISION, "Parms" TEXT, "ExclParms" TEXT, "DomainSize.Freq" DOUBLE PRECISION, "DomainSize.Time" DOUBLE PRECISION)
CREATE OR REPLACE FUNCTION blackboard.add_solve_step(TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, INTEGER, DOUBLE PRECISION, DOUBLE PRECISION, TEXT, TEXT, DOUBLE PRECISION, DOUBLE PRECISION)
RETURNS VOID AS
$$
    DECLARE
        _step_id INTEGER;
    BEGIN
        _step_id := blackboard.add_step($1, 'SOLVE', $2, $3, $4, $5, $6, $7, $8);
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
            VALUES (_step_id, $9, $10, $11, $12, $13, $14, $15);
    END;
$$
LANGUAGE plpgsql;


-- Function: blackboard.add_solve_arguments
-- Full signature:
-- blackboard.add_solve_arguments(_step_id INTEGER, "MaxIter" INTEGER, "Epsilon" DOUBLE PRECISION, "MinConverged" DOUBLE PRECISION, "Parms" TEXT, "ExclParms" TEXT, "DomainSize.Freq" DOUBLE PRECISION, "DomainSize.Time" DOUBLE PRECISION)
--CREATE OR REPLACE FUNCTION blackboard.add_solve_arguments(INTEGER, INTEGER, DOUBLE PRECISION, DOUBLE PRECISION, TEXT, TEXT, DOUBLE PRECISION, DOUBLE PRECISION)
--RETURNS VOID AS
--$$
--    INSERT INTO blackboard.solve_arguments(step_id, "MaxIter", "Epsilon", "MinConverged", "Parms", "ExclParms", "DomainSize.Freq", "DomainSize.Time")
--        VALUES ($1, $2, $3, $4, $5, $6, $7, $8);
--$$
--LANGUAGE SQL;


-- ----------------- --
-- WORK ORDER STATUS --
-- ----------------- --
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
