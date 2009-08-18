--
--  create_PIC_tables.sql: creates all PIC tables
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
-- There are three PIC tables:
-- PICparamref: this is a flat table containing the full PVSS names.
-- 				each parameter is exactly ONCE is this table, independant
--				in how many trees the parameters are used.
-- PIChierarchy: this table contains the hierarchy from the PIC trees.
--				The table is mainly used for support for the Observation
--				Tree Browser so that the VIC and the PIC can be used in the
--				same way.
-- PICkvt:		This table contains the key-value-timestamp values of the PIC
--				parameters. These values are added to the database continuesly,
--				independant if there are observations running of not.
--
-- Needs the tables:
--		param_type, unit, OTDBtree

DROP TABLE PIChierarchy CASCADE;
DROP TABLE PICkvt 		CASCADE;
DROP TABLE PICparamref  CASCADE;
DROP SEQUENCE PICparamrefID;
DROP SEQUENCE PIChierarchID;

--
-- The PIC reference table is the representation of the master PIC 
-- tree from PVSS. The tree is completely flat, the hierarchy is in
-- the naming convention of the parameters.
--
-- Table is filled from an export of the master PIC from PVSS.
--
CREATE SEQUENCE PICparamrefID;

CREATE TABLE PICparamref (
	paramID		INT4			NOT NULL DEFAULT nextval('PICparamrefID'),
	PVSSname	VARCHAR(150)	NOT NULL,
	par_type	INT2			REFERENCES param_type(ID),
	unit		INT2			DEFAULT 0 REFERENCES unit(ID),
	pruning		INT2			DEFAULT 10,
	description	TEXT,

	CONSTRAINT	paramID_uniq		UNIQUE(paramID),
	CONSTRAINT	paramname_uniq		UNIQUE(PVSSname)
) WITHOUT OIDS;


--
-- When a new master PIC file is loaded into the PICparamref a new tree
-- in the PIChierarchy table is created simultaneously.
-- The hierachy stored in this table is derived the naming convention
-- used for the PVSSnames.
--
CREATE SEQUENCE	PIChierarchID;

CREATE TABLE PIChierarchy (
	treeID		INT4			NOT NULL REFERENCES OTDBtree(treeID),
	nodeID		INT4			NOT NULL DEFAULT nextval('PIChierarchID'),
	parentID	INT4			NOT NULL, --  REFERENCES PIChierachy(nodeID),
	paramRefID	INT4			NOT NULL REFERENCES PICparamref(paramID),
	name		VARCHAR(150)		NOT NULL,
	index		INT2			NOT NULL DEFAULT -1,
	leaf		BOOLEAN			DEFAULT TRUE,

	CONSTRAINT	param_uniq_in_tree	UNIQUE(treeID, nodeID)
) WITHOUT OIDS;


--
-- PIC Key Values Time sets.
--
-- Stores the updates from the PIC values.
-- Note: the PIC KVTs do NOT have a reference to a PIC tree
-- because in simultaneous observations different PIC reference trees
-- can be used. A change in value is important for all observations.
CREATE TABLE PICkvt (
	paramID		INT4			NOT NULL REFERENCES PICparamref(paramID), 
	value		TEXT			NOT NULL,
	time		TIMESTAMP		DEFAULT now(),

	CONSTRAINT	pickvt_uniq		UNIQUE(paramID, time)
) WITHOUT OIDS;

CREATE INDEX PIC_kvt_id   ON PICkvt(paramID);
CREATE INDEX PIC_kvt_time ON PICkvt(time);

