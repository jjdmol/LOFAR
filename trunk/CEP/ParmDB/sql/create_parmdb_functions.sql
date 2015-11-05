CREATE TYPE xp.iface_parm_value AS
(
    name            TEXT,
    type            TEXT,
    expression      TEXT,
    constants       BYTEA,
    shape           BYTEA,
    mask            BYTEA,
    perturbation    DOUBLE PRECISION,
    pert_rel        BOOL,
    domain          BOX,
    coefficients    BYTEA
);

CREATE OR REPLACE FUNCTION xp.put_value(parm xp.iface_parm_value)
RETURNS VOID AS
$$
    BEGIN
        UPDATE xp.parameter
            SET type = parm.type,
                expression = parm.expression,
                constants = parm.constants,
                shape = parm.shape,
                mask = parm.mask,
                perturbation = parm.perturbation,
                pert_rel = parm.pert_rel,
                coefficients = parm.coefficients
            WHERE name = parm.name
            AND domain ~= parm.domain;
        
        IF NOT FOUND THEN
            LOCK TABLE xp.parameter;
            
            UPDATE xp.parameter
                SET type = parm.type,
                    expression = parm.expression,
                    constants = parm.constants,
                    shape = parm.shape,
                    mask = parm.mask,
                    perturbation = parm.perturbation,
                    pert_rel = parm.pert_rel,
                    coefficients = parm.coefficients
                WHERE name = parm.name
                AND domain ~= parm.domain;
            
            IF NOT FOUND THEN
                INSERT
                    INTO xp.parameter
                        (name,
                        type,
                        expression,
                        constants,
                        shape,
                        mask,
                        perturbation,
                        pert_rel,
                        coefficients,
                        domain)
                    VALUES
                        (parm.name,
                        parm.type,
                        parm.expression,
                        parm.constants,
                        parm.shape,
                        parm.mask,
                        parm.perturbation,
                        parm.pert_rel,
                        parm.coefficients,
                        parm.domain);
            END IF;
        END IF;
    END;
$$ LANGUAGE plpgsql;        


CREATE TYPE xp.iface_parm_default_value AS
(
    name            TEXT,
    type            TEXT,
    expression      TEXT,
    constants       BYTEA,
    shape           BYTEA,
    mask            BYTEA,
    coefficients    BYTEA,
    perturbation    DOUBLE PRECISION,
    pert_rel        BOOL
);


CREATE OR REPLACE FUNCTION
    xp.put_default_value(parm xp.iface_parm_default_value)
RETURNS VOID AS
$$
    BEGIN
        UPDATE xp.default_parameter
            SET type = parm.type,
                expression = parm.expression,
                constants = parm.constants,
                shape = parm.shape,
                mask = parm.mask,
                coefficients = parm.coefficients,
                perturbation = parm.perturbation,
                pert_rel = parm.pert_rel
            WHERE name = parm.name;
        
        IF NOT FOUND THEN
            LOCK TABLE xp.default_parameter;
            
            UPDATE xp.default_parameter
                SET type = parm.type,
                    expression = parm.expression,
                    constants = parm.constants,
                    shape = parm.shape,
                    mask = parm.mask,
                    coefficients = parm.coefficients,
                    perturbation = parm.perturbation,
                    pert_rel = parm.pert_rel
                WHERE name = parm.name;
            
            IF NOT FOUND THEN
                INSERT
                    INTO xp.default_parameter
                        (name,
                        type,
                        expression,
                        constants,
                        shape,
                        mask,
                        coefficients,
                        perturbation,
                        pert_rel)
                    VALUES
                        (parm.name,
                        parm.type,
                        parm.expression,
                        parm.constants,
                        parm.shape,
                        parm.mask,
                        parm.coefficients,
                        parm.perturbation,
                        parm.pert_rel);
            END IF;
        END IF;
    END;
$$ LANGUAGE plpgsql;        
