-- Copied from JR's script, maybe we should do this?
-- DROP DATABASE IF EXISTS resourceassignment;
-- CREATE DATABASE resourceassignment
--   WITH OWNER = resourceassignment
--       ENCODING = 'UTF8'
--       TABLESPACE = pg_default
--       LC_COLLATE = 'en_US.UTF-8'
--       LC_CTYPE = 'en_US.UTF-8'
--       CONNECTION LIMIT = -1;

-- psql resourceassignment -U resourceassignment -f create_database.sql -W
CREATE SCHEMA virtual_instrument;
CREATE SCHEMA resource_monitoring;
CREATE SCHEMA resource_allocation;

BEGIN;

-- This is insanity, but works, order needs to be the reverse of the CREATE TABLE statements
DROP VIEW IF EXISTS resource_allocation.task_view CASCADE;
DROP VIEW IF EXISTS resource_allocation.resource_claim_view CASCADE;
DROP TABLE IF EXISTS resource_allocation.config CASCADE;
DROP TABLE IF EXISTS resource_monitoring.resource_group_availability CASCADE;
DROP TABLE IF EXISTS resource_monitoring.resource_availability CASCADE;
DROP TABLE IF EXISTS resource_monitoring.resource_capacity CASCADE;
DROP TABLE IF EXISTS resource_allocation.resource_claim CASCADE;
DROP TABLE IF EXISTS resource_allocation.resource_claim_status CASCADE;
DROP TABLE IF EXISTS resource_allocation.claim_session CASCADE;
DROP TABLE IF EXISTS resource_allocation.task CASCADE;
DROP TABLE IF EXISTS resource_allocation.specification CASCADE;
DROP TABLE IF EXISTS resource_allocation.task_type CASCADE;
DROP TABLE IF EXISTS resource_allocation.task_status CASCADE;
DROP TABLE IF EXISTS virtual_instrument.resource_group_to_resource_group CASCADE;
DROP TABLE IF EXISTS virtual_instrument.resource_to_resource_group CASCADE;
DROP TABLE IF EXISTS virtual_instrument.resource_group CASCADE;
DROP TABLE IF EXISTS virtual_instrument.resource_group_type CASCADE;
DROP TABLE IF EXISTS virtual_instrument.resource CASCADE;
DROP TABLE IF EXISTS virtual_instrument.resource_type CASCADE;
DROP TABLE IF EXISTS virtual_instrument.unit CASCADE;
-- Would like to use this instead, but I can not get it to do something useful: SET CONSTRAINTS ALL DEFERRED;

CREATE TABLE virtual_instrument.unit (
  id serial NOT NULL,
  units text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE virtual_instrument.unit
  OWNER TO resourceassignment;

CREATE TABLE virtual_instrument.resource_type (
  id serial NOT NULL,
  name text NOT NULL,
  unit_id integer NOT NULL REFERENCES virtual_instrument.unit DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE virtual_instrument.resource_type
  OWNER TO resourceassignment;

CREATE TABLE virtual_instrument.resource (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL REFERENCES virtual_instrument.resource_type ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE virtual_instrument.resource
  OWNER TO resourceassignment;

CREATE TABLE virtual_instrument.resource_group_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE virtual_instrument.resource_group_type
  OWNER TO resourceassignment;

CREATE TABLE virtual_instrument.resource_group (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL REFERENCES virtual_instrument.resource_group_type ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE virtual_instrument.resource_group
  OWNER TO resourceassignment;

CREATE TABLE virtual_instrument.resource_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL REFERENCES virtual_instrument.resource ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  parent_id integer NOT NULL REFERENCES virtual_instrument.resource_group ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE virtual_instrument.resource_to_resource_group
  OWNER TO resourceassignment;

CREATE TABLE virtual_instrument.resource_group_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL REFERENCES virtual_instrument.resource_group ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  parent_id integer REFERENCES virtual_instrument.resource_group ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE virtual_instrument.resource_group_to_resource_group
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.task_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.task_status
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.task_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.task_type
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.specification (
  id serial NOT NULL,
  starttime timestamp,
  endtime timestamp,
  content text,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.specification
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.task (
  id serial NOT NULL,
  mom_id integer,
  otdb_id integer,
  status_id integer NOT NULL REFERENCES resource_allocation.task_status DEFERRABLE INITIALLY IMMEDIATE,
  type_id integer NOT NULL REFERENCES resource_allocation.task_type DEFERRABLE INITIALLY IMMEDIATE,
  specification_id integer NOT NULL REFERENCES resource_allocation.specification DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.task
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.claim_session (
  id serial NOT NULL,
  username text NOT NULL,
  user_id integer NOT NULL,
  starttime timestamp NOT NULL,
  token text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.claim_session
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.resource_claim_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.resource_claim_status
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.resource_claim (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES virtual_instrument.resource ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  task_id integer NOT NULL REFERENCES resource_allocation.task ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  starttime timestamp NOT NULL,
  endtime timestamp NOT NULL,
  status_id integer NOT NULL REFERENCES resource_allocation.resource_claim_status DEFERRABLE INITIALLY IMMEDIATE,
  session_id integer NOT NULL REFERENCES resource_allocation.claim_session DEFERRABLE INITIALLY IMMEDIATE,
  claim_size bigint NOT NULL,
  nr_of_parts int NOT NULL DEFAULT 1,
  username text,
  user_id integer,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.resource_claim
  OWNER TO resourceassignment;

CREATE TABLE resource_monitoring.resource_capacity (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES virtual_instrument.resource ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  available bigint NOT NULL,
  total bigint NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_monitoring.resource_capacity
  OWNER TO resourceassignment;

CREATE TABLE resource_monitoring.resource_availability (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES virtual_instrument.resource ON DELETE CASCADE DEFERRABLE INITIALLY IMMEDIATE,
  available bool NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_monitoring.resource_availability
  OWNER TO resourceassignment;

CREATE TABLE resource_monitoring.resource_group_availability (
  id serial NOT NULL,
  resource_group_id integer NOT NULL REFERENCES virtual_instrument.resource_group DEFERRABLE INITIALLY IMMEDIATE,
  available bool NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_monitoring.resource_group_availability
  OWNER TO resourceassignment;

CREATE TABLE resource_allocation.config (
  id serial NOT NULL,
  name text NOT NULL,
  value text,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_allocation.config
  OWNER TO resourceassignment;

-- VIEWS ----------------------------------------------

CREATE OR REPLACE VIEW resource_allocation.task_view AS
 SELECT t.id, t.mom_id, t.otdb_id, t.status_id, t.type_id, t.specification_id,
    ts.name AS status, tt.name AS type, s.starttime, s.endtime
   FROM resource_allocation.task t
   JOIN resource_allocation.task_status ts ON ts.id = t.status_id
   JOIN resource_allocation.task_type tt ON tt.id = t.type_id
   JOIN resource_allocation.specification s ON s.id = t.specification_id;
ALTER TABLE resource_allocation.task_view
  OWNER TO resourceassignment;
COMMENT ON VIEW resource_allocation.task_view
  IS 'plain view on task table including task_status.name task_type.name specification.starttime and specification.endtime';


CREATE OR REPLACE VIEW resource_allocation.resource_claim_view AS
 SELECT rc.id, rc.resource_id, rc.task_id, rc.starttime, rc.endtime,
    rc.status_id, rc.session_id, rc.claim_size, rc.username, rc.user_id,
    rcs.name AS status
   FROM resource_allocation.resource_claim rc
   JOIN resource_allocation.resource_claim_status rcs ON rcs.id = rc.status_id;
ALTER TABLE resource_allocation.resource_claim_view
  OWNER TO resourceassignment;
COMMENT ON VIEW resource_allocation.resource_claim_view
  IS 'plain view on resource_claim table, including resource_claim_status.name';


COMMIT;
