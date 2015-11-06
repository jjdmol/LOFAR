-- Copied from JR's script, maybe we should do this?
-- DROP DATABASE IF EXISTS resourceallocation;
-- CREATE DATABASE resourceallocation
--   WITH OWNER = renting
--       ENCODING = 'UTF8'
--       TABLESPACE = pg_default
--       LC_COLLATE = 'en_US.UTF-8'
--       LC_CTYPE = 'en_US.UTF-8'
--       CONNECTION LIMIT = -1;
--CREATE SCHEMA resourceallocation;
--SET SCHEMA 'resourceallocation';

-- USE resourceassignment;

BEGIN;

-- This is insanity, but will hopefully work
DROP TABLE IF EXISTS resource_group_availability;
DROP TABLE IF EXISTS resource_availability;
DROP TABLE IF EXISTS resource_capacity;
DROP TABLE IF EXISTS resource_claim;
DROP TABLE IF EXISTS resource_claim_status;
DROP TABLE IF EXISTS task;
DROP TABLE IF EXISTS specification;
DROP TABLE IF EXISTS task_type;
DROP TABLE IF EXISTS task_status;
DROP TABLE IF EXISTS resource_group_to_resource_group;
DROP TABLE IF EXISTS resource_to_resource_group;
DROP TABLE IF EXISTS resource_group;
DROP TABLE IF EXISTS resource_group_type;
DROP TABLE IF EXISTS resource;
DROP TABLE IF EXISTS resource_type;
DROP TABLE IF EXISTS unit;
-- I can not get this to do something useful instead: SET CONSTRAINTS ALL DEFERRED;

CREATE TABLE unit (
  id serial NOT NULL,
  units text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE unit
  OWNER TO renting;

CREATE TABLE resource_type (
  id serial NOT NULL,
  name text NOT NULL,
  unit_id integer NOT NULL REFERENCES unit DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_type
  OWNER TO renting;

CREATE TABLE resource (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL REFERENCES resource_type DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource
  OWNER TO renting;

CREATE TABLE resource_group_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_type
  OWNER TO renting;

CREATE TABLE resource_group (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL REFERENCES resource_group_type DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group
  OWNER TO renting;

CREATE TABLE resource_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL REFERENCES resource DEFERRABLE INITIALLY IMMEDIATE,
  parent_id integer NOT NULL REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_to_resource_group
  OWNER TO renting;

CREATE TABLE resource_group_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  parent_id integer REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_to_resource_group
  OWNER TO renting;

CREATE TABLE task_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task_status
  OWNER TO renting;

CREATE TABLE task_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task_type
  OWNER TO renting;

CREATE TABLE specification (
  id serial NOT NULL,
  starttime timestamp,
  endtime timestamp,
  content text,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE specification
  OWNER TO renting;

CREATE TABLE task (
  id serial NOT NULL,
  mom_id integer,
  otdb_id integer,
  status_id integer NOT NULL REFERENCES task_status DEFERRABLE INITIALLY IMMEDIATE,
  type_id integer NOT NULL REFERENCES task_type DEFERRABLE INITIALLY IMMEDIATE,
  specification_id integer NOT NULL REFERENCES specification DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task
  OWNER TO renting;

CREATE TABLE resource_claim_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_claim_status
  OWNER TO renting;

CREATE TABLE resource_claim (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES resource DEFERRABLE INITIALLY IMMEDIATE,
  task_id integer NOT NULL REFERENCES task DEFERRABLE INITIALLY IMMEDIATE, -- ON DELETE CASCADE,
  starttime timestamp NOT NULL,
  endtime timestamp NOT NULL,
  status_id integer NOT NULL REFERENCES resource_claim_status DEFERRABLE INITIALLY IMMEDIATE,
  claim_endtime timestamp NOT NULL,
  claim_size bigint NOT NULL,
  username text,
  user_id integer,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_claim
  OWNER TO renting;

CREATE TABLE resource_capacity (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES resource DEFERRABLE INITIALLY IMMEDIATE,
  available bigint NOT NULL,
  total bigint NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_capacity
  OWNER TO renting;

CREATE TABLE resource_availability (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES resource DEFERRABLE INITIALLY IMMEDIATE,
  available bool NOt NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_availability
  OWNER TO renting;

CREATE TABLE resource_group_availability (
  id serial NOT NULL,
  resource_group_id integer NOT NULL REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  available bool NOt NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_availability
  OWNER TO renting;

COMMIT;
