-- DROP DATABASE IF EXISTS resourceallocation;
-- CREATE DATABASE resourceallocation
--   WITH OWNER = renting
 --      ENCODING = 'UTF8'
--       TABLESPACE = pg_default
--       LC_COLLATE = 'en_US.UTF-8'
--       LC_CTYPE = 'en_US.UTF-8'
--       CONNECTION LIMIT = -1;
--CREATE SCHEMA resourceallocation;
--SET SCHEMA 'resourceallocation';

-- USE resourceassignment;

BEGIN;
DROP TABLE IF EXISTS resource;
CREATE TABLE resource (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource
  OWNER TO renting;

-- Could this be a CREATE TYPE?
DROP TABLE IF EXISTS resource_type;
CREATE TABLE resource_type (
  id serial NOT NULL,
  name text NOT NULL,
  unit_id integer NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_type
  OWNER TO renting;

-- Could this be a CREATE TYPE?
DROP TABLE IF EXISTS unit;
CREATE TABLE unit (
  id serial NOT NULL,
  units text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE unit
  OWNER TO renting;

DROP TABLE IF EXISTS resource_to_resource_group;
CREATE TABLE resource_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL,
  parent_id integer NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_to_resource_group
  OWNER TO renting;

DROP TABLE IF EXISTS resource_group;
CREATE TABLE resource_group (
  id serial NOT NULL,
  name text NOT NULL,
  type_id integer NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group
  OWNER TO renting;

-- Could this be a CREATE TYPE?
DROP TABLE IF EXISTS resource_group_type;
CREATE TABLE resource_group_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_type
  OWNER TO renting;

DROP TABLE IF EXISTS resource_group_to_resource_group;
CREATE TABLE resource_group_to_resource_group (
  id serial NOT NULL,
  child_id integer NOT NULL,
  parent_id integer,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_to_resource_group
  OWNER TO renting;

DROP TABLE IF EXISTS resource_claim;
CREATE TABLE resource_claim (
  id serial NOT NULL,
  resource_id integer NOT NULL,
  task_id integer NOT NULL,
  starttime timestamp NOT NULL,
  endtime timestamp NOT NULL,
  status_id integer NOT NULL,
  claim_endtime timestamp NOT NULL,
  claim_size bigint NOT NULL,
  username text,
  user_id integer,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_claim
  OWNER TO renting;

DROP TABLE IF EXISTS resource_claim_status;
CREATE TABLE resource_claim_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_claim_status
  OWNER TO renting;

DROP TABLE IF EXISTS task;
CREATE TABLE task (
  id serial NOT NULL,
  mom_id integer,
  otdb_id integer,
  status_id integer NOT NULL,
  type_id integer NOT NULL,
  specification_id integer NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task
  OWNER TO renting;

DROP TABLE IF EXISTS task_status;
CREATE TABLE task_status (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task_status
  OWNER TO renting;

DROP TABLE IF EXISTS task_type;
CREATE TABLE task_type (
  id serial NOT NULL,
  name text NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE task_type
  OWNER TO renting;

DROP TABLE IF EXISTS specification;
CREATE TABLE specification (
  id serial NOT NULL,
  content text,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE specification
  OWNER TO renting;

DROP TABLE IF EXISTS resource_capacity;
CREATE TABLE resource_capacity (
  id serial NOT NULL,
  resource_id integer NOT NULL,
  available bigint NOT NULL,
  total bigint NOT NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_capacity
  OWNER TO renting;

DROP TABLE IF EXISTS resource_availability;
CREATE TABLE resource_availability (
  id serial NOT NULL,
  resource_id integer NOT NULL,
  available bool NOt NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_availability
  OWNER TO renting;

DROP TABLE IF EXISTS resource_group_availability;
CREATE TABLE resource_group_availability (
  id serial NOT NULL,
  resource_group_id integer NOT NULL,
  available bool NOt NULL,
  PRIMARY KEY (id)
) WITH (OIDS=FALSE);
ALTER TABLE resource_group_availability
  OWNER TO renting;

COMMIT;
