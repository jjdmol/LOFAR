
-- NOTE: Initialize the "Systems" schema before this one!

DROP SCHEMA Observations CASCADE;

CREATE SCHEMA Observations;

CREATE TABLE Observations.ObservableTypes (
    type_id         INTEGER                       PRIMARY KEY,
    type_name       TEXT                          UNIQUE NOT NULL
);

INSERT INTO Observations.ObservableTypes(type_id, type_name) VALUES (1, 'string');
INSERT INTO Observations.ObservableTypes(type_id, type_name) VALUES (2, 'integer');
INSERT INTO Observations.ObservableTypes(type_id, type_name) VALUES (3, 'boolean');
INSERT INTO Observations.ObservableTypes(type_id, type_name) VALUES (4, 'float');

CREATE TABLE Observations.Observables (
    obs_id          BIGSERIAL                     PRIMARY KEY,
    si_id           BIGINT                        NOT NULL REFERENCES Systems.SystemInstances ON DELETE CASCADE ON UPDATE CASCADE,
    obs_name        TEXT                          NOT NULL,
    obs_unit        TEXT                          NOT NULL,
    type_id         INTEGER                       NOT NULL REFERENCES Observations.ObservableTypes,
    UNIQUE(si_id, obs_name)
);

--INSERT INTO Observables(obs_name, obs_unit, si_id, type_id) VALUES('outside_temperature', 'Kelvin');

-- NOTE: TIMESTAMP WITH TIME ZONE will store its timestamps in UTC, while the time zones
--       for TIMESTAMP WITHOUT TIME ZONE are basically unspecified. We prefer the former.

CREATE TABLE Observations.Observations (
    time            TIMESTAMP WITH TIME ZONE      NOT NULL,
    obs_id          BIGINT                        REFERENCES Observations.Observables ON DELETE CASCADE,
    value           TEXT                          NOT NULL,
    -- constraints
    PRIMARY KEY(time, obs_id)
);

CREATE INDEX Observations_obs_id_time_idx ON Observations.Observations(obs_id, time);

------------------------------------------------------------------------------------

CREATE VIEW Observations.FullObservations AS SELECT sc_name, sc_description, si_name, si_description, obs_name, obs_unit, type_name, time, value FROM Systems.SystemClasses NATURAL JOIN Systems.SystemInstances NATURAL JOIN Observations.Observables NATURAL JOIN Observations.ObservableTypes NATURAL JOIN Observations.Observations;
