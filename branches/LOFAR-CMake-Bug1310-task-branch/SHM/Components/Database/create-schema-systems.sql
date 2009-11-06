
DROP SCHEMA Systems CASCADE;

CREATE SCHEMA Systems;

CREATE TABLE Systems.SystemClasses (
    sc_id           INTEGER                       PRIMARY KEY,
    sc_name         TEXT                          UNIQUE NOT NULL,
    sc_description  TEXT                          NOT NULL
);

CREATE TABLE Systems.SystemInstances (
    si_id           INTEGER                       PRIMARY KEY,
    si_name         TEXT                          UNIQUE NOT NULL,
    si_description  TEXT                          NOT NULL,
    sc_id           INTEGER                       NOT NULL REFERENCES Systems.SystemClasses
);
