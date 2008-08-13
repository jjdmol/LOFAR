-- This file creates the databasetable for class TestBidirectional
--
-- WARNING: THE TABLE IS DROPPED BEFORE CREATION!!!
--
drop table TestBidirectional;
create table TestBidirectional (
	ObjId		BIGINT		NOT NULL UNIQUE PRIMARY KEY,
	Owner		BIGINT		NOT	NULL,
	VersionNr	INTEGER		NOT NULL,
	ID	INTEGER,
	TAG	INTEGER,
	SEQNR	BIGINT,
	DATA	TEXT
);

CREATE TRIGGER ExamplePLUpdateVersionNr BEFORE INSERT OR UPDATE ON TestBidirectional
FOR EACH ROW EXECUTE PROCEDURE UpdateVersionNr();
