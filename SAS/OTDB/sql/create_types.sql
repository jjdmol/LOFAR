--
-- Creates the general types equal to the types in OTDBtypes.h
--

DROP TYPE	treeInfo	CASCADE;
DROP TYPE	OTDBnode	CASCADE;
DROP TYPE	OTDBvalue	CASCADE;

CREATE TYPE treeInfo AS (
	ID				INT4,			-- OTDBtree.treeID%TYPE,
	classification	INT2,			-- classification.ID%TYPE,
	creator			VARCHAR(20),	-- OTDBuser.username%TYPE,
	creationDate	timestamp(0),
	type			INT2,			-- treetype.ID%TYPE,
	originalTree	INT4,			-- OTDBtree.treeID%TYPE,
	campaign		VARCHAR(30),	-- campaign.name%TYPE,
	starttime		timestamp(0),
	stoptime		timestamp(0)
);

CREATE TYPE OTDBnode AS (
	ID				INT8,
	parentID		INT8,
	name			VARCHAR(30),
	index			SMALLINT,
	paramtype		INT2,			-- param_type.ID%TYPE,
	unit			VARCHAR(4),		-- unit.name%TYPE,
	description		VARCHAR(80)
);


CREATE TYPE OTDBvalue AS (
	ID				INT8,
	value			TEXT,
	time			timestamp(0)
);
