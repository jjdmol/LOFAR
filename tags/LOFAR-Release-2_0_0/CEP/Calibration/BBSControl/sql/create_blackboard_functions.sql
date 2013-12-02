-- create_blackboard_functions.sql
--
-- Copyright (C) 2007
-- ASTRON (Netherlands Institute for Radio Astronomy)
-- P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
--
-- This file is part of the LOFAR software suite.
-- The LOFAR software suite is free software: you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as published
-- by the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- The LOFAR software suite is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License along
-- with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
--
-- $Id$

-------------
-- SESSION --
-------------

CREATE OR REPLACE FUNCTION blackboard.init_session(_key TEXT, OUT _id INTEGER)
    AS
$$
    BEGIN
        BEGIN
            INSERT INTO blackboard.session (key)
                VALUES  (_key);
        EXCEPTION
            WHEN unique_violation THEN
                -- NOT considered an error.
                NULL;
        END;

        -- We cannot use a RETURNING clause on the above INSERT to return the
        -- session id: If the unique_violation is triggered we still want to
        -- return the id of the (existing) session, but postgres returns NULL
        -- in that case.
        SELECT id
            INTO    _id
            FROM    blackboard.session
            WHERE   key = _key;
    END;
$$
LANGUAGE plpgsql;

-- Return type for blackboard.get_session_info() that excludes the time axis and
-- parset fields.
CREATE TYPE blackboard.session_iface AS
(
    id                  INTEGER,
    key                 TEXT,
    control_hostname    TEXT,
    control_pid         BIGINT,
    state               INTEGER,
    chunk_count         INTEGER,
    start               TIMESTAMP WITH TIME ZONE,
    finish              TIMESTAMP WITH TIME ZONE,
    duration            INTERVAL
);

CREATE OR REPLACE FUNCTION blackboard.get_session_info(_session_key TEXT)
    RETURNS blackboard.session_iface AS
$$
    SELECT  id, key, control_hostname, control_pid, state, chunk_count, start,
            finish, COALESCE(finish, now()) - start AS duration
        FROM    blackboard.session
        WHERE   key = $1;
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION blackboard.get_session_info(_active BOOLEAN)
    RETURNS SETOF blackboard.session_iface AS
$$
    SELECT  id, key, control_hostname, control_pid, state, chunk_count, start,
            finish, COALESCE(finish, now()) - start AS duration
        FROM        blackboard.session
        WHERE       $1 IS FALSE
        OR          ($1 IS TRUE
                    AND state >= 0
                    AND state <= 3)
        ORDER BY    id;
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION blackboard.set_state(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _state INTEGER, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Verify that the caller is the control process and set the state in a
        -- single query. Also, set the finish field if appropriate.
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

CREATE OR REPLACE FUNCTION blackboard.set_axis_time(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _axis_time_lower BYTEA, _axis_time_upper BYTEA,
    OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Verify that the caller is the control process and that the session
        -- state equals INITIALIZING, and set the time axis in a single query.
        UPDATE blackboard.session
            SET     axis_time_lower = _axis_time_lower,
                    axis_time_upper = _axis_time_upper
            WHERE   id = _session_id
            AND     control_hostname = _hostname
            AND     control_pid = _pid
            AND     state = 2;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_axis_time(_session_id INTEGER,
    OUT _status INTEGER, OUT _axis_time_lower BYTEA, OUT _axis_time_upper BYTEA)
    AS
$$
    BEGIN
        _status := -1;

        -- Get session time axis and verify that the session has been
        -- initialized (i.e. session state > INTIALIZED).
        SELECT      axis_time_lower, axis_time_upper
            INTO    _axis_time_lower, _axis_time_upper
            FROM    blackboard.session
            WHERE   id = _session_id
            AND     state > 2;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.set_parset(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _parset TEXT, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Test if caller is the control process and set the parset in a single
        -- query.
        UPDATE blackboard.session
            SET     parset = _parset
            WHERE   id = _session_id
            AND     control_hostname = _hostname
            AND     control_pid = _pid;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_parset(_session_id INTEGER,
    OUT _status INTEGER, OUT _parset TEXT) AS
$$
    BEGIN
        _status := -1;

        SELECT parset
            INTO    _parset
            FROM    blackboard.session
            WHERE   id = _session_id;

        IF FOUND AND _parset IS NOT NULL THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.set_chunk_count(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _chunk_count INTEGER, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Verify that the caller is the control process and that the session
        -- state equals INITIALIZING, and set the chunk count in a single query.
        UPDATE blackboard.session
            SET     chunk_count = _chunk_count
            WHERE   id = _session_id
            AND     control_hostname = _hostname
            AND     control_pid = _pid
            AND     state = 2;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_chunk_count(_session_id INTEGER,
    OUT _status INTEGER, OUT _chunk_count INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Get session chunk count and verify that the session has been
        -- initialized (i.e. session state > INTIALIZED).
        SELECT      chunk_count
            INTO    _chunk_count
            FROM    blackboard.session
            WHERE   id = _session_id
            AND     state > 2;

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
        -- state equals WAITING_FOR_CONTROL. Row lock the session to prevent
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
        -- state equals WAITING_FOR_CONTROL. Row lock the session to prevent
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

        -- Verify that the session state equals WAITING_FOR_CONTROL and try
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
    _freq_lower DOUBLE PRECISION, _freq_upper DOUBLE PRECISION,
    _time_lower DOUBLE PRECISION, _time_upper DOUBLE PRECISION,
    _axis_freq_lower BYTEA, _axis_freq_upper BYTEA, _axis_time_lower BYTEA,
    _axis_time_upper BYTEA, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Verify that the session state equals WAITING_FOR_WORKERS and row lock
        -- the session to prevent concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, 1) THEN
            RETURN;
        END IF;

        -- Try to register.
        UPDATE blackboard.worker
            SET     hostname = _hostname, pid = _pid,
                    freq_lower = _freq_lower,
                    freq_upper = _freq_upper,
                    time_lower = _time_lower,
                    time_upper = _time_upper,
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

        -- Verify that the session state equals WAITING_FOR_WORKERS and row lock
        -- the session to prevent concurrent updates.
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

-- Return type for blackboard.get_worker_register() that excludes the frequency
-- and time axis information (which can take a lot of space).
CREATE TYPE blackboard.worker_register_iface AS
(
    hostname    TEXT,
    pid         BIGINT,
    index       INTEGER,
    type        INTEGER,
    port        INTEGER,
    filesys     TEXT,
    path        TEXT,
    freq_lower  DOUBLE PRECISION,
    freq_upper  DOUBLE PRECISION,
    time_lower  DOUBLE PRECISION,
    time_upper  DOUBLE PRECISION
);

CREATE OR REPLACE FUNCTION blackboard.get_worker_register(_session_id INTEGER)
    RETURNS SETOF blackboard.worker_register_iface AS
$$
    SELECT  hostname, pid, index, type, port, filesys, path, freq_lower,
            freq_upper, time_lower, time_upper
        FROM        blackboard.worker
        WHERE       session_id = $1
        ORDER BY    id;
$$
LANGUAGE SQL;

CREATE OR REPLACE FUNCTION blackboard.get_worker_axis_freq(_session_id INTEGER,
    _worker_hostname TEXT, _worker_pid BIGINT, OUT _status INTEGER,
    OUT _axis_freq_lower BYTEA, OUT _axis_freq_upper BYTEA) AS
$$
    BEGIN
        _status := -1;

        SELECT axis_freq_lower, axis_freq_upper
            INTO    _axis_freq_lower, _axis_freq_upper
            FROM    blackboard.worker
            WHERE   session_id = _session_id
            AND     hostname = _worker_hostname
            AND     pid = _worker_pid
            AND     type = 0;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.get_worker_axis_time(_session_id INTEGER,
    _worker_hostname TEXT, _worker_pid BIGINT, OUT _status INTEGER,
    OUT _axis_time_lower BYTEA, OUT _axis_time_upper BYTEA) AS
$$
    BEGIN
        _status := -1;

        SELECT axis_time_lower, axis_time_upper
            INTO    _axis_time_lower, _axis_time_upper
            FROM    blackboard.worker
            WHERE   session_id = _session_id
            AND     hostname = _worker_hostname
            AND     pid = _worker_pid
            AND     type = 0;

        IF FOUND THEN
            _status := 0;
        END IF;
    END;
$$
LANGUAGE plpgsql;

CREATE OR REPLACE FUNCTION blackboard.set_worker_index(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _worker_hostname TEXT, _worker_pid BIGINT,
    _index INTEGER, OUT _status INTEGER) AS
$$
    BEGIN
        _status := -1;

        -- Verify that the caller is the control process and that the session
        -- state equals INITIALIZING. Row lock the session to prevent concurrent
        -- updates.
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
        -- error. However, this situation never occurs because of the UNIQUE
        -- column constraint on blackboard.command.id.
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

        SELECT type, name, args
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

        INSERT INTO blackboard.command(session_id, addressee, type, name, args)
            VALUES      (_session_id, _addressee, _type, _name, _args)
            RETURNING   id
            INTO        _id;

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

        -- Check if the state equals PROCESSING and row lock the session to
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
    hostname    TEXT,
    pid         BIGINT,
    result_code INTEGER,
    message     TEXT
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

--------------
-- PROGRESS --
--------------

CREATE OR REPLACE FUNCTION blackboard.set_progress(_session_id INTEGER,
    _hostname TEXT, _pid BIGINT, _chunk_count INTEGER, OUT _status INTEGER) AS
$$
    DECLARE
        _worker_id INTEGER;
    BEGIN
        _status := -1;

        -- Check if the state equals PROCESSING and row lock the session to
        -- prevent concurrent updates.
        IF NOT blackboard.test_and_lock_session(_session_id, 3) THEN
            RETURN;
        END IF;

        -- Get the worker_id of the calling process and check the type field to
        -- verify the calling process is a KERNEL process. Because there is no
        -- stored procedure that can change the worker_id assigned to a process
        -- after it has registered, concurrent transactions are harmless.
        SELECT id
            INTO    _worker_id
            FROM    blackboard.worker
            WHERE   session_id = _session_id
            AND     hostname = _hostname
            AND     pid = _pid
            AND     type = 0;

        IF NOT FOUND THEN
            RETURN;
        END IF;

        UPDATE blackboard.progress
            SET     chunk_count = _chunk_count,
                    timestamp = now()
            WHERE   worker_id = _worker_id;

        -- The following INSERT may trigger a unique_violation if another
        -- process inserted a progress result for the same (_session_id,
        -- _worker_id) pair. That would indicate a serious problem because the
        -- _worker_id is linked to a specific (hostname, pid) pair through the
        -- blackboard.worker table. Therefore, we let this exception propagate.
        IF NOT FOUND THEN
            INSERT INTO blackboard.progress (worker_id, chunk_count)
                VALUES  (_worker_id, _chunk_count);
        END IF;

        _status := 0;
    END;
$$
LANGUAGE plpgsql;

-- Return type for blackboard.get_progress() that includes a hostname field and
-- a pid field instead of a worker_id field and excludes the session_id field.
CREATE TYPE blackboard.progress_iface AS
(
    hostname    TEXT,
    pid         BIGINT,
    timestamp   TIMESTAMP WITH TIME ZONE,
    chunk_count INTEGER
);

CREATE OR REPLACE FUNCTION blackboard.get_progress(_session_id INTEGER)
    RETURNS SETOF blackboard.progress_iface AS
$$
    SELECT  hostname, pid, timestamp, chunk_count
        FROM        blackboard.worker AS worker,
                    blackboard.progress AS progress
        WHERE       worker.session_id = $1
        AND         progress.worker_id = worker.id
        ORDER BY    worker.id;
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
