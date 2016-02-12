-- create_blackboard_triggers.sql
--
-- Copyright (C) 2007
-- ASTRON (Netherlands Institute for Radio Astronomy)
-- P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
--
-- This file is part of the LOFAR software suite.
-- The LOFAR software suite is free software: you can redistribute it and/or
-- modify it under the terms of the GNU General Public License as published
-- by the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- The LOFAR software suite is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License along
-- with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
--
-- $Id$

CREATE OR REPLACE FUNCTION blackboard.notify_client() RETURNS TRIGGER AS
$$
    BEGIN
        EXECUTE 'NOTIFY ' || quote_ident(TG_NAME);
        RETURN NULL;
    END;
$$
LANGUAGE plpgsql;

CREATE TRIGGER modify_worker_register
    AFTER INSERT OR UPDATE OR DELETE ON blackboard.worker
    FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();

CREATE TRIGGER insert_command
    AFTER INSERT ON blackboard.command
    FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();

CREATE TRIGGER insert_result
    AFTER INSERT ON blackboard.result
    FOR EACH STATEMENT EXECUTE PROCEDURE blackboard.notify_client();

