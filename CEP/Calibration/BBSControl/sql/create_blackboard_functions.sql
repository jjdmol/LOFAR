----------------
-- RUN STATES --
----------------

-- -1 - FAILED
--  0 - WAITING_FOR_CONTROL
--  1 - WAITING_FOR_WORKERS
--  2 - COMPUTING_WORKER_INDEX
--  3 - PROCESSING
--  4 - DONE


------------------
-- SHARED STATE --
------------------

CREATE OR REPLACE FUNCTION blackboard.init_shared_state(_key TEXT,
    OUT _id INTEGER) AS
$$
    BEGIN
        BEGIN
            INSERT INTO blackboard.shared_state (key)
                VALUES      (_key);
        EXCEPTION
            WHEN unique_violation THEN
                -- NOT considered an error.
                NULL;
        END;

        -- Should always succeed.
        SELECT id
            INTO STRICT _id
            FROM        blackboard.shared_state
            WHERE       key = _key;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.set_run_state(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, _run_state INTEGER, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Test if caller is the control process and set the run_state in a
        -- single query. Also, set the run_finish field.
        UPDATE blackboard.shared_state
            SET     run_state = _run_state
            WHERE   id = _state_id
            AND     control_hostname = _hostname
            AND     control_pid = _pid;

        IF FOUND AND (_run_state = -1 OR _run_state = 4) THEN
            UPDATE blackboard.shared_state
                SET     run_finish = now()
                WHERE   id = _state_id;
        END IF;            
                
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_run_state(_state_id INTEGER,
    OUT _status INTEGER, OUT _run_state INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        SELECT run_state
            INTO    _run_state
            FROM    blackboard.shared_state
            WHERE   id = _state_id;
        
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;


--------------
-- REGISTER --
--------------

CREATE OR REPLACE FUNCTION blackboard.create_kernel_slot(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, _filesystem TEXT, _path TEXT)
RETURNS VOID AS
$$
    BEGIN
        -- Verify that the caller is the control process and that the run_state
        -- is set to WAITING_FOR_CONTROL. Row lock the shared_state to prevent
        -- concurrent updates.
        IF NOT blackboard.test_and_lock_shared_state(_state_id, _hostname, _pid,
            0) THEN
            RAISE EXCEPTION 'Incorrect run state or operation not permitted';
        END IF;

        INSERT INTO blackboard.worker_register (state_id, worker_type,
            filesystem, path)
            VALUES  (_state_id, 'K', _filesystem, _path);
    END;
$$    
LANGUAGE plpgsql;
    
CREATE OR REPLACE FUNCTION blackboard.create_solver_slot(_state_id INTEGER,
    _hostname TEXT, _pid INTEGER)
RETURNS VOID AS
$$
    BEGIN
        -- Verify that the caller is the control process and that the run_state
        -- is set to WAITING_FOR_CONTROL. Row lock the shared_state to prevent
        -- concurrent updates.
        IF NOT blackboard.test_and_lock_shared_state(_state_id, _hostname, _pid,
            0) THEN
            RAISE EXCEPTION 'Incorrect run state or operation not permitted';
        END IF;

        INSERT INTO blackboard.worker_register (state_id, worker_type)
            VALUES  (_state_id, 'S');
    END;
$$    
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.register_as_control(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        -- Verify that run_state is set to WAITING_FOR_CONTROL and try to
        -- register in a single query.
        UPDATE blackboard.shared_state
            SET     control_hostname = _hostname, control_pid = _pid
            WHERE   id = _state_id
            AND     run_state = 0
            AND     control_hostname IS NULL
            AND     control_pid IS NULL;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$    
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.register_as_kernel(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, _filesystem TEXT, _path TEXT,
    _axis_freq_lower BYTEA, _axis_freq_upper BYTEA, _axis_time_lower BYTEA,
    _axis_time_upper BYTEA, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Check if the run_state is set to WAITING_FOR_WORKERS and row lock the
        -- shared_state to prevent concurrent updates.
        IF NOT blackboard.test_and_lock_shared_state(_state_id, 1) THEN
            RETURN;
        END IF;

        -- Try to register.
        UPDATE blackboard.worker_register
            SET     hostname = _hostname, pid = _pid,
                    axis_freq_lower = _axis_freq_lower,
                    axis_freq_upper = _axis_freq_upper,
                    axis_time_lower = _axis_time_lower,
                    axis_time_upper = _axis_time_upper
            WHERE   state_id = _state_id
            AND     worker_type = 'K'
            AND     hostname IS NULL
            AND     pid IS NULL
            AND     filesystem = _filesystem
            AND     path = _path;
        
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.register_as_solver(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, _port INTEGER, OUT _status INTEGER) AS
$$
    DECLARE
        _slot_id    INTEGER;
    BEGIN
        _status := -1;

        -- Check if the run_state is set to WAITING_FOR_WORKERS and row lock the
        -- shared_state to prevent concurrent updates.
        IF NOT blackboard.test_and_lock_shared_state(_state_id, 1) THEN
            RETURN;
        END IF;

        -- Row lock all remaining free slots FOR UPDATE in order of increasing
        -- id and return the free slot with the lowest id.
        -- TODO: Does the ORDER BY clause affect the order in which the rows
        -- are locked? If not, the following query may create a deadlock.
        SELECT id
            INTO        _slot_id
            FROM        blackboard.worker_register
            WHERE       state_id = _state_id
            AND         worker_type = 'S'
            AND         hostname IS NULL
            AND         pid IS NULL
            ORDER BY    id
            FOR UPDATE;

        IF NOT FOUND THEN
            RETURN;
        END IF;
            
        -- Try to register.
        UPDATE blackboard.worker_register
            SET     hostname = _hostname, pid = _pid, port = _port
            WHERE   id = _slot_id;
            
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_register(_state_id INTEGER)
RETURNS SETOF blackboard.worker_register AS
$$
    SELECT *
        FROM        blackboard.worker_register
        WHERE       state_id = $1
        ORDER BY    id;
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION blackboard.set_index(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, _target_hostname TEXT, _target_pid BIGINT,
    _index INTEGER, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        -- Verify that the caller is the control process and that the run_state
        -- is set to COMPUTING_WORKER_INDEX. Row lock the shared_state to
        -- prevent concurrent updates.
        IF NOT blackboard.test_and_lock_shared_state(_state_id, _hostname, _pid,
            2) THEN
            RETURN;
        END IF;
        
        UPDATE blackboard.worker_register
            SET     index = _index
            WHERE   state_id = _state_id
            AND     hostname = _target_hostname
            AND     pid = _target_pid;
            
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;


-------------
-- CONTROL --
-------------

-- (PRIVATE FUNCTION, DO NOT CALL FROM C++)
CREATE OR REPLACE FUNCTION blackboard.get_active_command_id(_state_id INTEGER,
    _worker_id INTEGER, OUT _id INTEGER) AS
$$
    BEGIN
        -- Note that STRICT is not used with INTO in the query below because it
        -- can return zero rows, which is ok and should not raise an exception.
        -- Without the STRICT modifier, returning multiple rows will not raise
        -- an exception, even though for this query it should be considered an
        -- error. However, this is already covered by the UNIQUE column
        -- constraint on blackboard.command.id.
        --
        -- PRECONDITION: _worker_id refers to a worker that is registered
        -- to the shared_state with id _state_id.
        SELECT command.id
            INTO        _id
            FROM        blackboard.command AS command,
                        blackboard.worker_register AS worker_register
            WHERE       command.state_id = _state_id
            AND         worker_register.id = _worker_id
            AND         (command.target IS NULL
                        OR worker_register.worker_type = command.target)
            AND         command.id >
                (
                    SELECT COALESCE(MAX(command_id), -1)
                        FROM    blackboard.result
                        WHERE   worker_id = _worker_id
                )
            ORDER BY    id
            LIMIT       1;
    END;                
$$    
LANGUAGE plpgsql;

-- NOTE: This function has an extra status code!
-- -2 - Queue empty.
CREATE OR REPLACE FUNCTION blackboard.get_command(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, OUT _status INTEGER, OUT _id INTEGER,
    OUT _type TEXT, OUT _name TEXT, OUT _args TEXT) AS
$$
    DECLARE
        _worker_id INTEGER;
    BEGIN
        _status := -1;

        -- Get the worker_id of the calling process. Because there is no stored
        -- procedure that can change the worker_id assigned to a process after
        -- it has registered, concurrent transactions are harmless. Therefore,
        -- the register does not need to be locked.
        SELECT id
            INTO    _worker_id
            FROM    blackboard.worker_register
            WHERE   state_id = _state_id
            AND     hostname = _hostname
            AND     pid = _pid;
        
        IF NOT FOUND THEN
            RETURN;
        END IF;

        -- Get the id of the active command for this process. It is assumed that
        -- a process does not interfere with itself, e.g. it will not call
        -- get_active_command() and add_result() concurrently. If it would, the
        -- active command might change between fetching the id of the active
        -- command and fetching the details of that command using the SELECT
        -- query below.
        _id := blackboard.get_active_command_id(_state_id, _worker_id);
        IF _id IS NULL THEN
            -- Queue is empty, so return an empty command.
            _status := -2;
            RETURN;
        END IF;

        SELECT type, name, args
            INTO STRICT _type, _name, _args
            FROM        blackboard.command
            WHERE       id = _id;
            
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$    
LANGUAGE plpgsql;


-------------
-- COMMAND --
-------------

CREATE OR REPLACE FUNCTION blackboard.add_command(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, _target CHARACTER, _type TEXT, _name TEXT, _args TEXT,
    OUT _status INTEGER, OUT _id INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        -- Verify that the caller is the control process and that the run_state
        -- is set to PROCESSING. Row lock the shared_state to prevent concurrent
        -- updates.
        IF NOT blackboard.test_and_lock_shared_state(_state_id, _hostname, _pid,
            3) THEN
            RETURN;
        END IF;

        INSERT
            INTO        blackboard.command(state_id, target, type, name, args)
            VALUES      (_state_id, _target, _type, _name, _args)
            RETURNING   id
            INTO STRICT _id;
            
        _status := 0;
    END;
$$
LANGUAGE plpgsql;


--CREATE OR REPLACE FUNCTION blackboard.add_command(_state_id INTEGER,
--    _hostname TEXT, _pid BIGINT, _type TEXT, OUT _status INTEGER,
--    OUT _id INTEGER) AS
--$$
--    BEGIN
--        SELECT *
--            INTO STRICT _status, _id
--            FROM        blackboard.add_command(_state_id, _hostname, _pid,
--                        _type, NULL, NULL);
--    END;
--$$
--LANGUAGE plpgsql;


------------
-- RESULT --
------------

CREATE OR REPLACE FUNCTION blackboard.add_result(_state_id INTEGER,
    _hostname TEXT, _pid BIGINT, _command_id INTEGER, _result_code INTEGER,
    _message TEXT, OUT _status INTEGER) AS
$$
    DECLARE
        _worker_id INTEGER;
        _active_command_id INTEGER;
    BEGIN
        _status := -1;
        
        -- Check if the run_state is set to PROCESSING and row lock the
        -- shared_state to prevent concurrent updates.
        IF NOT blackboard.test_and_lock_shared_state(_state_id, 3) THEN
            RETURN;
        END IF;

        -- Get the worker_id of the calling process. Because there is no stored
        -- procedure that can change the worker_id assigned to a process after
        -- it has registered, concurrent transactions are harmless. Therefore,
        -- the register does not need to be locked.
        SELECT id
            INTO    _worker_id
            FROM    blackboard.worker_register
            WHERE   state_id = _state_id
            AND     hostname = _hostname
            AND     pid = _pid;
        
        IF NOT FOUND THEN
            RETURN;
        END IF;

        -- Find the active command for this process.
        _active_command_id := blackboard.get_active_command_id(_state_id,
            _worker_id);
            
        -- Verify that the caller is trying to add a result corresponding to its
        -- active command.
        IF _active_command_id IS NULL OR _active_command_id != _command_id THEN
            RETURN;
        END IF;

        -- Insert the result.
        INSERT
            INTO    blackboard.result (command_id, worker_id, result_code,
                    message)
            VALUES  (_command_id, _worker_id, _result_code, _message);

        _status := 0;
    END;
$$
LANGUAGE plpgsql;
    --OUT _result_count INTEGER, OUT _fail_count INTEGER) AS

CREATE OR REPLACE FUNCTION blackboard.get_command_status(_command_id INTEGER,
    OUT _status INTEGER, OUT _target CHARACTER, OUT _result_count INTEGER,
    OUT _fail_count INTEGER) AS
$$
    DECLARE
        _state_id   INTEGER;
    BEGIN
        _status := -1;
        
        SELECT state_id, target
            INTO    _state_id, _target
            FROM    blackboard.command
            WHERE   id = _command_id;
        
        IF NOT FOUND THEN
            RETURN;
        END IF;

        -- COUNT() counts everything except NULL. NULLIF(x, y) returns NULL
        -- if x = y, otherwise it returns x. NULLIF is used below to count all
        -- rows where result_code != 0.
        SELECT COUNT(1), COUNT(NULLIF(result_code = 0, TRUE))
            INTO    _result_count, _fail_count
            FROM    blackboard.worker_register AS worker_register,
                    blackboard.command AS command,
                    blackboard.result AS result
            WHERE   command.id = _command_id
            AND     (command.target IS NULL
                    OR worker_register.worker_type = command.target)
            AND     worker_register.state_id = _state_id
            AND     result.worker_id = worker_register.id
            AND     result.command_id = _command_id;
            
        _status := 0;
    END;
$$
LANGUAGE plpgsql;

CREATE TYPE blackboard.foo AS
(
    worker_type CHARACTER,
    result_count BIGINT,
    fail_count BIGINT
);

CREATE OR REPLACE FUNCTION blackboard.get_command_status3(_command_id INTEGER)
RETURNS SETOF blackboard.foo AS
$$
    SELECT worker_register.worker_type, COUNT(1), COUNT(NULLIF(result_code = 0, TRUE))
        FROM    blackboard.command AS command,
                blackboard.worker_register AS worker_register,
                blackboard.result AS result
        WHERE   command.id = $1
        AND     worker_register.state_id = command.state_id
        AND     result.worker_id = worker_register.id
        AND     result.command_id = command.id
        GROUP BY worker_register.worker_type
$$
LANGUAGE SQL;
        
CREATE OR REPLACE FUNCTION blackboard.get_command_status2(_command_id INTEGER,
    OUT _worker_type CHARACTER, OUT _result_count INTEGER,
    OUT _fail_count INTEGER)
RETURNS SETOF RECORD AS    
--    OUT _status INTEGER, OUT _result_count
--    OUT _solver_ok INTEGER, OUT _solver_fail INTEGER) AS
$$
    DECLARE
        _state_id   INTEGER;
        _row RECORD;
    BEGIN
--        _status := -1;
        
--        SELECT state_id
--            INTO    _state_id
--            FROM    blackboard.command
--            WHERE   id = _command_id;
        
--        IF NOT FOUND THEN
--            RETURN;
--        END IF;

        -- COUNT() counts everything except NULL. NULLIF(x, y) returns NULL
        -- if x = y, otherwise it returns x. NULLIF is used below to count all
        -- rows where result_code != 0.
--        SELECT COUNT(1), COUNT(NULLIF(result_code = 0, TRUE))
--        result_code = 0 AND , TRUE)), COUNT(NULLIF(result_code = 0, TRUE))
--        SELECT COUNT(NULLIF((worker_type = 'S' OR result_count != 0), TRUE))
        FOR _row IN 
            SELECT worker_register.worker_type AS a, COUNT(1) AS b, COUNT(NULLIF(result_code = 0, TRUE)) AS c
                FROM    blackboard.command AS command,
                        blackboard.worker_register AS worker_register,
                        blackboard.result AS result
                WHERE   command.id = _command_id
                AND     worker_register.state_id = command.state_id
                AND     result.worker_id = worker_register.id
                AND     result.command_id = command.id
                GROUP BY worker_register.worker_type
        LOOP
            _worker_type := _row.a;
            _result_count := _row.b;
            _fail_count := _row.c;
            RETURN NEXT;
        END LOOP;
        RETURN;
            
--        _status := 0;
    END;
$$
LANGUAGE plpgsql;

-- Return type for blackboard.get_results() that hides implementation details
-- from the caller.
CREATE TYPE blackboard.result_iface AS
(
--    command_id      INTEGER,
    hostname        TEXT,
    pid             BIGINT,
    result_code     INTEGER,
    message         TEXT
);    

--CREATE OR REPLACE FUNCTION blackboard.get_results(_command_id INTEGER)
--RETURNS SETOF blackboard.result_iface AS
--$$
--    DECLARE
--        _row        blackboard.result_iface;
--    BEGIN
--        FOR _row IN 
--            SELECT hostname, pid, result_code, message
--                FROM    blackboard.command AS command,
--                        blackboard.worker_register AS worker_register,
--                        blackboard.result AS result
--                WHERE   command.id = _command_id
--                AND     worker_register.state_id = command.state_id
--                AND     result.worker_id = worker_register.id
--                AND     result.command_id = _command_id
--        LOOP
--            RETURN NEXT _row;
--        END LOOP;
--        RETURN;
--    END;
--$$
--LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_results(_command_id INTEGER)
RETURNS SETOF blackboard.result_iface AS
$$
    SELECT hostname, pid, result_code, message
        FROM    blackboard.command AS command,
                blackboard.worker_register AS worker_register,
                blackboard.result AS result
        WHERE   command.id = $1
        AND     worker_register.state_id = command.state_id
        AND     result.worker_id = worker_register.id
        AND     result.command_id = $1;
$$
LANGUAGE SQL;

----------------------
-- HELPER FUNCTIONS --
----------------------

CREATE OR REPLACE FUNCTION blackboard.test_and_lock_shared_state
    (_state_id INTEGER, _hostname TEXT, _pid BIGINT, _run_state INTEGER)
RETURNS BOOLEAN AS
$$
    BEGIN
        -- Checks if the process is the control process _and_ if the current
        -- run_state matches the specified _run_state. The shared_state is row
        -- locked FOR SHARE for the duration of the calling transaction to
        -- prevent concurrent modification of the run_state or the control
        -- process.
        PERFORM *
            FROM        blackboard.shared_state
            WHERE       id = _state_id
            AND         run_state = _run_state
            AND         control_hostname = _hostname
            AND         control_pid = _pid
            FOR SHARE;
            
        RETURN FOUND;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.test_and_lock_shared_state
    (_state_id INTEGER, _run_state INTEGER)
RETURNS BOOLEAN AS
$$
    BEGIN
        -- Checks if the current run_state matches the specified _run_state.
        -- The shared_state is row locked FOR SHARE for the duration of the
        -- calling transaction to prevent concurrent modification of the
        -- run_state.
        PERFORM *
            FROM        blackboard.shared_state
            WHERE       id = _state_id
            AND         run_state = _run_state
            FOR SHARE;
            
        RETURN FOUND;
    END;
$$
LANGUAGE plpgsql;


---------
-- LOG --
---------

--CREATE OR REPLACE FUNCTION blackboard.log
--    (_command_id INTEGER,
--    pid INTEGER,
--    level INTEGER,
--    scope TEXT,
--    line_no INTEGER,
--    message TEXT)
--RETURNS VOID AS
--$$
--    INSERT INTO blackboard.log(command_id, pid, level, scope, line_no, message)
--        VALUES ($1, $2, $3, $4, $5, $6);
--$$
--LANGUAGE SQL;
