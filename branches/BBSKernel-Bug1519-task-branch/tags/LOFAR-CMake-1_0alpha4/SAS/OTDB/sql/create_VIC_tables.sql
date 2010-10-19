--
--  create_VIC_tables.sql: creates all VIC tables
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
-- There are five VIC tables:
-- VICnodedef:	defines the loose nodes (components) of a tree.
-- VICparamref: defines the parameters of a node.
-- VICtemplate:	contains the 'folded' trees. These trees are edited by the
--				astronimer and instrument scientist.
-- VIChierarchy: this table contains the full hierarchical VIC trees used
--				by the scheduler and MAC.
-- VICkvt:		This table contains the key-value-timestamp values of the VIC
--				parameters. These values are added to the database during
--				observations. KVT from shared applications are logged by
--				parametername others are logged with parameterID.
--
-- Needs the tables:
--		OTDBtree, classification, param_type, unit, treestate 

DROP TABLE VICnodedef 	CASCADE;
DROP TABLE VICparamdef  CASCADE;
DROP TABLE VICtemplate	CASCADE;
DROP TABLE VIChierarchy CASCADE;
DROP TABLE VICkvt 		CASCADE;

DROP SEQUENCE VICnodedefID;
DROP SEQUENCE VICparamdefID;
DROP SEQUENCE VICtemplateID;
DROP SEQUENCE VIChierarchID;

--
-- The VIC node Definition table contains the definitions from
-- the node architecture. Note that a node is nothing more than a
-- container for parameters. The relation with the children nodes
-- are also stored as parameters of the node:
-- name of the parameter is #<name of the childnode>
--
-- Table is filled by reading in definitionfiles from the developers.
--
CREATE SEQUENCE	VICnodedefID;

CREATE TABLE VICnodedef (
	nodeID		INT4			NOT NULL DEFAULT nextval('VICnodedefID'),
	name		VARCHAR(40)		NOT NULL,
	version		INT4			NOT NULL DEFAULT 010000,
	classif		INT2			NOT NULL REFERENCES classification(ID),
	constraints	TEXT,			-- interpreted by OTDB
	description	TEXT,

	CONSTRAINT	Vnodedef_node_uniq	UNIQUE(nodeID),
	CONSTRAINT	Vnodedef_name_uniq  UNIQUE(name, version, classif)
) WITHOUT OIDS;


--
-- The VIC parameter Definition table contains the definitions from
-- the parameters from a node.
--
-- Table is filled by reading in definitionfiles from the developers.
--
CREATE SEQUENCE VICparamdefID;

CREATE TABLE VICparamdef (
	paramID		INT4			NOT NULL DEFAULT nextval('VICparamdefID'),
	nodeID		INT4			NOT NULL REFERENCES VICnodedef(nodeID),
	name		VARCHAR(40)		NOT NULL,
	par_type	INT2			REFERENCES param_type(ID),
	unit		INT2			REFERENCES unit(ID),
	pruning		INT2			DEFAULT 10,
	validmoment	INT2,			-- REFERENCES treeState(ID)
	RTmod		BOOLEAN 		DEFAULT FALSE,
	limits		TEXT,			-- interpreted by GUI: range, enum, default
	description	TEXT,

	CONSTRAINT	VparamID_uniq		UNIQUE(paramID),
	CONSTRAINT	Vparamname_uniq		UNIQUE(nodeID,name)
) WITHOUT OIDS;


--
-- The VIC template table containts the 'folded' trees that are used during
-- the define fase of the tree.
--
-- Note: When index = -1 the node is a 'master' node defining the number of
-- nodes of that type that are available in the variable 'instances'.
-- When index != -1 the record specifies one specific node the instances
-- variable is 1 in that case.
--
-- Note2: When leaf = false the record is a node and the limits field contains
-- the max number of childs the node can have. When leaf = true the record
-- defines a parameter, the limit field is than used as parameter value.
-- (= initial startup value for the parameter).
--
CREATE SEQUENCE	VICtemplateID;

CREATE TABLE VICtemplate (
	treeID		INT4			NOT NULL REFERENCES OTDBtree(treeID),
	nodeID		INT4			NOT NULL DEFAULT nextval('VICtemplateID'),
	parentID	INT4			NOT NULL,  -- REFERENCES VICtemplate(nodeID),
	originID	INT4			NOT NULL DEFAULT 0, -- REF VICnode or VICparam
	name		VARCHAR(40)		NOT NULL,
	index		INT2			NOT NULL DEFAULT -1,
	leaf		BOOLEAN			DEFAULT TRUE,
	instances	INT2			NOT NULL DEFAULT 1,
	limits		TEXT,			-- interpreted by GUI: range, enum, default

	CONSTRAINT	VTemplNode_uniqin_tree	UNIQUE(treeID, nodeID)
) WITHOUT OIDS;


--
-- The VIChierarchy table contains complete VIC trees, the structure is
-- similar to the structure of the PIChierarchy table.
-- The VIChierachy trees are created from a VICtemplate tree by a script.
--
CREATE SEQUENCE	VIChierarchID;

CREATE TABLE VIChierarchy (
	treeID		INT4			NOT NULL REFERENCES OTDBtree(treeID),
	nodeID		INT4			NOT NULL DEFAULT nextval('VIChierarchID'),
	parentID	INT4			NOT NULL, -- REFERENCES VIChierachy(nodeID),
	paramRefID	INT4			NOT NULL, -- REFERENCES VICparamref(paramID),
	name		VARCHAR(150)	NOT NULL,
	index		INT2			NOT NULL DEFAULT -1,
	leaf		BOOLEAN			DEFAULT TRUE,
	value		TEXT,			-- empty for nodes, filled for params

	CONSTRAINT	Vparam_uniq_in_tree	UNIQUE(treeID, nodeID)
) WITHOUT OIDS;


--
-- VIC Key Values Time sets.
--
-- Stores the updates from the VIC values.
-- Note: parameter updates of shared applications are stored by name
-- because there is not 1 unique paramID assigned to it.
-- For other parameter updates the name is translated to a paramID and the
-- parametername is set to NULL.
--
CREATE TABLE VICkvt (
	treeID		INT4			NOT NULL REFERENCES OTDBtree(treeID),
	paramName	VARCHAR(150)	DEFAULT NULL, -- for shared applications
	value		TEXT			NOT NULL,
	time		TIMESTAMP		DEFAULT now(),

	CONSTRAINT	vickvt_uniq		UNIQUE(treeID, paramName, time)
) WITHOUT OIDS;

CREATE INDEX VIC_kvt_name ON VICkvt(paramName);
CREATE INDEX VIC_kvt_time ON VICkvt(time);

