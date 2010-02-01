--
--  create_types.sql: Define the SQL equivalents of the C++ types.
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
-- Creates the general types equal to the types in OTDBtypes.h
--

DROP TYPE	treeInfo		CASCADE;
DROP TYPE	stateInfo		CASCADE;
DROP TYPE	OTDBnode		CASCADE;
DROP TYPE	OTDBparamDef	CASCADE;
DROP TYPE	OTDBvalue		CASCADE;
DROP TYPE	OTDBnodeDef		CASCADE;

CREATE TYPE treeInfo AS (
	treeID			INT4,			-- OTDBtree.treeID%TYPE,
	momID			INT4,
	classification	INT2,			-- classification.ID%TYPE,
	creator			VARCHAR(20),	-- OTDBuser.username%TYPE,
	creationDate	timestamp(0),
	type			INT2,			-- treetype.ID%TYPE,
	state			INT2,			-- treestate.ID%TYPE,
	originalTree	INT4,			-- OTDBtree.treeID%TYPE,
	campaign		VARCHAR(30),	-- campaign.name%TYPE,
	starttime		timestamp(0),
	stoptime		timestamp(0),
	description		TEXT
);

CREATE TYPE stateInfo AS (
	treeID			INT4,			-- OTDBtree.treeID%TYPE,
	momID			INT4,			-- OTDBtree.momID%TYPE,
	state			INT2,			-- treestate.ID%TYPE,
	username		VARCHAR(20),	-- OTDBuser.username%TYPE,
	modtime			timestamp(0)
);

CREATE TYPE OTDBnode AS (
	nodeID			INT4,
	parentID		INT4,
	paramDefID		INT4,
	name			VARCHAR(40),
	index			SMALLINT,
	leaf			BOOLEAN,
	instances		INT2,			-- only filled for VIC template
	limits			TEXT,			-- only filled for VIC template
	description		TEXT			-- only filled for VIC template
);

-- make constructor for OTDBnode
CREATE OR REPLACE FUNCTION makeOTDBnode(INT4,INT4,INT4,VARCHAR(40),INT2,BOOLEAN,INT2,TEXT,TEXT)
  RETURNS OTDBnode AS '
	DECLARE
	  vResult	RECORD;

	BEGIN
	  SELECT $1,$2,$3,$4,$5,$6,$7,$8,$9 INTO vResult;
	  RETURN vResult;
	END;
' LANGUAGE plpgsql;


CREATE TYPE OTDBparamDef AS (
	paramID			INT4,
	nodeID			INT4,
	name			VARCHAR(40),
	par_type		INT2,			-- param_type.ID%TYPE,
--	unit			VARCHAR(4),		-- unit.name%TYPE,
	unit			INT2,
	pruning			INT2,
	valmoment		INT2,
	RTmod			BOOLEAN,
	limits			TEXT,
	description		TEXT
);


CREATE TYPE OTDBvalue AS (
	paramID			INT4,
	name			VARCHAR(150),
	value			TEXT,
	time			timestamp(0)
);


CREATE TYPE OTDBnodeDef AS (
	nodeID			INT4,
	name			VARCHAR(40),
	version			INT4,
	classif			INT2,
	constraints		TEXT,
	description		TEXT
);
