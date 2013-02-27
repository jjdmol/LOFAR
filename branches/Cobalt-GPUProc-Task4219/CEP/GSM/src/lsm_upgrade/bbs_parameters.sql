CREATE SEQUENCE "seq_bss" AS INTEGER;
CREATE TABLE barts.bbs_parameters (
    bss_src_id           INTEGER       NOT NULL DEFAULT next value for barts.seq_bss,
    catsrcid             INTEGER       NOT NULL,
    wenssm_catsrcid      INTEGER,
    wenssp_catsrcid      INTEGER,
    nvss_catsrcid        INTEGER,
    ra                   DOUBLE        NOT NULL,
    decl                 DOUBLE        NOT NULL,
    pa                   DOUBLE,
    minor                DOUBLE,
    major                DOUBLE,
    spectral_index_power SMALLINT      NOT NULL,
    spectral_index_0     DOUBLE        NOT NULL DEFAULT 0,
    spectral_index_1     DOUBLE        NOT NULL DEFAULT 0,
    spectral_index_2     DOUBLE        NOT NULL DEFAULT 0,
    x                    DOUBLE,
    y                    DOUBLE,
    z                    DOUBLE,
    catsrcname           VARCHAR(120)
);
--copy from bbs_parameters.dat into bbs_parameters delimiters ';';