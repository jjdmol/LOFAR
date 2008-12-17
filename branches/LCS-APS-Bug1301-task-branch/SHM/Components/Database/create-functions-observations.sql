
CREATE OR REPLACE FUNCTION Observations.InsertObservation(the_time TIMESTAMP WITH TIME ZONE, the_si_name TEXT, the_obs_name TEXT, the_value TEXT) RETURNS VOID AS $$
    DECLARE
        the_obs_id BIGINT;
    BEGIN

        the_obs_id := obs_id FROM Systems.SystemInstances NATURAL JOIN Observations.Observables WHERE si_name = the_si_name AND obs_name = the_obs_name;

        IF the_obs_id IS NULL THEN
            RAISE EXCEPTION 'InsertObservation(): attempt to insert Observation for unknown Observable.';
        END IF;

        INSERT INTO Observations.Observations(time, obs_id, value) VALUES(the_time, the_obs_id, the_value);

    END;
$$ LANGUAGE plpgsql;

--DROP FUNCTION Observations.GetAllSystemValuesOnTime(the_time TIMESTAMP, the_si_name TEXT);
--DROP TYPE Observations.GetAllSystemValuesOnTimeReturnType;

CREATE TYPE Observations.GetAllSystemValuesOnTimeReturnType AS (
    name  TEXT,
    value TEXT,
    unit  TEXT,
    age   FLOAT
);

CREATE OR REPLACE FUNCTION Observations.GetAllSystemValuesOnTime(the_time TIMESTAMP WITH TIME ZONE, the_si_name TEXT) RETURNS SETOF Observations.GetAllSystemValuesOnTimeReturnType AS $$
    DECLARE
        the_observable RECORD;
        result         Observations.GetAllSystemValuesOnTimeReturnType;
    BEGIN

        FOR the_observable IN
            SELECT obs_id, obs_name, obs_unit
                FROM Systems.SystemInstances NATURAL JOIN Observations.Observables
                WHERE si_name = the_si_name LOOP

            SELECT the_observable.obs_name, value, the_observable.obs_unit, EXTRACT(EPOCH FROM AGE(the_time, time)) INTO result
                FROM Observations.Observations
                WHERE obs_id = the_observable.obs_id AND time < the_time
                ORDER BY time DESC
                LIMIT 1;

            IF NOT FOUND THEN
                SELECT the_observable.obs_name, NULL, the_observable.obs_unit, NULL INTO result;
            END IF;

            RETURN NEXT result;

        END LOOP;
        RETURN;
    END;
$$ LANGUAGE plpgsql;

--
-- FUNCTION GetTimesWhenNameIsValue()
--

--CREATE OR REPLACE FUNCTION GetTimesWhenNameIsValue(the_name TEXT, the_value TEXT, t_start TIMESTAMP, t_end TIMESTAMP) RETURNS SETOF TIMESTAMP AS $$
--    DECLARE
--        the_name_id BIGINT;
--        evt         RECORD;
--    BEGIN
--        the_name_id := name_id FROM StatusVariables WHERE name = the_name;
--        IF the_name_id IS NOT NULL THEN
--            FOR evt IN SELECT time FROM StatusEvents WHERE t_start <= time AND time <= t_end AND the_name_id = name_id AND value = the_value ORDER BY time LOOP
--                RETURN NEXT evt.time;
--            END LOOP;
--        END IF;
--        RETURN;
--    END;
--$$ LANGUAGE plpgsql;

--
-- FUNCTION GetValuesOnTime()
--

--DROP FUNCTION GetValuesOnTime(TEXT, TIMESTAMP);
--DROP TYPE GetValuesOnTimeReturnType;


--CREATE OR REPLACE FUNCTION GetValuesOnTime(prefix TEXT, t_now TIMESTAMP) RETURNS SETOF GetValuesOnTimeReturnType AS $$
--    DECLARE
--        result GetValuesOnTimeReturnType;
--        var    StatusVariables%ROWTYPE;
--    BEGIN
--        FOR var IN SELECT * FROM StatusVariables WHERE name LIKE (prefix || '%') LOOP
--            SELECT INTO result time, var.name, value FROM StatusEvents WHERE time <= t_now AND name_id = var.name_id ORDER BY time DESC LIMIT 1;
--            IF FOUND THEN
--                RETURN NEXT result;
--            END IF;
--        END LOOP;
--        RETURN;
--    END;
--$$ LANGUAGE plpgsql;

--
-- FUNCTION ns1970()
--
-- Returns a 'TIME WITHOUT TIMEZONE' value, interpreted as a UTC timestamp,
-- as a number-of-nanoseconds-since-1970.
--

--CREATE OR REPLACE FUNCTION ns1970(ts TIMESTAMP WITH TIME ZONE) RETURNS BIGINT AS $$
--    BEGIN
--        RETURN CAST(EXTRACT(EPOCH FROM ts)*1e6 AS BIGINT)*1000;
--    END;
--$$ LANGUAGE plpgsql;
