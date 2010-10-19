-- create_blackboard_tables.sql
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
