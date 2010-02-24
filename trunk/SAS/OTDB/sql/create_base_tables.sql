--
--  create_base_tables.sql: Creates the 'lookup' tables for OTDB
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
DROP TABLE classification CASCADE;
DROP TABLE constr_type 	  CASCADE;
DROP TABLE param_type 	  CASCADE;
DROP TABLE pvss_type 	  CASCADE;
DROP TABLE validation 	  CASCADE;
DROP TABLE unit		 	  CASCADE;
DROP TABLE treetype 	  CASCADE;
DROP TABLE treestate 	  CASCADE;

DROP SEQUENCE			  campaignID;
DROP TABLE campaign 	  CASCADE;
DROP TABLE operator 	  CASCADE;

--
-- Classification table
-- 
-- Assigns a level of 'maturity' to a node or tree
--
CREATE TABLE classification (
	ID			INT2			NOT NULL,
	name		VARCHAR(15)		NOT NULL,

	CONSTRAINT class_id_uniq	UNIQUE (ID),
	CONSTRAINT class_name_uniq	UNIQUE (name)
) WITHOUT OIDS;
INSERT INTO classification VALUES (1, 'development');
INSERT INTO classification VALUES (2, 'test');
INSERT INTO classification VALUES (3, 'operational');
INSERT INTO classification VALUES (4, 'example');

--
-- Constraint table
--
-- Defines the ways the node+parameter constraints can be checked.
--
CREATE TABLE constr_type (
	ID			INT2			NOT NULL,
	name		VARCHAR(10)		NOT NULL,

	CONSTRAINT constr_type_uniq		UNIQUE (ID),
	CONSTRAINT constr_name_uniq		UNIQUE (name)
) WITHOUT OIDS;
INSERT INTO constr_type VALUES (0, 'none');
INSERT INTO constr_type VALUES (1, 'shell');
INSERT INTO constr_type VALUES (2, 'python');
INSERT INTO constr_type VALUES (3, 'exec');

--
-- Parameter type
--
-- Assigns a type to a parameter. The type is only ment for the UI
-- so that it can check the values that are entered.
--
CREATE TABLE param_type (
	ID			INT2			NOT NULL,
	name		VARCHAR(5)		NOT NULL,

	CONSTRAINT param_type_uniq		UNIQUE (ID),
	CONSTRAINT param_name_uniq		UNIQUE (name)
) WITHOUT OIDS;
INSERT INTO param_type VALUES (  0, 'node');
INSERT INTO param_type VALUES (101, 'bool');
INSERT INTO param_type VALUES (102, 'int');
INSERT INTO param_type VALUES (103, 'uint');
INSERT INTO param_type VALUES (104, 'long');
INSERT INTO param_type VALUES (105, 'ulng');
INSERT INTO param_type VALUES (106, 'flt');
INSERT INTO param_type VALUES (107, 'dbl');
INSERT INTO param_type VALUES (108, 'icpx');
INSERT INTO param_type VALUES (109, 'lcpx');
INSERT INTO param_type VALUES (110, 'fcpx');
INSERT INTO param_type VALUES (111, 'dcpx');
INSERT INTO param_type VALUES (112, 'text');
INSERT INTO param_type VALUES (113, 'bin');
INSERT INTO param_type VALUES (114, 'time');
INSERT INTO param_type VALUES (115, 'date');
INSERT INTO param_type VALUES (201, 'vbool');
INSERT INTO param_type VALUES (202, 'vint');
INSERT INTO param_type VALUES (203, 'vuint');
INSERT INTO param_type VALUES (204, 'vlong');
INSERT INTO param_type VALUES (205, 'vulng');
INSERT INTO param_type VALUES (206, 'vflt');
INSERT INTO param_type VALUES (207, 'vdbl');
INSERT INTO param_type VALUES (208, 'vicpx');
INSERT INTO param_type VALUES (209, 'vlcpx');
INSERT INTO param_type VALUES (210, 'vfcpx');
INSERT INTO param_type VALUES (211, 'vdcpx');
INSERT INTO param_type VALUES (212, 'vtext');
INSERT INTO param_type VALUES (214, 'vtime');
INSERT INTO param_type VALUES (215, 'vdate');
INSERT INTO param_type VALUES (300, 'pnode');
INSERT INTO param_type VALUES (301, 'pbool');
INSERT INTO param_type VALUES (302, 'pint');
INSERT INTO param_type VALUES (303, 'puint');
INSERT INTO param_type VALUES (304, 'plong');
INSERT INTO param_type VALUES (305, 'pulng');
INSERT INTO param_type VALUES (306, 'pflt');
INSERT INTO param_type VALUES (307, 'pdbl');
INSERT INTO param_type VALUES (308, 'picpx');
INSERT INTO param_type VALUES (309, 'plcpx');
INSERT INTO param_type VALUES (310, 'pfcpx');
INSERT INTO param_type VALUES (311, 'pdcpx');
INSERT INTO param_type VALUES (312, 'ptext');
INSERT INTO param_type VALUES (314, 'ptime');
INSERT INTO param_type VALUES (315, 'pdate');

--
-- PVSS type
--
-- Assigns a name to the typeIDs of PVSS. The names should exist
-- in the param_type table.
--
CREATE TABLE pvss_type (
	ID			INT2			NOT NULL,
	name		VARCHAR(5)		NOT NULL REFERENCES param_type(name),

	CONSTRAINT pvss_type_uniq		UNIQUE (ID)
) WITHOUT OIDS;
-- PVSS values
INSERT INTO pvss_type VALUES ( 4, 'vuint');
INSERT INTO pvss_type VALUES ( 5, 'vint');
INSERT INTO pvss_type VALUES ( 6, 'vflt');
INSERT INTO pvss_type VALUES ( 9, 'vtext');
INSERT INTO pvss_type VALUES (20, 'int');
INSERT INTO pvss_type VALUES (21, 'int');
INSERT INTO pvss_type VALUES (22, 'flt');
INSERT INTO pvss_type VALUES (23, 'bool');
INSERT INTO pvss_type VALUES (25, 'text');

--
-- Validation table
--
-- Defines the validation moments in the lifecycle of a tree.
-- On such a moment all parameters labeled with that value must
-- meet the constraint criteria.
-- The validation values are passed to the constraint scripts.
--
-- TODO: Check contents of the table.
--
CREATE TABLE validation (
	ID			INT2			NOT NULL,
	name		VARCHAR(10)		NOT NULL,

	CONSTRAINT validation_id_uniq		UNIQUE (ID),
	CONSTRAINT validation_name_uniq		UNIQUE (name)
) WITHOUT OIDS;
INSERT INTO validation VALUES (0, 'never');
INSERT INTO validation VALUES (1, 'request');
INSERT INTO validation VALUES (2, 'time');
INSERT INTO validation VALUES (3, 'resource');

--
-- Unit table
--
-- The table defines the unit-information that should be displayed in
-- the UI when a parameter is displayed for input.
--
-- TODO: Extend table with a lot more records.
--
CREATE TABLE unit (
	ID			INT2			NOT NULL,
	name 		VARCHAR(10)		NOT NULL,
	label		VARCHAR(5),
	format		VARCHAR(15),
	scalable	BOOL			DEFAULT FALSE,

	CONSTRAINT unit_id_uniq		UNIQUE (ID),
	CONSTRAINT unit_name_uniq	UNIQUE (name)
) WITHOUT OIDS;
INSERT INTO unit values (0,  '-',		'',		 '',			false);
INSERT INTO unit values (1,  'ampere',	'A',	 '',			true);
INSERT INTO unit values (2,  'm/s',		'm/s',	 '',			false);
INSERT INTO unit values (3,  'dB',		'dB',	 '',			false);
INSERT INTO unit values (4,  'time4',	'',		 '99:99',		false);
INSERT INTO unit values (5,  'time6',	'',		 '99:99:99',	false);
INSERT INTO unit values (6,  'RAM',		'MB',	 '',			true);
INSERT INTO unit values (7,  'DISK',	'GB',	 '',			true);
INSERT INTO unit values (8,  'GFLOP',	'GFLOP', '',			false);
INSERT INTO unit values (9,  'MB/s',	'MB/s',	 '',			true);
INSERT INTO unit values (10, 'Hz',		'Hz',	 '',			true);
INSERT INTO unit values (11, 'kHz',		'kHz',	 '',			true);
INSERT INTO unit values (12, 'MHz',		'MHz',	 '',			true);
INSERT INTO unit values (13, 'period',  's|m|h', '',			false);

--
-- Treetype table
-- 
-- Define the kind of tree the information refers to.
CREATE TABLE treetype (
	ID			INT2			NOT NULL,
	name		VARCHAR(10)		NOT NULL,

	CONSTRAINT typeID_uniq		UNIQUE(ID),
	CONSTRAINT typename_uniq	UNIQUE(name)
)WITHOUT OIDS;
INSERT INTO treetype VALUES (10, 'hardware');
INSERT INTO treetype VALUES (20, 'VItemplate');
INSERT INTO treetype VALUES (30, 'VHtree');

--
-- Treetype table
-- 
-- Define states for trees that indicate the phase of the
-- lifecycle the tree is in.
CREATE TABLE treestate (
	ID			INT2			NOT NULL,
	name		VARCHAR(20)		NOT NULL,

	CONSTRAINT stateid_uniq		UNIQUE(ID),
	CONSTRAINT statename_uniq	UNIQUE(name)
)WITHOUT OIDS;
INSERT INTO treestate VALUES (  0, 'idle');
INSERT INTO treestate VALUES (100, 'described');
INSERT INTO treestate VALUES (200, 'prepared');
INSERT INTO treestate VALUES (300, 'approved');
INSERT INTO treestate VALUES (400, 'scheduled');
INSERT INTO treestate VALUES (500, 'queued');
INSERT INTO treestate VALUES (600, 'active');
INSERT INTO treestate VALUES (1000, 'finished');
INSERT INTO treestate VALUES (1100, 'aborted');
INSERT INTO treestate VALUES (1200, 'obsolete');

--
-- Campaign table
--
-- Used for assigning observation(tree)s to a campaign.
--
-- TODO: Remove records with ID's other than 0.
--
CREATE SEQUENCE	campaignID;
CREATE TABLE campaign (
	ID			INT2			NOT NULL DEFAULT NEXTVAL('campaignID'),
	name		VARCHAR(30)		NOT NULL,
	title		VARCHAR(100)	NOT NULL,
	PI			VARCHAR(80)		NOT NULL,
	CO_I		VARCHAR(80),
	contact		VARCHAR(120),
	
	CONSTRAINT	campaign_id_uniq	UNIQUE(ID),
	CONSTRAINT	campaign_name_uniq	UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO campaign(id, name, title, PI) VALUES (0, 'no campaign', 'not related to a campaign/project', 'unkwown');

--
-- Operator table
--
-- Names and ID of the operater allowed to manage the trees.
--
CREATE TABLE operator (
	ID			INT4			NOT NULL,
	name		VARCHAR(30)		NOT NULL,
	telephone	VARCHAR(10)		NOT NULL,
	
	CONSTRAINT	operator_id_uniq	UNIQUE (ID),
	CONSTRAINT	operator_name_uniq	UNIQUE (name)
) WITHOUT OIDS;
INSERT INTO operator VALUES (1, 'eucalypta', '0612345678');
INSERT INTO operator VALUES (2, 'gargamel', '0123456789');
