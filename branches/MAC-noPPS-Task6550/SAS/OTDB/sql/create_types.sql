--
--  create_types.sql: Define the SQL equivalents of the C++ types.
--
--  Copyright (C) 2005
--  ASTRON (Netherlands Foundation for Research in Astronomy)
--  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
-- Creates the general types equal to the types in OTDBtypes.h
--

DROP TYPE IF EXISTS	treeInfo		CASCADE;
DROP TYPE IF EXISTS	stateInfo		CASCADE;
DROP TYPE IF EXISTS	OTDBnode		CASCADE;
DROP TYPE IF EXISTS	OTDBparamDef	CASCADE;
DROP TYPE IF EXISTS	OTDBvalue		CASCADE;
DROP TYPE IF EXISTS	OTDBnodeDef		CASCADE;
DROP TYPE IF EXISTS	campaignInfo	CASCADE;

CREATE TYPE treeInfo AS (
    --  $Id$
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

CREATE TYPE stateInfo AS (
    --  $Id$
	treeID			INT4,			-- OTDBtree.treeID%TYPE,
	momID			INT4,			-- OTDBtree.momID%TYPE,
	state			INT2,			-- treestate.ID%TYPE,
	username		VARCHAR(20),	-- OTDBuser.username%TYPE,
	modtime			timestamp(0)
);

CREATE TYPE OTDBnode AS (
    --  $Id$
	nodeID			INT4,
	parentID		INT4,
	paramDefID		INT4,
	name			VARCHAR(150),
	index			SMALLINT,
	leaf			BOOLEAN,
	instances		INT2,			-- only filled for VIC template
	limits			TEXT,			-- only filled for VIC template
	description		TEXT			-- only filled for VIC template
);

-- make constructor for OTDBnode
CREATE OR REPLACE FUNCTION makeOTDBnode(INT4,INT4,INT4,VARCHAR(150),INT2,BOOLEAN,INT2,TEXT,TEXT)
  RETURNS OTDBnode AS $$
    --  $Id$
	DECLARE
	  vResult	RECORD;

	BEGIN
	  SELECT $1,$2,$3,$4,$5,$6,$7,$8,$9 INTO vResult;
	  RETURN vResult;
	END;
$$ LANGUAGE plpgsql;


CREATE TYPE OTDBparamDef AS (
    --  $Id$
	paramID			INT4,
	nodeID			INT4,
	name			VARCHAR(150),
	par_type		INT2,			-- param_type.ID%TYPE,
	unit			INT2,
	pruning			INT2,
	valmoment		INT2,
	RTmod			BOOLEAN,
	limits			TEXT,
	description		TEXT
);


CREATE TYPE OTDBvalue AS (
    --  $Id$
	paramID			INT4,
	name			VARCHAR(150),
	value			TEXT,
	time			timestamp
);


CREATE TYPE OTDBnodeDef AS (
    --  $Id$
	nodeID			INT4,
	name			VARCHAR(150),
	version			INT4,
	classif			INT2,
	constraints		TEXT,
	description		TEXT
);

CREATE TYPE campaignInfo AS (
    --  $Id$
    ID          INT2,
    name        VARCHAR(30),
    title       VARCHAR(100),
    PI          VARCHAR(80),
    CO_I        VARCHAR(80),
    contact     VARCHAR(120)
);

CREATE TYPE templateInfo AS (
    --  $Id$
	treeID			INT4,
	name			VARCHAR(32),
	processType		VARCHAR(20),
	processSubtype	VARCHAR(50),
	strategy		VARCHAR(30)
);

