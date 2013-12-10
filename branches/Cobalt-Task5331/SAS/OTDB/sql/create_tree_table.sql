--
--  create_tree_table.sql: Create the tree table (the root to all info)
--
--  Copyright (C) 2005
--  ASTRON (Netherlands Foundation for Research in Astronomy)
--  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
--
--  This program is free software; you can redistribute it and/or modify
--  it under the terms of the GNU General Public License as published by
--  the Free Software Foundation; either version 2 of the License, or
--  (at your option) any later version.
--
--  This program is distributed in the hope that it will be useful,
--  but WITHOUT ANY WARRANTY; without even the implied warranty of
--  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--  GNU General Public License for more details.
--
--  You should have received a copy of the GNU General Public License
--  along with this program; if not, write to the Free Software
--  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
--
--  $Id$
--

--
-- Creates the OTDBtree table. This table is like the base tables used
-- by the rest of the tables.
--
-- Needs the tables:
--		classification
--		treetype
--		operator
--		campaign
--
DROP TABLE IF EXISTS OTDBtree CASCADE;
DROP SEQUENCE IF EXISTS	OTDBtreeID;
DROP SEQUENCE IF EXISTS	OTDBgroupID;
DROP TABLE IF EXISTS StateHistory CASCADE;
DROP INDEX IF EXISTS otdbtree_treeid_indx;

CREATE SEQUENCE	OTDBtreeID START 6112;
CREATE SEQUENCE	OTDBgroupID START 1;

CREATE TABLE OTDBtree (
    --  $Id$
	-- required info
	treeID		INT4			NOT NULL DEFAULT nextval('OTDBtreeID'),
	momID 		INT4			NOT NULL DEFAULT 0,
	originID	INT4			NOT NULL,
	groupID		INT4			NOT NULL DEFAULT 0,
	classif		INT2			NOT NULL REFERENCES classification(ID),
	treetype	INT2			NOT NULL REFERENCES treetype(ID),
	state		INT2			NOT NULL REFERENCES treestate(ID),
	creator		INT4			NOT NULL REFERENCES operator(ID),
	d_creation	TIMESTAMP(0)	DEFAULT now(),

	-- optional info
	campaign	INT2			REFERENCES campaign(ID),
	starttime	TIMESTAMP(0),
	stoptime	TIMESTAMP(0),
	owner		INT4			REFERENCES operator(ID),
	description	TEXT,
	name		VARCHAR(32),	-- for default templates only

	-- categorisation
	processType			VARCHAR(20) DEFAULT '',
	processSubtype		VARCHAR(50) DEFAULT '',
	strategy			VARCHAR(30) DEFAULT '',

	-- modifier
	modificationDate	TIMESTAMP(0) DEFAULT now(),

	-- contraints
	CONSTRAINT	tree_uniq		UNIQUE (treeID),
	CONSTRAINT 	tree_PK 		PRIMARY KEY (treeid)

) WITHOUT OIDS;

CREATE OR REPLACE FUNCTION newGroupID()
    --  $Id$
  RETURNS INT4 AS $$
  BEGIN
   RETURN nextval('OTDBgroupID');
  END;
$$ LANGUAGE plpgsql;

-- Index: otdbtree_treeid_indx

CREATE INDEX otdbtree_treeid_indx ON otdbtree(treeid);

CREATE TABLE StateHistory (
    --  $Id$
	treeID		INT4			NOT NULL,
	momID		INT4			NOT NULL,
	state		INT2			NOT NULL,
	userID		INT4			NOT NULL REFERENCES operator(ID),
	timestamp	TIMESTAMP(0)	DEFAULT now()
) WITHOUT OIDS;

