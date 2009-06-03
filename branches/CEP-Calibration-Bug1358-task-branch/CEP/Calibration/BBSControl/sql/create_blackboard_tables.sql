--------------------
-- SESSION STATES --
--------------------

-- -1 - FAILED
--  0 - WAITING_FOR_CONTROL
--  1 - WAITING_FOR_WORKERS
--  2 - COMPUTING_WORKER_INDEX
--  3 - workerING
--  4 - DONE

------------------
-- WORKER TYPES --
------------------

-- 0 - KERNEL
-- 1 - SOLVER


CREATE TABLE blackboard.session
(
    id                  SERIAL                      PRIMARY KEY,
    key                 TEXT                        NOT NULL UNIQUE,
    control_hostname    TEXT                        ,
    control_pid         BIGINT                      ,
    state               INTEGER                     NOT NULL DEFAULT 0,
    start               TIMESTAMP WITH TIME ZONE    NOT NULL DEFAULT now(),
    finish              TIMESTAMP WITH TIME ZONE
);

CREATE TABLE blackboard.worker
(
    id              SERIAL      PRIMARY KEY,    
    session_id      INTEGER     NOT NULL
                                REFERENCES
                                blackboard.session (id)
                                ON DELETE CASCADE,
    hostname        TEXT        ,
    pid             BIGINT      ,
    index           INTEGER     ,
    type            INTEGER     NOT NULL,
    port            INTEGER     ,
    filesys         TEXT        ,
    path            TEXT        ,
    axis_freq_lower BYTEA       ,
    axis_freq_upper BYTEA       ,
    axis_time_lower BYTEA       ,
    axis_time_upper BYTEA       ,
    
    UNIQUE (session_id, hostname, pid)
);

CREATE TABLE blackboard.command
(
    id          SERIAL      PRIMARY KEY,
    session_id  INTEGER     NOT NULL
                            REFERENCES
                            blackboard.session (id)
                            ON DELETE CASCADE,
    addressee   INTEGER     ,
    type        TEXT        NOT NULL,
    name        TEXT        ,
    args        TEXT
);

CREATE TABLE blackboard.result
(
    command_id  INTEGER                     NOT NULL
                                            REFERENCES
                                            blackboard.command (id)
                                            ON DELETE CASCADE,
    worker_id   INTEGER                     NOT NULL
                                            REFERENCES
                                            blackboard.worker (id)
                                            ON DELETE CASCADE,
    timestamp   TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    result_code INTEGER                     ,
    message     TEXT                        NOT NULL,

    UNIQUE (command_id, worker_id)
);

--CREATE TABLE blackboard.log
--(
--    command_id      INTEGER                     DEFAULT NULL
--                                                REFERENCES
--                                                blackboard.command (id)
--                                                ON DELETE CASCADE,
--
--    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now(),
--    node            INET                        DEFAULT inet_client_addr(),
--    pid             BIGINT                      ,
--    level           INTEGER                     DEFAULT 7,
--    scope           TEXT                        NOT NULL,
--    line_no         INTEGER                     NOT NULL,
--    message         TEXT                        NOT NULL
--);
