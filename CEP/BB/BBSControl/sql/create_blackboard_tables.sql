CREATE TABLE blackboard.strategy
(
    state                               TEXT                DEFAULT 'UNDEFINED',

    "DataSet"                           TEXT                NOT NULL,

    "ParmDB.Sky"                        TEXT                NOT NULL,
    "ParmDB.Instrument"                 TEXT                NOT NULL,
    "ParmDB.History"                    TEXT                NOT NULL,

    "Strategy.Stations"                 TEXT                DEFAULT '[]',
    "Strategy.InputColumn"              TEXT                DEFAULT 'DATA',

    "Strategy.RegionOfInterest.Freq"    TEXT                DEFAULT '[]',
    "Strategy.RegionOfInterest.Time"    TEXT                DEFAULT '[]',

    "Strategy.ChunkSize"                INTEGER             NOT NULL,

    "Strategy.Correlation.Selection"    TEXT                DEFAULT 'CROSS',
    "Strategy.Correlation.Type"         TEXT                DEFAULT '[]'
);


CREATE SEQUENCE blackboard.command_id_seq;
CREATE TABLE blackboard.command
(
    id                      INTEGER             PRIMARY KEY,
    "Type"                  TEXT                NOT NULL,
    "Name"                  TEXT                ,
    "ParameterSet"          TEXT
);


CREATE TABLE blackboard.result
(
    command_id      INTEGER                     NOT NULL
                                                REFERENCES
                                                blackboard.command (id)
                                                ON DELETE CASCADE,

    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node            INET                        DEFAULT inet_client_addr(),
    pid             INTEGER                     NOT NULL,
    sender_type     INTEGER                     DEFAULT -1,
    sender_id       INTEGER                     DEFAULT -1,
    result_code     INTEGER                     DEFAULT -1,
    message         TEXT                        NOT NULL,
    read_flag       BOOL                        DEFAULT 'false',
    
    UNIQUE (command_id, node, pid)
);


CREATE TABLE blackboard.log
(
    command_id      INTEGER                     DEFAULT NULL
                                                REFERENCES
                                                blackboard.command (id)
                                                ON DELETE CASCADE,

    timestamp       TIMESTAMP WITH TIME ZONE    DEFAULT now(),
    node            INET                        DEFAULT inet_client_addr(),
    pid             INTEGER                     ,
    level           INTEGER                     DEFAULT 7,
    scope           TEXT                        NOT NULL,
    line_no         INTEGER                     NOT NULL,
    message         TEXT                        NOT NULL
);
