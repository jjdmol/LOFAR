--
--  This file creates the database tables for classes A, B, and C,
--  which are used by the test programs tA and tC.
--
--  WARNING: THE TABLES ARE DROPPED BEFORE CREATION!!!
--
--  $Id$
--
drop table A;
create table A (
	ObjId		BIGINT		NOT NULL UNIQUE PRIMARY KEY,
	Owner		BIGINT		NOT NULL,
	VersionNr	INTEGER		NOT NULL,
	ItsInt		INTEGER,
	ItsDouble	DOUBLE PRECISION,
	ItsString	TEXT,
	itsComplex_Real DOUBLE PRECISION,
	itsComplex_Imag DOUBLE PRECISION
);

CREATE TRIGGER A_UpdateVersionNr BEFORE INSERT OR UPDATE ON A
FOR EACH ROW EXECUTE PROCEDURE UpdateVersionNr();

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

CREATE TRIGGER B_UpdateVersionNr BEFORE INSERT OR UPDATE ON B
FOR EACH ROW EXECUTE PROCEDURE UpdateVersionNr();

drop table C;
create table C (
	ObjId		BIGINT		NOT NULL UNIQUE PRIMARY KEY,
	Owner		BIGINT		NOT NULL,
	VersionNr	INTEGER		NOT NULL,
        ItsBlob         TEXT,
        ItsString       TEXT
);

CREATE TRIGGER C_UpdateVersionNr BEFORE INSERT OR UPDATE ON C
FOR EACH ROW EXECUTE PROCEDURE UpdateVersionNr();
