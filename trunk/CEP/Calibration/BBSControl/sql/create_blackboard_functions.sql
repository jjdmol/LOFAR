-------------
-- SESSION --
-------------

CREATE OR REPLACE FUNCTION blackboard.init_session(_key TEXT, OUT _id INTEGER)
    AS
$$
    BEGIN
        BEGIN
            INSERT INTO blackboard.session (key)
                VALUES      (_key);
        EXCEPTION
            WHEN unique_violation THEN
                -- NOT considered an error.
                NULL;
        END;

        -- Should always succeed.
        -- STRICT is a postgresql 8.2 feature, so it cannot be used.
        SELECT id
--            INTO STRICT _id
            INTO    _id
            FROM    blackboard.session
            WHERE   key = _key;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.set_state(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _state INTEGER, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Test if caller is the control process and set the state in a single
        -- query. Also, set the finish field if appropriate.
        UPDATE blackboard.session
            SET     state = _state
            WHERE   id = _session_id
            AND     control_hostname = _hostname
            AND     control_pid = _pid;

        IF FOUND AND (_state = -1 OR _state = 4) THEN
            UPDATE blackboard.session
                SET     finish = now()
                WHERE   id = _session_id;
        END IF;            
                
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_state(_session_id INTEGER,
    OUT _status INTEGER, OUT _state INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        SELECT state
            INTO    _state
            FROM    blackboard.session
            WHERE   id = _session_id;
        
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;


------------------
-- REGISTRATION --
------------------

CREATE OR REPLACE FUNCTION blackboard.create_kernel_slot(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _filesys TEXT, _path TEXT)
RETURNS VOID AS
$$
    BEGIN
        -- Verify that the caller is the control process and that the session
        -- state is set to WAITING_FOR_CONTROL. Row lock the session to prevent
        -- concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, _hostname, _pid, 0)
            THEN
            RAISE EXCEPTION 'Operation not permitted';
        END IF;

        INSERT INTO blackboard.worker (session_id, type, filesys, path)
            VALUES  (_session_id, 0, _filesys, _path);
    END;
$$    
LANGUAGE plpgsql;
    
CREATE OR REPLACE FUNCTION blackboard.create_solver_slot(_session_id INTEGER,
    _hostname TEXT, _pid INTEGER)
RETURNS VOID AS
$$
    BEGIN
        -- Verify that the caller is the control process and that the session
        -- state is set to WAITING_FOR_CONTROL. Row lock the session to prevent
        -- concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, _hostname, _pid, 0)
            THEN
            RAISE EXCEPTION 'Operation not permitted';
        END IF;

        INSERT INTO blackboard.worker (session_id, type)
            VALUES  (_session_id, 1);
    END;
$$    
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.register_as_control(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        -- Verify that the session state is set to WAITING_FOR_CONTROL and try
        -- to register in a single query.
        UPDATE blackboard.session
            SET     control_hostname = _hostname, control_pid = _pid
            WHERE   id = _session_id
            AND     state = 0
            AND     control_hostname IS NULL
            AND     control_pid IS NULL;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$    
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.register_as_kernel(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _filesys TEXT, _path TEXT,
    _axis_freq_lower BYTEA, _axis_freq_upper BYTEA, _axis_time_lower BYTEA,
    _axis_time_upper BYTEA, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Verify that the session state is set to WAITING_FOR_WORKERS and row
        -- lock the session to prevent concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, 1) THEN
            RETURN;
        END IF;

        -- Try to register.
        UPDATE blackboard.worker
            SET     hostname = _hostname, pid = _pid,
                    axis_freq_lower = _axis_freq_lower,
                    axis_freq_upper = _axis_freq_upper,
                    axis_time_lower = _axis_time_lower,
                    axis_time_upper = _axis_time_upper
            WHERE   session_id = _session_id
            AND     type = 0
            AND     hostname IS NULL
            AND     pid IS NULL
            AND     filesys = _filesys
            AND     path = _path;
        
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.register_as_solver(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _port INTEGER, OUT _status INTEGER) AS
$$
    DECLARE
        _slot_id    INTEGER;
    BEGIN
        _status := -1;

        -- Verify that the session state is set to WAITING_FOR_WORKERS and row
        -- lock the session to prevent concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, 1) THEN
            RETURN;
        END IF;

        -- Row lock all remaining free slots FOR UPDATE in order of increasing
        -- id and return the free slot with the lowest id.
        -- TODO: Does the ORDER BY clause affect the order in which the rows
        -- are locked? If not, the following query may create a deadlock.
        SELECT id
            INTO        _slot_id
            FROM        blackboard.worker
            WHERE       session_id = _session_id
            AND         type = 1
            AND         hostname IS NULL
            AND         pid IS NULL
            ORDER BY    id
            FOR UPDATE;

        IF NOT FOUND THEN
            RETURN;
        END IF;
            
        -- Try to register.
        UPDATE blackboard.worker
            SET     hostname = _hostname, pid = _pid, port = _port
            WHERE   id = _slot_id;
            
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_worker_register(_session_id INTEGER)
RETURNS SETOF blackboard.worker AS
$$
    SELECT *
        FROM        blackboard.worker
        WHERE       session_id = $1
        ORDER BY    id;
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION blackboard.set_worker_index(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _worker_hostname TEXT, _worker_pid BIGINT,
    _index INTEGER, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        -- Verify that the caller is the control process and that the session 
        -- state is set to COMPUTING_WORKER_INDEX. Row lock the session to
        -- prevent concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, _hostname, _pid, 2)
            THEN
            RETURN;
        END IF;
        
        UPDATE blackboard.worker
            SET     index = _index
            WHERE   session_id = _session_id
            AND     hostname = _worker_hostname
            AND     pid = _worker_pid;
            
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
CREATE OR REPLACE FUNCTION blackboard.get_active_command_id(_session_id INTEGER,
    _worker_id INTEGER, OUT _id INTEGER) AS
$$
    BEGIN
        -- Note that STRICT is not used with INTO in the query below because it
        -- can return zero rows, which is OK and should not raise an exception.
        -- Without the STRICT modifier, returning multiple rows will not raise
        -- an exception, even though for this query it should be considered an
        -- error. However, this is already covered by the UNIQUE column
        -- constraint on blackboard.command.id.
        --
        -- PRECONDITION: _worker_id refers to a worker that is registered to the
        -- session with id _session_id.
        SELECT command.id
            INTO        _id
            FROM        blackboard.command AS command,
                        blackboard.worker AS worker
            WHERE       command.session_id = _session_id
            AND         worker.id = _worker_id
            AND         (command.addressee IS NULL
                        OR command.addressee = worker.type)
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
CREATE OR REPLACE FUNCTION blackboard.get_command(_session_id INTEGER,
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
        -- the worker register does not need to be locked.
        SELECT id
            INTO    _worker_id
            FROM    blackboard.worker
            WHERE   session_id = _session_id
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
        _id := blackboard.get_active_command_id(_session_id, _worker_id);
        IF _id IS NULL THEN
            -- Queue is empty, so return an empty command.
            _status := -2;
            RETURN;
        END IF;

        -- STRICT is a postgresql 8.2 feature, so it cannot be used.
        SELECT type, name, args
--            INTO STRICT _type, _name, _args
            INTO    _type, _name, _args
            FROM    blackboard.command
            WHERE   id = _id;
            
        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$    
LANGUAGE plpgsql;


-------------
-- COMMAND --
-------------

CREATE OR REPLACE FUNCTION blackboard.post_command(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _addressee INTEGER, _type TEXT, _name TEXT,
    _args TEXT, OUT _status INTEGER, OUT _id INTEGER) AS
$$
    BEGIN
        _status := -1;
        
        -- Verify that the caller is the control process and that the state is
        -- set to PROCESSING. Row lock the session to prevent concurrent
        -- updates.
        IF NOT blackboard.test_and_lock_session(_session_id, _hostname, _pid,
            3) THEN
            RETURN;
        END IF;

        -- STRICT is a postgresql 8.2 feature, so it cannot be used.
        -- RETURNING is a postgresql 8.2 feature, so it cannot be used.
        INSERT
            INTO        blackboard.command(session_id, addressee, type, name,
                        args)
            VALUES      (_session_id, _addressee, _type, _name, _args);
--            RETURNING   id
--            INTO STRICT _id;

        SELECT lastval()
            INTO    _id;

        _status := 0;
    END;
$$
LANGUAGE plpgsql;


------------
-- RESULT --
------------

CREATE OR REPLACE FUNCTION blackboard.post_result(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _command_id INTEGER, _result_code INTEGER,
    _message TEXT, OUT _status INTEGER) AS
$$
    DECLARE
        _worker_id INTEGER;
        _active_command_id INTEGER;
    BEGIN
        _status := -1;
        
        -- Check if the state is set to PROCESSING and row lock the session to
        -- prevent concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, 3) THEN
            RETURN;
        END IF;

        -- Get the worker_id of the calling process. Because there is no stored
        -- procedure that can change the worker_id assigned to a process after
        -- it has registered, concurrent transactions are harmless. Therefore,
        -- the worker register does not need to be locked.
        SELECT id
            INTO    _worker_id
            FROM    blackboard.worker
            WHERE   session_id = _session_id
            AND     hostname = _hostname
            AND     pid = _pid;
        
        IF NOT FOUND THEN
            RETURN;
        END IF;

        -- Find the active command for this process.
        _active_command_id := blackboard.get_active_command_id(_session_id,
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

CREATE OR REPLACE FUNCTION blackboard.get_command_status(_command_id INTEGER,
    OUT _status INTEGER, OUT _addressee INTEGER, OUT _result_count INTEGER,
    OUT _fail_count INTEGER) AS
$$
    DECLARE
        _session_id   INTEGER;
    BEGIN
        _status := -1;
        
        SELECT session_id, addressee
            INTO    _session_id, _addressee
            FROM    blackboard.command
            WHERE   id = _command_id;
        
        IF NOT FOUND THEN
            RETURN;
        END IF;

        -- COUNT() counts everything except NULL. NULLIF(x, y) returns NULL if
        -- x = y, otherwise it returns x. NULLIF is used below to count all rows
        -- where result_code != 0.
        SELECT COUNT(1), COUNT(NULLIF(result_code = 0, TRUE))
            INTO    _result_count, _fail_count
            FROM    blackboard.worker AS worker,
                    blackboard.command AS command,
                    blackboard.result AS result
            WHERE   command.id = _command_id
            AND     (command.addressee IS NULL
                    OR command.addressee = worker.type)
            AND     worker.session_id = _session_id
            AND     result.worker_id = worker.id
            AND     result.command_id = _command_id;
            
        _status := 0;
    END;
$$
LANGUAGE plpgsql;

-- Return type for blackboard.get_results() that hides implementation details
-- from the caller.
CREATE TYPE blackboard.result_iface AS
(
    hostname        TEXT,
    pid             BIGINT,
    result_code     INTEGER,
    message         TEXT
);    

CREATE OR REPLACE FUNCTION blackboard.get_results(_command_id INTEGER)
RETURNS SETOF blackboard.result_iface AS
$$
    SELECT hostname, pid, result_code, message
        FROM    blackboard.command AS command,
                blackboard.worker AS worker,
                blackboard.result AS result
        WHERE   command.id = $1
        AND     worker.session_id = command.session_id
        AND     result.worker_id = worker.id
        AND     result.command_id = $1;
$$
LANGUAGE SQL;


----------------------
-- HELPER FUNCTIONS --
----------------------

CREATE OR REPLACE FUNCTION blackboard.test_and_lock_session(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _state INTEGER)
RETURNS BOOLEAN AS
$$
    BEGIN
        -- Checks if the process is the control process _and_ if the current
        -- session state matches the specified _state. The session is row locked
        -- FOR SHARE for the duration of the calling transaction to prevent
        -- concurrent modification of the state or the control process.
        PERFORM *
            FROM        blackboard.session
            WHERE       id = _session_id
            AND         state = _state
            AND         control_hostname = _hostname
            AND         control_pid = _pid
            FOR SHARE;
            
        RETURN FOUND;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.test_and_lock_session(_session_id INTEGER,
    _state INTEGER)
RETURNS BOOLEAN AS
$$
    BEGIN
        -- Checks if the current state matches the specified _state. The session
        -- is row locked FOR SHARE for the duration of the calling transaction
        -- to prevent concurrent modification of the session state.
        PERFORM *
            FROM        blackboard.session
            WHERE       id = _session_id
            AND         state = _state
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
