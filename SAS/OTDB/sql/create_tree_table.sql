--
-- create_tree_table
--
-- Creates the OTDBtree table. This table is like the base tables used
-- by the rest of the tables.
--
DROP TABLE OTDBtree CASCADE;
DROP SEQUENCE	OTDBtreeID;


CREATE SEQUENCE	OTDBtreeID;

CREATE TABLE OTDBtree (
	-- required info
	treeID		INT4			NOT NULL DEFAULT nextval('OTDBtreeID'),
	originID	INT4			NOT NULL,
	classif		INT2			NOT NULL REFERENCES classification(ID),
	treetype	INT2			NOT NULL REFERENCES treetype(ID),
	creator		INT2			NOT NULL REFERENCES operator(ID),
	d_creation	TIMESTAMP(0)	DEFAULT 'now',

	-- optional info
	campaign	INT4			REFERENCES campaign(ID),
	starttime	TIMESTAMP(0),
	stoptime	TIMESTAMP(0),
	owner		INT2			REFERENCES operator(ID),

	-- contraints
	CONSTRAINT	tree_uniq		UNIQUE (treeID)
) WITHOUT OIDS;


