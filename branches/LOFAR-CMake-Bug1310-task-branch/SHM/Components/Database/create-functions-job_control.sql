-- JOB STATES
--
-- 0 = PENDING
-- 1 = RUNNING
-- 2 = STOPPED

-- LOG LEVELS
--
-- 0 = LOG_EMERG   # system is unusable
-- 1 = LOG_ALERT   # action must be taken immediately
-- 2 = LOG_CRIT    # critical conditions
-- 3 = LOG_ERR     # error conditions
-- 4 = LOG_WARNING # warning conditions
-- 5 = LOG_NOTICE  # normal but significant condition
-- 6 = LOG_INFO    # informational
-- 7 = LOG_DEBUG   # debug-level messages


DROP FUNCTION job_control.reschedule(job job_control.queue);
DROP FUNCTION job_control.valid(job_id BIGINT, job_ticket_no BIGINT);
DROP FUNCTION job_control.update_queue();
DROP FUNCTION job_control.bind();
DROP FUNCTION job_control.unbind(job_id BIGINT, job_ticket_no BIGINT);
DROP FUNCTION job_control.stop(job_id BIGINT);
DROP FUNCTION job_control.start(job_id BIGINT);
DROP FUNCTION job_control.reset_watchdog(job_id BIGINT, job_ticket_no BIGINT);
DROP FUNCTION job_control.log(_level INT, _pid INT, _context TEXT, _line_no INT, _message TEXT, _job_id BIGINT, _job_ticket_no BIGINT, _source INET);
DROP FUNCTION job_control.conditional_commit(job_id BIGINT, job_ticket_no BIGINT);


CREATE OR REPLACE FUNCTION job_control.reschedule(job job_control.queue) RETURNS VOID AS $$
    DECLARE
        new_scheduled_time TIMESTAMP WITH TIME ZONE;
    BEGIN

        IF job.period IS NULL THEN
            -- single job -> set state to STOPPED
            UPDATE job_control.queue
                SET state = 2, client = NULL, time_of_bind = NULL, watchdog = NULL
                WHERE id = job.id;

        ELSEIF EXTRACT(EPOCH FROM job.period) = 0 THEN
            -- persistent job -> schedule for immediate execution
            UPDATE job_control.queue
                SET scheduled_time = now(), ticket_no = job.ticket_no + 1, state = 0, client = NULL, time_of_bind = NULL, watchdog = NULL 
                WHERE id = job.id;

        ELSE
            -- periodic job -> try to schedule for next period, but schedule for immediate
            -- execution if the start of the next period has already passed.
            IF (job.scheduled_time IS NOT NULL) AND (job.scheduled_time + job.period > now()) THEN
                new_scheduled_time := job.scheduled_time + job.period;
            ELSIF (job.time_of_bind IS NOT NULL) AND (job.time_of_bind + job.period > now()) THEN
                new_scheduled_time := job.time_of_bind + job.period;
            ELSE
                new_scheduled_time := now();
            END IF;

            UPDATE job_control.queue
                SET scheduled_time = new_scheduled_time, ticket_no = job.ticket_no + 1, state = 0, client = NULL, time_of_bind = NULL, watchdog = NULL
                WHERE id = job.id;
        END IF;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.valid(job_id BIGINT, job_ticket_no BIGINT) RETURNS BOOLEAN AS $$
    DECLARE
        job job_control.queue%ROWTYPE;
    BEGIN
        -- block other transactions that try to SELECT FOR UPDATE
        -- the same row.
        SELECT * INTO job
            FROM job_control.queue
            WHERE id = job_id
            AND ticket_no = job_ticket_no
            AND state = 1
            FOR UPDATE;

        IF FOUND THEN
            RETURN (job.wd_timeout IS NULL) OR (now() - job.watchdog < job.wd_timeout);
        ELSE
            RETURN FALSE;
        END IF;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.update_queue() RETURNS VOID AS $$
    DECLARE
        job job_control.queue%ROWTYPE;
    BEGIN
        -- check watchdog of all running jobs.
        FOR job IN
            -- block other transactions that try to SELECT FOR UPDATE
            -- the same row.
            SELECT *
                FROM job_control.queue
                WHERE state = 1
                AND (wd_timeout IS NOT NULL)
                AND (now() - watchdog) >= wd_timeout
                FOR UPDATE
        LOOP
            PERFORM job_control.log(4, -1, 'server::update_queue()', -1, 'job timed out', job.id, job.ticket_no, inet_server_addr());
            PERFORM job_control.reschedule(job);
        END LOOP;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.bind() RETURNS RECORD AS $$
    DECLARE
        result RECORD;
    BEGIN
        -- SELECT FOR UPDATE is used to lock the affected row. this prevents two
        -- clients from binding to the same job. it is stated explicitly in the
        -- PostgreSQL documentation that SELECT FOR UPDATE will discard rows that
        -- no longer match the WHERE clause when it unblocks. (However, it is not
        -- stated if it will also include any rows that did not match the WHERE
        -- clause previously, but do match it at the moment the statement unblocks.)
        --
        -- see http://www.postgresql.org/docs/8.1/interactive/sql-select.html
        --

        -- find the oldest pending job in the queue and block other transactions
        -- that try to SELECT FOR UPDATE the same row.
        SELECT id, ticket_no INTO result
            FROM job_control.queue
            WHERE state = 0
            AND scheduled_time <= now()
            ORDER BY scheduled_time ASC
            LIMIT 1
            FOR UPDATE;

        -- FOUND is not set by SELECT (and therefore by assignments), only by SELECT INTO.
        IF FOUND THEN
            -- bind job
            UPDATE job_control.queue
            SET state = 1, client = inet_client_addr(), time_of_bind = now(), watchdog = now()
            WHERE id = result.id;

            -- log state change
            PERFORM job_control.log(5, -1, 'server::bind()', -1, 'job was bound', result.id, result.ticket_no, inet_server_addr());
        END IF;

        RETURN result;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.unbind(job_id BIGINT, job_ticket_no BIGINT) RETURNS BOOLEAN AS $$
    DECLARE
        job job_control.queue%ROWTYPE;
    BEGIN
        -- block other transactions that try to SELECT FOR UPDATE
        -- the same row.
        SELECT * INTO job
            FROM job_control.queue 
            WHERE id = job_id
            AND ticket_no = job_ticket_no
            AND state = 1
            AND client = inet_client_addr()
            FOR UPDATE;

        IF FOUND THEN
            -- unbind and reschedule
            PERFORM job_control.reschedule(job);

            -- log state change
            PERFORM job_control.log(5, -1, 'server::unbind()', -1, 'job was unbound', job.id, job.ticket_no, inet_server_addr());

            RETURN TRUE;
        ELSE
            RETURN FALSE;
        END IF;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.stop(job_id BIGINT) RETURNS BOOLEAN AS $$
    DECLARE
        job job_control.queue%ROWTYPE;
    BEGIN
        -- block other transactions that try to SELECT FOR UPDATE
        -- the same row.
        SELECT * INTO job 
            FROM job_control.queue 
            WHERE id = job_id
            AND state <> 2 
            FOR UPDATE;

        IF FOUND THEN
            -- set state to STOPPED
            UPDATE job_control.queue
                SET scheduled_time = NULL, state = 2, client = NULL, time_of_bind = NULL, watchdog = NULL
                WHERE id = job_id;

            -- log state change
            PERFORM job_control.log(5, -1, 'server::stop()', -1, 'job was stopped', job.id, job.ticket_no, inet_server_addr());

            RETURN TRUE;
        ELSE
            RETURN FALSE;
        END IF;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.start(job_id BIGINT) RETURNS BOOLEAN AS $$
    DECLARE
        job job_control.queue%ROWTYPE;
    BEGIN
        -- block other transactions that try to SELECT FOR UPDATE
        -- the same row.
        SELECT * INTO job 
            FROM job_control.queue 
            WHERE id = job_id
            AND state = 2
            FOR UPDATE;

        IF FOUND THEN
            -- schedule job for immediate execution
            UPDATE job_control.queue 
                SET scheduled_time = now(), ticket_no = job.ticket_no + 1, state = 0, client = NULL, time_of_bind = NULL, watchdog = NULL
                WHERE id = job.id;

            -- log state change
            PERFORM job_control.log(5, -1, 'server::start()', -1, 'job was started', job.id, job.ticket_no + 1, inet_server_addr());

            RETURN TRUE;
        ELSE
            RETURN FALSE;
        END IF;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.reset_watchdog(job_id BIGINT, job_ticket_no BIGINT) RETURNS BOOLEAN AS $$
    DECLARE
        job job_control.queue%ROWTYPE;
    BEGIN
        -- block other transactions that try to SELECT FOR UPDATE
        -- the same row.
        SELECT * INTO job
            FROM job_control.queue 
            WHERE id = job_id
            AND ticket_no = job_ticket_no
            AND state = 1
            AND client = inet_client_addr()
            FOR UPDATE;

        IF FOUND THEN
            -- check for timeout
            IF (job.wd_timeout IS NOT NULL) AND (now() - job.watchdog >= job.wd_timeout) THEN
                PERFORM job_control.reschedule(job);
                PERFORM job_control.log(4, -1, 'server::reset_watchdog()', -1, 'watchdog reset failed; job already timed out', job.id, job.ticket_no, inet_server_addr());
                RETURN FALSE;
            END IF;

            -- reset watchdog
            UPDATE job_control.queue
                SET watchdog = now()
                WHERE id = job_id;

            RETURN TRUE;
        ELSE
            RETURN FALSE;
        END IF;
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.log(_level INT, _pid INT, _context TEXT, _line_no INT, _message TEXT, _job_id BIGINT, _job_ticket_no BIGINT, _source INET) RETURNS VOID AS $$
    BEGIN
        INSERT INTO job_control.log(timestamp, level, pid, context, line_no, message, job_id, job_ticket_no, source)
            VALUES (now(), _level, _pid, _context, _line_no, _message, _job_id, _job_ticket_no, _source);
    END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION job_control.conditional_commit(job_id BIGINT, job_ticket_no BIGINT) RETURNS VOID AS $$
    DECLARE
        job job_control.queue%ROWTYPE;
    BEGIN
        -- block other transactions that try to SELECT FOR UPDATE
        -- the same row.
        SELECT * INTO job
            FROM job_control.queue
            WHERE id = job_id
            AND ticket_no = job_ticket_no
            AND state = 1
            FOR UPDATE;

        IF FOUND AND ((job.wd_timeout IS NULL) OR (now() - job.watchdog < job.wd_timeout)) THEN
            RETURN;
        ELSE
            RAISE EXCEPTION 'Invalid ticket number or job has timed out: commit not allowed';
        END IF;
    END;
$$ LANGUAGE plpgsql;
