CREATE TYPE blackboard.iface_single_step_args AS
(
    "Name"                  TEXT,
    "Operation"             TEXT,
    "Baselines.Station1"    TEXT,
    "Baselines.Station2"    TEXT,
    "Correlation.Selection" TEXT,
    "Correlation.Type"      TEXT,
    "Sources"               TEXT,
    "InstrumentModel"       TEXT,
    "OutputData"            TEXT
);

CREATE TYPE blackboard.iface_predict_args AS
(
    "Name"                  TEXT,
    "Operation"             TEXT,
    "Baselines.Station1"    TEXT,
    "Baselines.Station2"    TEXT,
    "Correlation.Selection" TEXT,
    "Correlation.Type"      TEXT,
    "Sources"               TEXT,
    "InstrumentModel"       TEXT,
    "OutputData"            TEXT
);

CREATE TYPE blackboard.iface_subtract_args AS
(
    "Name"                  TEXT,
    "Operation"             TEXT,
    "Baselines.Station1"    TEXT,
    "Baselines.Station2"    TEXT,
    "Correlation.Selection" TEXT,
    "Correlation.Type"      TEXT,
    "Sources"               TEXT,
    "InstrumentModel"       TEXT,
    "OutputData"            TEXT
);

CREATE TYPE blackboard.iface_correct_args AS
(
    "Name"                  TEXT,
    "Operation"             TEXT,
    "Baselines.Station1"    TEXT,
    "Baselines.Station2"    TEXT,
    "Correlation.Selection" TEXT,
    "Correlation.Type"      TEXT,
    "Sources"               TEXT,
    "InstrumentModel"       TEXT,
    "OutputData"            TEXT
);

CREATE TYPE blackboard.iface_solve_args AS
(
    "Name"                  TEXT,
    "Operation"             TEXT,
    "Baselines.Station1"    TEXT,
    "Baselines.Station2"    TEXT,
    "Correlation.Selection" TEXT,
    "Correlation.Type"      TEXT,
    "Sources"               TEXT,
    "InstrumentModel"       TEXT,
    "OutputData"            TEXT,
    "MaxIter"               INTEGER,
    "Epsilon"               DOUBLE PRECISION,
    "MinConverged"          DOUBLE PRECISION,
    "Parms"                 TEXT,
    "ExclParms"             TEXT,
    "DomainSize.Freq"       DOUBLE PRECISION,
    "DomainSize.Time"       DOUBLE PRECISION
);
