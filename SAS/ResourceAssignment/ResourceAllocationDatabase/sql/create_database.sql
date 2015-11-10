-- Copied from JR's script, maybe we should do this?
-- DROP DATABASE IF EXISTS resourceallocation;
-- CREATE DATABASE resourceallocation
--   WITH OWNER = renting
--       ENCODING = 'UTF8'
--       TABLESPACE = pg_default
--       LC_COLLATE = 'en_US.UTF-8'
--       LC_CTYPE = 'en_US.UTF-8'
--       CONNECTION LIMIT = -1;
CREATE SCHEMA virtual_instrument;
CREATE SCHEMA resource_monitoring;
CREATE SCHEMA resource_allocation;

-- USE resourceassignment;?

BEGIN;

-- This is insanity, but works, order needs to be the reverse of the CREATE TABLE statements
DROP TABLE IF EXISTS resource_monitoring.resource_group_availability;
DROP TABLE IF EXISTS resource_monitoring.resource_availability;
DROP TABLE IF EXISTS resource_monitoring.resource_capacity;
DROP TABLE IF EXISTS resource_allocation.resource_claim;
DROP TABLE IF EXISTS resource_allocation.resource_claim_status;
DROP TABLE IF EXISTS resource_allocation.task;
DROP TABLE IF EXISTS resource_allocation.specification;
DROP TABLE IF EXISTS resource_allocation.task_type;
DROP TABLE IF EXISTS resource_allocation.task_status;
DROP TABLE IF EXISTS virtual_instrument.resource_group_to_resource_group;
DROP TABLE IF EXISTS virtual_instrument.resource_to_resource_group;
DROP TABLE IF EXISTS virtual_instrument.resource_group;
DROP TABLE IF EXISTS virtual_instrument.resource_group_type;
DROP TABLE IF EXISTS virtual_instrument.resource;
DROP TABLE IF EXISTS virtual_instrument.resource_type;
DROP TABLE IF EXISTS virtual_instrument.unit;
-- Would like to use this instead, but I can not get it to do something useful: SET CONSTRAINTS ALL DEFERRED;

CREATE TABLE virtual_instrument.unit (
  id serial NOT NULL,
  units text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE unit
  OWNER TO renting;

CREATE TABLE virtual_instrument.resource_type (
  id serial NOT NULL,
  name text NOT NULL,
  unit_id integer NOT NULL REFERENCES unit DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_type
  OWNER TO renting;

CREATE TABLE virtual_instrument.resource (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL REFERENCES resource_type DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource
  OWNER TO renting;

CREATE TABLE virtual_instrument.resource_group_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_type
  OWNER TO renting;

CREATE TABLE virtual_instrument.resource_group (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL REFERENCES resource_group_type DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group
  OWNER TO renting;

CREATE TABLE virtual_instrument.resource_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL REFERENCES resource DEFERRABLE INITIALLY IMMEDIATE,
  parent_id integer NOT NULL REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_to_resource_group
  OWNER TO renting;

CREATE TABLE virtual_instrument.resource_group_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  parent_id integer REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_to_resource_group
  OWNER TO renting;

CREATE TABLE resource_allocation.task_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task_status
  OWNER TO renting;

CREATE TABLE resource_allocation.task_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task_type
  OWNER TO renting;

CREATE TABLE resource_allocation.specification (
  id serial NOT NULL,
  starttime timestamp,
  endtime timestamp,
  content text,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE specification
  OWNER TO renting;

CREATE TABLE resource_allocation.task (
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

CREATE TABLE resource_allocation.resource_claim_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_claim_status
  OWNER TO renting;

CREATE TABLE resource_allocation.resource_claim (
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

CREATE TABLE resource_monitoring.resource_capacity (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES resource DEFERRABLE INITIALLY IMMEDIATE,
  available bigint NOT NULL,
  total bigint NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_capacity
  OWNER TO renting;

CREATE TABLE resource_monitoring.resource_availability (
  id serial NOT NULL,
  resource_id integer NOT NULL REFERENCES resource DEFERRABLE INITIALLY IMMEDIATE,
  available bool NOt NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_availability
  OWNER TO renting;

CREATE TABLE resource_monitoring.resource_group_availability (
  id serial NOT NULL,
  resource_group_id integer NOT NULL REFERENCES resource_group DEFERRABLE INITIALLY IMMEDIATE,
  available bool NOt NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_availability
  OWNER TO renting;

COMMIT;
