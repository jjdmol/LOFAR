-- -------- --
-- STRATEGY --
-- -------- --
-- Function: blackboard.set_strategy
-- Full signature:
-- blackboard.set_strategy("DataSet" TEXT, "ParmDB.LocalSky" TEXT, "ParmDB.Instrument" TEXT, "ParmDB.History" TEXT, "Stations" TEXT[], "InputData" TEXT, "WorkDomainSize.Freq" DOUBLE PRECISION, "WorkDomainSize.Time" DOUBLE PRECISION, "Correlation.Selection" TEXT, "Correlation.Type" TEXT[])
CREATE OR REPLACE FUNCTION blackboard.set_strategy(TEXT, TEXT, TEXT, TEXT, TEXT[], TEXT, DOUBLE PRECISION, DOUBLE PRECISION, TEXT, TEXT[])
RETURNS VOID AS
$$
    INSERT INTO blackboard.strategy("DataSet", "ParmDB.LocalSky", "ParmDB.Instrument", "ParmDB.History", "Stations", "InputData", "WorkDomainSize.Freq", "WorkDomainSize.Time", "Correlation.Selection", "Correlation.Type")
        VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.get_strategy()
RETURNS blackboard.strategy AS
$$
    SELECT *
        FROM blackboard.strategy
        LIMIT 1;
$$
LANGUAGE SQL;


-- ---------- --
-- WORK ORDER --
-- ---------- --
CREATE OR REPLACE FUNCTION blackboard.add_work_order()
RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
    BEGIN
        _id := nextval('blackboard.work_order_id_seq');
        INSERT INTO blackboard.work_order(id)
            VALUES (_id);

        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_next_work_order(current_work_order_id INTEGER)
RETURNS blackboard.work_order AS
$$
    SELECT *
        FROM blackboard.work_order
        WHERE id > $1
        ORDER BY id LIMIT 1;
$$
LANGUAGE SQL;


-- ---- --
-- STEP --
-- ---- --
-- Function: blackboard.get_step
-- Full signature:
-- blackboard.get_step(_work_order_id INTEGER)
CREATE OR REPLACE FUNCTION blackboard.get_step(INTEGER)
RETURNS blackboard.step AS
$$
    
    SELECT *
        FROM blackboard.step
        WHERE blackboard.step.work_order_id = $1;
$$
LANGUAGE SQL;


-- Function: blackboard.add_step
-- Full signature:
-- blackboard.add_step("Name" TEXT, "Operation" TEXT, "Baselines.Station1" TEXT[], "Baselines.Station2" TEXT[], "Correlation.Selection" TEXT, "Correlation.Type" TEXT[], "Sources" TEXT[], "InstrumentModel" TEXT[], "OutputData" TEXT)
CREATE OR REPLACE FUNCTION blackboard.add_step(TEXT, TEXT, TEXT[], TEXT[], TEXT, TEXT[], TEXT[], TEXT[], TEXT)
RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
        _work_order_id INTEGER;
    BEGIN
        _work_order_id := blackboard.add_work_order();
        _id := nextval('blackboard.step_id_seq');
        INSERT INTO blackboard.step(id, work_order_id, "Name", "Operation", "Baselines.Station1", "Baselines.Station2", "Correlation.Selection", "Correlation.Type", "Sources", "InstrumentModel", "OutputData")
            VALUES (_id, _work_order_id, $1, $2, $3, $4, $5, $6, $7, $8, $9);

        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


-- Function: blackboard.get_solve_arguments
-- Full signature:
-- blackboard.add_solve_arguments(_step_id INTEGER)
CREATE OR REPLACE FUNCTION blackboard.get_solve_arguments(INTEGER)
RETURNS blackboard.solve_arguments AS
$$
    DECLARE
        step blackboard.step%ROWTYPE;
        arguments blackboard.solve_arguments%ROWTYPE;
    BEGIN
        SELECT * INTO step
            FROM blackboard.step
            WHERE id = $1;

        IF NOT FOUND OR step."Operation" != 'SOLVE' THEN
            RAISE EXCEPTION 'Step % either does not exist or is not a solve step.', $1;
        END IF;

        SELECT * INTO arguments
            FROM blackboard.solve_arguments
            WHERE step_id = $1;

        RETURN arguments;
    END;
$$
LANGUAGE plpgsql;


-- Function: blackboard.add_solve_arguments
-- Full signature:
-- blackboard.add_solve_arguments(step_id INTEGER, "MaxIter" INTEGER, "Epsilon" DOUBLE PRECISION, "MinConverged" DOUBLE PRECISION, "Parms" TEXT[], "ExclParms" TEXT[], "DomainSize.Freq" DOUBLE PRECISION, "DomainSize.Time" DOUBLE PRECISION)
CREATE OR REPLACE FUNCTION blackboard.add_solve_arguments(INTEGER, INTEGER, DOUBLE PRECISION, DOUBLE PRECISION, TEXT[], TEXT[], DOUBLE PRECISION, DOUBLE PRECISION)
RETURNS VOID AS
$$
    INSERT INTO blackboard.solve_arguments(step_id, "MaxIter", "Epsilon", "MinConverged", "Parms", "ExclParms", "DomainSize.Freq", "DomainSize.Time")
        VALUES ($1, $2, $3, $4, $5, $6, $7, $8);
$$
LANGUAGE SQL;



-- ----------------- --
-- WORK ORDER STATUS --
-- ----------------- --
CREATE OR REPLACE FUNCTION blackboard.set_status(work_order_id INTEGER, status_code INTEGER, message TEXT)
RETURNS VOID AS
$$
    INSERT INTO blackboard.status(work_order_id, status_code, status_message)
        VALUES ($1, $2, $3);
$$
LANGUAGE SQL;


-- --- --
-- LOG --
-- --- --
CREATE OR REPLACE FUNCTION blackboard.log(work_order_id INTEGER, level INTEGER, pid INTEGER, scope TEXT, line_no INTEGER, message TEXT)
RETURNS VOID AS
$$
    INSERT INTO blackboard.log(work_order_id, level, pid, scope, line_no, message)
        VALUES ($1, $2, $3, $4, $5, $6);
$$
LANGUAGE SQL;
