CREATE TABLE blackboard.shared_state
(
    id                  SERIAL                      PRIMARY KEY,
    key                 TEXT                        NOT NULL UNIQUE,
    control_hostname    TEXT                        ,
    control_pid         BIGINT                      ,
    run_state           INTEGER                     NOT NULL DEFAULT 0,
    run_start           TIMESTAMP WITH TIME ZONE    NOT NULL DEFAULT now(),
    run_finish          TIMESTAMP WITH TIME ZONE
);

CREATE TABLE blackboard.worker_register
(
    id              SERIAL      PRIMARY KEY,    
    state_id        INTEGER     NOT NULL
                                REFERENCES
                                blackboard.shared_state (id)
                                ON DELETE CASCADE,
    hostname        TEXT        ,
    pid             BIGINT      ,
    index           INTEGER     ,
    worker_type     CHARACTER   NOT NULL,
    port            INTEGER     ,
    filesystem      TEXT        ,
    path            TEXT        ,
    axis_freq_lower BYTEA       ,
    axis_freq_upper BYTEA       ,
    axis_time_lower BYTEA       ,
    axis_time_upper BYTEA       ,
    
    UNIQUE (state_id, hostname, pid)
);

CREATE TABLE blackboard.command
(
    id          SERIAL      PRIMARY KEY,
    state_id    INTEGER     NOT NULL
                            REFERENCES
                            blackboard.shared_state (id)
                            ON DELETE CASCADE,
    target      CHARACTER   ,
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
                                            blackboard.worker_register (id)
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
