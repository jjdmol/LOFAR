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
	paramID			INT4,
	parentID		INT4,
	name			VARCHAR(40),
	index			SMALLINT,
	leaf			BOOLEAN,
	par_type		INT2,			-- param_type.ID%TYPE,
	unit			INT2,
--	unit			VARCHAR(4),		-- unit.name%TYPE,
	description		TEXT
);


CREATE TYPE OTDBvalue AS (
	name			VARCHAR(120),
	value			TEXT,
	time			timestamp(0)
);
