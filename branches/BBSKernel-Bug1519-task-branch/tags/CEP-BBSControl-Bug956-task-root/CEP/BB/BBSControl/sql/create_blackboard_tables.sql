CREATE TABLE blackboard.strategy
(
    state                   TEXT                DEFAULT 'UNDEFINED',

    "DataSet"               TEXT                NOT NULL,

    "ParmDB.LocalSky"       TEXT                NOT NULL,
    "ParmDB.Instrument"     TEXT                NOT NULL,
    "ParmDB.History"        TEXT                NOT NULL,

    "Stations"              TEXT                DEFAULT '[]',
    "InputData"             TEXT                DEFAULT 'DATA',

    "RegionOfInterest.Freq" TEXT                DEFAULT '[]',
    "RegionOfInterest.Time" TEXT                DEFAULT '[]',

    "WorkDomainSize.Freq"   DOUBLE PRECISION    NOT NULL,
    "WorkDomainSize.Time"   DOUBLE PRECISION    NOT NULL,

    "Correlation.Selection" TEXT                DEFAULT 'CROSS',
    "Correlation.Type"      TEXT                DEFAULT '[]'
);


CREATE SEQUENCE blackboard.command_id_seq;
CREATE TABLE blackboard.command
(
    id                      INTEGER             PRIMARY KEY,
    "Type"                  TEXT                NOT NULL
);


CREATE TABLE blackboard.single_step_args
(
    command_id              INTEGER             UNIQUE
                                                NOT NULL
                                                REFERENCES
                                                blackboard.command(id)
                                                ON DELETE CASCADE,

    "Name"                  TEXT                NOT NULL,
    "Operation"             TEXT                NOT NULL,

    "Baselines.Station1"    TEXT                DEFAULT '[]',
    "Baselines.Station2"    TEXT                DEFAULT '[]',

    "Correlation.Selection" TEXT                DEFAULT 'CROSS',
    "Correlation.Type"      TEXT                DEFAULT '[]',

    "Sources"               TEXT                DEFAULT '[]',
    "InstrumentModel"       TEXT                DEFAULT '[]',

    "OutputData"            TEXT                NOT NULL
);


CREATE TABLE blackboard.solve_args
(
    command_id              INTEGER             UNIQUE
                                                NOT NULL
                                                REFERENCES
                                                blackboard.command (id)
                                                ON DELETE CASCADE,

    "MaxIter"               INTEGER             NOT NULL,
    "Epsilon"               DOUBLE PRECISION    NOT NULL,
    "MinConverged"          DOUBLE PRECISION    NOT NULL,
    "Parms"                 TEXT                NOT NULL,
    "ExclParms"             TEXT                DEFAULT '[]',
    "DomainSize.Freq"       DOUBLE PRECISION    NOT NULL,
    "DomainSize.Time"       DOUBLE PRECISION    NOT NULL
);


CREATE TABLE blackboard.result
(
    command_id      INTEGER                     NOT NULL
                                                REFERENCES
                                                blackboard.command (id)
                                                ON DELETE CASCADE,

    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node            INET                        DEFAULT inet_client_addr(),
    result_code     INTEGER                     NOT NULL,
    message         TEXT                        NOT NULL,
    read_flag       BOOL                        DEFAULT 'false',
    
    UNIQUE (command_id, node)
);


CREATE TABLE blackboard.log
(
    command_id      INTEGER                     DEFAULT NULL
                                                REFERENCES
                                                blackboard.command (id)
                                                ON DELETE CASCADE,

    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node            INET                        DEFAULT inet_client_addr(),
    level           INTEGER                     DEFAULT 7,
    pid             INTEGER                     ,
    scope           TEXT                        NOT NULL,
    line_no         INTEGER                     NOT NULL,
    message         TEXT                        NOT NULL
);
