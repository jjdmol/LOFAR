CREATE TABLE blackboard.strategy
(
    data_set                TEXT                NOT NULL,
    
    local_sky_parmdb        TEXT                NOT NULL,
    instrument_parmdb       TEXT                NOT NULL,
    history_parmdb          TEXT                NOT NULL,

    stations                TEXT[]              DEFAULT '{}',
    input_column            TEXT                DEFAULT 'DATA',
    work_domain_size        DOUBLE PRECISION[2] NOT NULL,
      
    correlation_selection   TEXT                DEFAULT 'CROSS',
    correlation_type        TEXT[]              DEFAULT '{"XX","XY","YX","YY"}'
);


-- A work_order contains all the attributes needed for job control,
-- so the structure of 'step' is not polluted. In the future, it may also
-- be used as a generalisation for different types of commands (i.e. if
-- we define commands besides executing steps, e.g. explicit data caching).
CREATE SEQUENCE blackboard.work_order_id_seq;
CREATE TABLE blackboard.work_order
(
    id              INTEGER                     PRIMARY KEY,
    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now()
);


CREATE SEQUENCE blackboard.step_id_seq;
CREATE TABLE blackboard.step
( 
    id                      INTEGER             PRIMARY KEY,
    work_order_id           INTEGER             UNIQUE NOT NULL REFERENCES blackboard.work_order (id) ON DELETE CASCADE,
    
    name                    TEXT                NOT NULL,
    operation               TEXT                NOT NULL,
    
    station1                TEXT[]              DEFAULT '{}',
    station2                TEXT[]              DEFAULT '{}',
    
    correlation_selection   TEXT                DEFAULT 'CROSS',
    correlation_type        TEXT[]              DEFAULT '{"XX","XY","YX","YY"}',
    
    sources                 TEXT[]              DEFAULT '{}',
    instrument_model        TEXT[]              DEFAULT '{}',

    output_column           TEXT                DEFAULT 'CORRECTED_DATA'
);


CREATE TABLE blackboard.solve_arguments
(
    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,
    max_iter                INTEGER             DEFAULT 1,
    epsilon                 DOUBLE PRECISION    DEFAULT 1e-6,
    min_converged           DOUBLE PRECISION    DEFAULT 100.0,
    parms                   TEXT[]              DEFAULT '{}',
    excl_parms              TEXT[]              DEFAULT '{}',
    solve_domain_size       DOUBLE PRECISION[2] NOT NULL
);


--CREATE TABLE blackboard.predict_arguments
--(
--    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,
--    output_column           TEXT                DEFAULT 'MODEL_DATA'
--);


--CREATE TABLE blackboard.subtract_arguments
--(
--    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,
--    output_column           TEXT                DEFAULT 'CORRECTED_DATA'
--);


--CREATE TABLE blackboard.correct_arguments
--(
--    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,
--    output_column           TEXT                DEFAULT 'CORRECTED_DATA'
--);


CREATE TABLE blackboard.status
(
    work_order_id   INTEGER                     NOT NULL REFERENCES blackboard.work_order (id) ON DELETE CASCADE,
    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node            INET                        DEFAULT inet_client_addr(),
    status_code     INTEGER                     DEFAULT 0,
    status_message  TEXT                        NOT NULL
);


CREATE TABLE blackboard.log
(
    work_order_id           INTEGER                     DEFAULT NULL REFERENCES blackboard.work_order (id) ON DELETE CASCADE,
    timestamp               TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node                    INET                        DEFAULT inet_client_addr(),
    level                   INTEGER                     DEFAULT 7,
    pid                     INTEGER                     ,
    scope                   TEXT                        NOT NULL,
    line_no                 INTEGER                     NOT NULL,
    message                 TEXT                        NOT NULL
);
