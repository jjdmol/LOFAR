-- This file creates the databasetable for class ExamplePL
--
-- WARNING: THE TABLE IS DROPPED BEFORE CREATION!!!
--
drop table ExamplePL;
create table ExamplePL (
	ObjId		BIGINT		NOT NULL UNIQUE PRIMARY KEY,
	Owner		BIGINT		NOT	NULL,
	VersionNr	INTEGER		NOT NULL,
	ID	INTEGER,
	TAG	INTEGER,
	SEQNR	BIGINT,
	DATA	TEXT
);

CREATE TRIGGER ExamplePLUpdateVersionNr BEFORE INSERT OR UPDATE ON ExamplePL
FOR EACH ROW EXECUTE PROCEDURE UpdateVersionNr();
