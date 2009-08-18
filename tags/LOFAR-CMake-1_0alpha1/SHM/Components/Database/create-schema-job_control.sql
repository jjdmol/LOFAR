DROP SCHEMA job_control         CASCADE;
CREATE SCHEMA job_control;

-- NOTE: TIMESTAMP WITH TIME ZONE will store its timestamps in UTC, while the time zones
--       for TIMESTAMP WITHOUT TIME ZONE are basically unspecified. We prefer the former.


CREATE TABLE job_control.queue (
    id              BIGSERIAL                     PRIMARY KEY,
    name            TEXT                          NOT NULL,
    scheduled_time  TIMESTAMP WITH TIME ZONE      ,
    ticket_no       BIGINT                        NOT NULL,
    state           INT                           NOT NULL,
    client          INET                          ,
    time_of_bind    TIMESTAMP WITH TIME ZONE      ,
    watchdog        TIMESTAMP WITH TIME ZONE      ,
    wd_timeout      INTERVAL                      ,
    period          INTERVAL                      ,
    command         TEXT                          NOT NULL
);

CREATE TABLE job_control.log (
    timestamp       TIMESTAMP WITH TIME ZONE      NOT NULL,
    level           INT                           NOT NULL,
    pid             INT                           NOT NULL,
    context         TEXT                          NOT NULL,
    line_no         INT                           NOT NULL,
    message         TEXT                          NOT NULL,
    job_id          BIGINT                        ,
    job_ticket_no   BIGINT                        ,
    source          INET                          NOT NULL
    -- constraints
    PRIMARY KEY(timestamp)
);
