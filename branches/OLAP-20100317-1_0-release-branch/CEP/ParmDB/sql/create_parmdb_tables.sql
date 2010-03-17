CREATE TABLE xp.parameter
(
    id              SERIAL              PRIMARY KEY,
    name            TEXT                NOT NULL,
    type            TEXT                NOT NULL,
    expression      TEXT                NOT NULL,
    constants       BYTEA               ,
    shape           BYTEA               NOT NULL,
    mask            BYTEA               ,
    perturbation    DOUBLE PRECISION    DEFAULT 1e-6,
    pert_rel        BOOL                DEFAULT TRUE,
    domain          BOX                 NOT NULL,
    coefficients    BYTEA               NOT NULL
);


CREATE TABLE xp.default_parameter
(
    id              SERIAL              PRIMARY KEY,
    name            TEXT                UNIQUE
                                        NOT NULL,
    type            TEXT                NOT NULL,
    expression      TEXT                NOT NULL,
    constants       BYTEA               ,
    shape           BYTEA               NOT NULL,
    mask            BYTEA               ,
    coefficients    BYTEA               NOT NULL,
    perturbation    DOUBLE PRECISION    DEFAULT 1e-6,
    pert_rel        BOOL                DEFAULT TRUE
);

CREATE INDEX parameter_domain_idx ON xp.parameter USING gist(domain);
