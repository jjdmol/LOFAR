-- This file was generated by genLCSsql v1.0 on Thu Nov 13 14:25:25 CET 2003
-- with the command: genLCSsql MyModule
-- from the directory: /export/home/loose/temp
--
-- EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
-- IT MIGHT BE OVERWRITTEN THE NEXT TIME YOU RUN genLCSsql
--
-- This file creates the databasetable for class A
--
-- WARNING: THE TABLE IS DROPPED BEFORE CREATION!!!
--
drop table A;
create table A (
	ObjId		BIGINT		NOT NULL UNIQUE PRIMARY KEY,
	Owner		BIGINT		NOT NULL,
	VersionNr	INTEGER		NOT NULL,
	ItsInt		INTEGER,
	ItsDouble	DOUBLE PRECISION,
	ItsString	TEXT
);

CREATE TRIGGER A_update_versionnr BEFORE INSERT OR UPDATE ON A
FOR EACH ROW EXECUTE PROCEDURE update_versionnr();

drop table B;
create table B (
	ObjId		BIGINT		NOT NULL UNIQUE PRIMARY KEY,
	Owner		BIGINT		NOT NULL,
	VersionNr	INTEGER		NOT NULL,
	ItsBool		SMALLINT,
	ItsShort	SMALLINT,
	ItsFloat	REAL,
	ItsString	TEXT
);

CREATE TRIGGER B_update_versionnr BEFORE INSERT OR UPDATE ON B
FOR EACH ROW EXECUTE PROCEDURE update_versionnr();

drop table C;
create table C (
	ObjId		BIGINT		NOT NULL UNIQUE PRIMARY KEY,
	Owner		BIGINT		NOT NULL,
	VersionNr	INTEGER		NOT NULL,
        ItsBlob         TEXT,
        ItsString       TEXT
);

CREATE TRIGGER C_update_versionnr BEFORE INSERT OR UPDATE ON C
FOR EACH ROW EXECUTE PROCEDURE update_versionnr();
