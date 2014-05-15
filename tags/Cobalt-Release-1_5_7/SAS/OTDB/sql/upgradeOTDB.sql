-- add new columns to the tree metadata table
ALTER TABLE OTDBtree 
	ADD COLUMN modificationDate	timestamp(0) DEFAULT now();

-- Change treeInfo structure by adding 5 fields
DROP TYPE IF EXISTS treeInfo CASCADE;
CREATE TYPE treeInfo AS (
	treeID				INT4,			-- OTDBtree.treeID%TYPE,
	momID				INT4,
	groupID				INT4,
	classification		INT2,			-- classification.ID%TYPE,
	creator				VARCHAR(20),	-- OTDBuser.username%TYPE,
	creationDate		timestamp(0),
	modificationDate	timestamp(0),
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

-- Reload the functions that where dropped.
\i getBrokenHardware_func.sql
\i getDefaultTemplates_func.sql

-- Reload modified functions
\i getTreeGroup_func.sql
\i getTreesInPeriod_func.sql
\i getTreeList_func.sql
\i getTreeInfo_func.sql
\i getExecutableTrees_func.sql

-- Load new functions
\i create_rules.sql
\i getModifiedTrees_func.sql
