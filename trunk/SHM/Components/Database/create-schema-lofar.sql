-- NOTE: Initialize the "Systems" schema before this one!

DROP SCHEMA Lofar CASCADE;

CREATE SCHEMA Lofar;

CREATE TABLE Lofar.MacInformationServers (
    si_id           INTEGER                       PRIMARY KEY REFERENCES Systems.SystemInstances ON DELETE CASCADE ON UPDATE CASCADE,
    si_name         TEXT                          NOT NULL,
    mis_address     INET                          NOT NULL,
    mis_port        INTEGER                       NOT NULL
);

-- NOTE: TIMESTAMP WITH TIME ZONE will store its timestamps in UTC, while the time zones
--       for TIMESTAMP WITHOUT TIME ZONE are basically unspecified. We prefer the former.

CREATE TABLE Lofar.SubbandStatistics (
    time            TIMESTAMP WITH TIME ZONE      NOT NULL,
    si_id           INTEGER                       REFERENCES Systems.SystemInstances ON DELETE CASCADE ON UPDATE CASCADE,
    rcu_id          INTEGER                       NOT NULL,
    -- meta-information
    rcu_settings    INTEGER                       NOT NULL,
    spectrum        DOUBLE PRECISION []           NOT NULL,
    median_power    DOUBLE PRECISION              NOT NULL,
    median_chan     INTEGER                               ,
    peak_power      DOUBLE PRECISION              NOT NULL,
    classification  TEXT []                                 DEFAULT '{"UNCLASSIFIED"}',
    -- constraints
    PRIMARY KEY(time, si_id, rcu_id)
);

-- NOTE: TIMESTAMP WITH TIME ZONE will store its timestamps in UTC, while the time zones
--       for TIMESTAMP WITHOUT TIME ZONE are basically unspecified. We prefer the former.

CREATE TABLE Lofar.AntennaCorrelationMatrices (
    time            TIMESTAMP  WITH TIME ZONE     NOT NULL,
    si_id           BIGINT                        REFERENCES Systems.SystemInstances ON DELETE CASCADE ON UPDATE CASCADE,
    -- meta-information
    rcu_settings    INTEGER []                    NOT NULL,
    subband         INTEGER []                    NOT NULL,
    acm_data        DOUBLE PRECISION []           NOT NULL,
    geo_loc         DOUBLE PRECISION []           DEFAULT '{0}',
    ant_coord       REAL []                       DEFAULT '{0}',
    classification  TEXT []                                 DEFAULT '{"UNCLASSIFIED"}',
    -- constraints
    PRIMARY KEY(time, si_id)
);

-- NOTE: Initialize the "Systems" schema before this one!

CREATE TABLE Lofar.RSPStatus (
    time            TIMESTAMP  WITH TIME ZONE     NOT NULL,
    si_id           BIGINT                        REFERENCES Systems.SystemInstances ON DELETE CASCADE ON UPDATE CASCADE,
    -- meta-information
    rsp_id              INTEGER           NOT NULL, 
    ADO_adc_offset_x    BIGINT []         NOT NULL, 
    ADO_adc_offset_y    BIGINT []         NOT NULL,
    BS_ext_count        BIGINT []         NOT NULL,
    BS_sample_offset    BIGINT []         NOT NULL,
    BS_slice_count      BIGINT []         NOT NULL,
    BS_sync_count       BIGINT []         NOT NULL,
    DIAG_ap_ri_errors   INTEGER []        NOT NULL,
    DIAG_cep_errors     INTEGER           NOT NULL,
    DIAG_interface      INTEGER           NOT NULL,
    DIAG_lcu_errors     INTEGER           NOT NULL,
    DIAG_mode           INTEGER           NOT NULL,
    DIAG_rcux_errors    INTEGER           NOT NULL,
    DIAG_rcuy_errors    INTEGER           NOT NULL,
    DIAG_ri_errors      INTEGER           NOT NULL,
    DIAG_serdes_errors  INTEGER           NOT NULL,
    ETH_last_error      INTEGER           NOT NULL,
    ETH_num_errors      BIGINT            NOT NULL,
    ETH_num_frames      BIGINT            NOT NULL,
    MEP_error           INTEGER           NOT NULL,
    MEP_seqnr           INTEGER           NOT NULL,
    RCU_num_overflow_x  BIGINT []         NOT NULL,
    RCU_num_overflow_y  BIGINT []         NOT NULL,
    RCU_pllx            INTEGER []        NOT NULL,
    RCU_plly            INTEGER []        NOT NULL,
    RSP_board_temps     INTEGER []        NOT NULL,
    RSP_board_volts     REAL []           NOT NULL,
    RSP_bp_clock        INTEGER           NOT NULL,
    RSU_apbp            INTEGER []        NOT NULL,
    RSU_error           INTEGER []        NOT NULL,
    RSU_image_type      INTEGER []        NOT NULL,
    RSU_ready           INTEGER []        NOT NULL, 
    RSU_trig            INTEGER []        NOT NULL,
    classification      TEXT[]                      DEFAULT '{UNCLASSIFIED}',
    -- constraints
    PRIMARY KEY(time, si_id, rsp_id)
);

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
