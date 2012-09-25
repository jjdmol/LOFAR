-- add new columns to the tree metadata table
ALTER TABLE OTDBtree 
	ADD COLUMN groupID			INT4 NOT NULL DEFAULT 0,
	ADD COLUMN processType		VARCHAR(20),
	ADD COLUMN processSubtype	VARCHAR(50),
	ADD COLUMN strategy			VARCHAR(30);

DROP SEQUENCE	OTDBgroupID;
CREATE SEQUENCE	OTDBgroupID START 1;
CREATE OR REPLACE FUNCTION newGroupID()
  RETURNS INT4 AS $$
  BEGIN
   RETURN nextval('OTDBgroupID');
  END;
$$ LANGUAGE plpgsql;

-- Change treeInfo structure by adding 5 fields
DROP TYPE treeInfo CASCADE;
CREATE TYPE treeInfo AS (
	treeID				INT4,			-- OTDBtree.treeID%TYPE,
	momID				INT4,
	groupID				INT4,
	classification		INT2,			-- classification.ID%TYPE,
	creator				VARCHAR(20),	-- OTDBuser.username%TYPE,
	creationDate		timestamp(0),
	type				INT2,			-- treetype.ID%TYPE,
	state				INT2,			-- treestate.ID%TYPE,
	originalTree		INT4,			-- OTDBtree.treeID%TYPE,
	campaign			VARCHAR(30),	-- campaign.name%TYPE,
	starttime			timestamp(0),
	stoptime			timestamp(0),
	processType			VARCHAR(20),
	processSubtype		VARCHAR(50),
	strategy			VARCHAR(30),
	description			TEXT
);
DROP FUNCTION setmominfo(integer, integer, integer, text);
-- Reload the functions that where dropped.
\i  getTreeGroup_func.sql
\i  getTreesInPeriod_func.sql
\i  getExecutableTrees_func.sql
\i  getTreeInfo_func.sql
\i  getTreeList_func.sql
\i	setMomInfo_func.sql

-- Change the defaultTemplate information structure also
DROP TYPE templateInfo CASCADE;
CREATE TYPE templateInfo AS (
	treeID			INT4,
	name			VARCHAR(32),
	processType		VARCHAR(20),
	processSubtype	VARCHAR(50),
	strategy		VARCHAR(30)
);
-- Reload the functions that we dropped
\i	getDefaultTemplates_func.sql

-- Add new functions
\i	assignProcessType_func.sql

-- Reload modified trees
\i	copyTree_func.sql
\i	instanciateTree_func.sql
\i 	exportTree_func.sql

-- init new variables in current trees
UPDATE otdbtree SET strategy='' where strategy is null;
UPDATE otdbtree SET processSubtype='' where processSubtype is null;
UPDATE otdbtree SET processType='' where processType is null;

--DROP TABLE processTypes;
--CREATE TABLE processTypes  (
--	processType		VARCHAR(20)	  NOT NULL DEFAULT '',
--	processSubtype	VARCHAR(50)   NOT NULL DEFAULT '',
--	strategy		VARCHAR(30)	  NOT NULL DEFAULT '',
--	CONSTRAINT combi_uniq UNIQUE (processType,processSubtype,strategy)
--) WITHOUT OIDS;
-- NOTE: unfortunately indexes do not work on NULL values

--INSERT INTO processTypes VALUES ('');
--INSERT INTO processTypes VALUES ('Observation', '[Interferometer]');
--INSERT INTO processTypes VALUES ('Observation', '[TransientBufferBoard]');
--INSERT INTO processTypes VALUES ('Observation', '[CoherentStokes]');
--INSERT INTO processTypes VALUES ('Observation', '[IncoherentStokes]');
--INSERT INTO processTypes VALUES ('Observation', '[Interferometer, TransientBufferBoard]');
--INSERT INTO processTypes VALUES ('Pipeline', '[CompressionPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[CalibrationPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[SimpleImagePipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[TransientPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[RMSynthesisPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[TBBCosmicRayPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[TBBTransientPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[KnownPulsarPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[PulsarSearchPipeline]');
--INSERT INTO processTypes VALUES ('Pipeline', '[BFCosmicRayPipeline]');
--INSERT INTO processTypes VALUES ('System', '[Ingest]');
--INSERT INTO processTypes VALUES ('System', '[CleanUp]');
--INSERT INTO processTypes VALUES ('Maintenance');
--INSERT INTO processTypes VALUES ('Reservation');
