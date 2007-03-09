CREATE TABLE blackboard.strategy
(
    "DataSet"               TEXT                NOT NULL,
    
    "ParmDB.LocalSky"       TEXT                NOT NULL,
    "ParmDB.Instrument"     TEXT                NOT NULL,
    "ParmDB.History"        TEXT                NOT NULL,

    "Stations"              TEXT                DEFAULT '[]',
    "InputData"             TEXT                DEFAULT 'DATA',
    "WorkDomainSize.Freq"   DOUBLE PRECISION    NOT NULL,
    "WorkDomainSize.Time"   DOUBLE PRECISION    NOT NULL,
      
    "Correlation.Selection" TEXT                DEFAULT 'CROSS',
    "Correlation.Type"      TEXT                DEFAULT '["XX","XY","YX","YY"]'
);


-- A command contains all the attributes needed for control,
-- so the structure of 'step' is not polluted. In the future, it may also
-- be used as a generalisation for different types of commands (i.e. if
-- we define commands besides executing steps, e.g. explicit data caching).
CREATE SEQUENCE blackboard.command_id_seq;
CREATE TABLE blackboard.command
(
    id              INTEGER                     PRIMARY KEY,
    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now()
);


CREATE SEQUENCE blackboard.step_id_seq;
CREATE TABLE blackboard.step
( 
    id                      INTEGER             PRIMARY KEY,
    command_id              INTEGER             UNIQUE NOT NULL REFERENCES blackboard.command (id) ON DELETE CASCADE,
    
    "Name"                  TEXT                NOT NULL,
    "Operation"             TEXT                NOT NULL,
    
    "Baselines.Station1"    TEXT                DEFAULT '[]',
    "Baselines.Station2"    TEXT                DEFAULT '[]',
    
    "Correlation.Selection" TEXT                DEFAULT 'CROSS',
    "Correlation.Type"      TEXT                DEFAULT '["XX","XY","YX","YY"]',
    
    "Sources"               TEXT                DEFAULT '[]',
    "InstrumentModel"       TEXT                DEFAULT '[]',

    "OutputData"            TEXT                DEFAULT 'CORRECTED_DATA'
);


CREATE TABLE blackboard.solve_arguments
(
    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,

    "MaxIter"               INTEGER             DEFAULT 1,
    "Epsilon"               DOUBLE PRECISION    DEFAULT 1e-6,
    "MinConverged"          DOUBLE PRECISION    DEFAULT 100.0,
    "Parms"                 TEXT                DEFAULT '[]',
    "ExclParms"             TEXT                DEFAULT '[]',
    "DomainSize.Freq"       DOUBLE PRECISION    NOT NULL,
    "DomainSize.Time"       DOUBLE PRECISION    NOT NULL
);


--CREATE TABLE blackboard.predict_arguments
--(
--    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,
--    "OutputData"            TEXT                DEFAULT 'CORRECTED_DATA'
--);


--CREATE TABLE blackboard.subtract_arguments
--(
--    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,
--    "OutputData"            TEXT                DEFAULT 'CORRECTED_DATA'
--);


--CREATE TABLE blackboard.correct_arguments
--(
--    step_id                 INTEGER             UNIQUE NOT NULL REFERENCES blackboard.step (id) ON DELETE CASCADE,
--    "OutputData"            TEXT                DEFAULT 'CORRECTED_DATA'
--);


CREATE TABLE blackboard.result
(
    command_id      INTEGER                     NOT NULL REFERENCES blackboard.command (id) ON DELETE CASCADE,

    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node            INET                        DEFAULT inet_client_addr(),
    result_code     INTEGER                     DEFAULT 0,
    result_message  TEXT                        NOT NULL
);


CREATE TABLE blackboard.log
(
    command_id              INTEGER                     DEFAULT NULL REFERENCES blackboard.command (id) ON DELETE CASCADE,

    timestamp               TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node                    INET                        DEFAULT inet_client_addr(),
    level                   INTEGER                     DEFAULT 7,
    pid                     INTEGER                     ,
    scope                   TEXT                        NOT NULL,
    line_no                 INTEGER                     NOT NULL,
    message                 TEXT                        NOT NULL
);
