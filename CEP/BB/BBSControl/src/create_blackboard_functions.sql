-- -------- --
-- STRATEGY --
-- -------- --
CREATE OR REPLACE FUNCTION blackboard.set_strategy(data_set TEXT, local_sky_parmdb TEXT, instrument_parmdb TEXT, history_parmdb TEXT, stations TEXT[], input_column TEXT, work_domain_size DOUBLE PRECISION[2], correlation_selection TEXT, correlation_type TEXT[]) RETURNS VOID AS
$$
    INSERT INTO blackboard.strategy(data_set, local_sky_parmdb, instrument_parmdb, history_parmdb, stations, input_column, work_domain_size, correlation_selection, correlation_type) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9);
$$
LANGUAGE SQL;


CREATE OR REPLACE FUNCTION blackboard.get_strategy() RETURNS blackboard.strategy AS
$$
    SELECT * FROM blackboard.strategy LIMIT 1;
$$
LANGUAGE SQL;

-- ---------- --
-- WORK ORDER --
-- ---------- --
CREATE OR REPLACE FUNCTION blackboard.add_work_order() RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
    BEGIN
        _id := nextval('blackboard.work_order_id_seq');
        INSERT INTO blackboard.work_order(id) VALUES (_id);
        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.get_next_work_order(current_work_order_id INTEGER) RETURNS blackboard.work_order AS
$$
    SELECT * FROM blackboard.work_order WHERE id > $1 ORDER BY id LIMIT 1;
$$
LANGUAGE SQL;

-- ---- --
-- STEP --
-- ---- --
CREATE OR REPLACE FUNCTION blackboard.add_step(_name TEXT, _operation TEXT, _station1 TEXT[], _station2 TEXT[], _correlation_selection TEXT, _correlation_type TEXT[], _sources TEXT[], _instrument_model TEXT[], _output_column TEXT) RETURNS INTEGER AS
$$
    DECLARE
        _id INTEGER;
        _work_order_id INTEGER;
    BEGIN
        _work_order_id := add_work_order();
        _id := nextval('blackboard.step_id_seq');
        INSERT INTO blackboard.step(id, work_order_id, name, operation, station1, station2, correlation_selection, correlation_type, sources, instrument_model, output_column) VALUES (_id, _work_order_id, _name, _operation, _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model, _output_column);
        RETURN _id;
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.add_solve_step(_name TEXT, _station1 TEXT[], _station2 TEXT[], _correlation_selection TEXT, _correlation_type TEXT[], _sources TEXT[], _instrument_model TEXT[], _output_column TEXT, _max_iter INTEGER, _epsilon DOUBLE PRECISION, _min_converged DOUBLE PRECISION, _parms TEXT[], _excl_parms TEXT[], _solve_domain_size DOUBLE PRECISION[2]) RETURNS VOID AS
$$
    DECLARE
        _step_id INTEGER;
    BEGIN
        _step_id := add_step(_name, 'SOLVE', _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model, _output_column);
        INSERT INTO blackboard.solve_arguments(step_id, max_iter, epsilon, min_converged, parms, excl_parms, solve_domain_size) VALUES (_step_id, _max_iter, _epsilon, _min_converged, _parms, _excl_parms, _solve_domain_size);
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.add_predict_step(_name TEXT, _station1 TEXT[], _station2 TEXT[], _correlation_selection TEXT, _correlation_type TEXT[], _sources TEXT[], _instrument_model TEXT[], _output_column TEXT) RETURNS VOID AS
$$
--    DECLARE
--        _step_id INTEGER;
    BEGIN
--        _step_id := add_step(_name, 'PREDICT', _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model);
--        INSERT INTO blackboard.predict_arguments(step_id, output_column) VALUES (_step_id, _output_column);
        PERFORM add_step(_name, 'PREDICT', _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model, _output_column);
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.add_subtract_step(_name TEXT, _station1 TEXT[], _station2 TEXT[], _correlation_selection TEXT, _correlation_type TEXT[], _sources TEXT[], _instrument_model TEXT[], _output_column TEXT) RETURNS VOID AS
$$
--    DECLARE
--        _step_id INTEGER;
    BEGIN
--        _step_id := add_step(_name, 'SUBTRACT', _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model);
--        INSERT INTO blackboard.subtract_arguments(step_id, output_column) VALUES (_step_id, _output_column);
        PERFORM add_step(_name, 'SUBTRACT', _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model, _output_column);
    END;
$$
LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION blackboard.add_correct_step(_name TEXT, _station1 TEXT[], _station2 TEXT[], _correlation_selection TEXT, _correlation_type TEXT[], _sources TEXT[], _instrument_model TEXT[], _output_column TEXT) RETURNS VOID AS
$$
--    DECLARE
--        _step_id INTEGER;
    BEGIN
--        _step_id := add_step(_name, 'CORRECT', _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model);
--        INSERT INTO blackboard.correct_arguments(step_id, output_column) VALUES (_step_id, _output_column);
        PERFORM add_step(_name, 'CORRECT', _station1, _station2, _correlation_selection, _correlation_type, _sources, _instrument_model, _output_column);
    END;
$$
LANGUAGE plpgsql;

-- ----------------- --
-- WORK ORDER STATUS --
-- ----------------- --
CREATE OR REPLACE FUNCTION blackboard.set_status(work_order_id INTEGER, status_code INTEGER, message TEXT) RETURNS VOID AS
$$
    INSERT INTO blackboard.status(work_order_id, status_code, status_message) VALUES ($1, $2, $3);
$$
LANGUAGE SQL;

-- --- --
-- LOG --
-- --- --
CREATE OR REPLACE FUNCTION blackboard.log(work_order_id INTEGER, level INTEGER, pid INTEGER, scope TEXT, line_no INTEGER, message TEXT) RETURNS VOID AS
$$
    INSERT INTO blackboard.log(work_order_id, level, pid, scope, line_no, message) VALUES ($1, $2, $3, $4, $5, $6);
$$
LANGUAGE SQL;
