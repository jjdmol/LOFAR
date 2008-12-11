-- NOTE: Initialize the "Systems" schema before this one!

CREATE TABLE Lofar.Diagnoses (
    time            TIMESTAMP  WITH TIME ZONE     NOT NULL,
    si_id           BIGINT                        REFERENCES Systems.SystemInstances ON DELETE CASCADE ON UPDATE CASCADE,
    num_faults      INT,
    datum_epoch     TIMESTAMP  WITH TIME ZONE [],
    -- meta-information
    component       TEXT [],
    url             TEXT [],
    state           SMALLINT [],
    confidence      INT [],
    reported_to_MIS BOOL,
    -- constraints
    PRIMARY KEY(time, si_id)
);
