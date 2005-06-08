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
DROP TABLE validation 	  CASCADE;
DROP TABLE unit		 	  CASCADE;
DROP TABLE treetype 	  CASCADE;

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
	baseID		BOOL			DEFAULT TRUE,
	reference	BOOL			DEFAULT TRUE,

	CONSTRAINT class_id_uniq	UNIQUE (ID)
) WITHOUT OIDS;
INSERT INTO classification VALUES (1, 'development', TRUE, TRUE);
INSERT INTO classification VALUES (2, 'test', 		 TRUE, TRUE);
INSERT INTO classification VALUES (3, 'operational', TRUE, TRUE);
INSERT INTO classification VALUES (4, 'equal', 		 FALSE, TRUE);
INSERT INTO classification VALUES (5, 'obsolete', 	 TRUE, FALSE);

--
-- Constraint table
--
-- Defines the ways the node+parameter constraints can be checked.
--
CREATE TABLE constr_type (
	ID			INT2			NOT NULL,
	name		VARCHAR(6)		NOT NULL,

	CONSTRAINT constr_type_uniq		UNIQUE (ID)
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
	name		VARCHAR(4)		NOT NULL,

	CONSTRAINT param_type_uniq		UNIQUE (ID)
) WITHOUT OIDS;
INSERT INTO param_type VALUES (101, 'bool');
INSERT INTO param_type VALUES (102, 'int');
INSERT INTO param_type VALUES (103, 'long');
INSERT INTO param_type VALUES (104, 'flt');
INSERT INTO param_type VALUES (105, 'dbl');
INSERT INTO param_type VALUES (106, 'icpx');
INSERT INTO param_type VALUES (107, 'lcpx');
INSERT INTO param_type VALUES (108, 'fcpx');
INSERT INTO param_type VALUES (109, 'dcpx');
INSERT INTO param_type VALUES (110, 'text');
INSERT INTO param_type VALUES (111, 'bin');
-- PVSS values
INSERT INTO param_type VALUES ( 6, 'flt');
INSERT INTO param_type VALUES (20, 'uint');
INSERT INTO param_type VALUES (21, 'int');
INSERT INTO param_type VALUES (22, 'flt');
INSERT INTO param_type VALUES (23, 'bool');
INSERT INTO param_type VALUES (25, 'text');

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

	CONSTRAINT validation_id_uniq		UNIQUE (ID)
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
	label		VARCHAR(10)		NOT NULL,
	name		VARCHAR(5),
	format		VARCHAR(15),
	scalable	BOOL			DEFAULT FALSE,

	CONSTRAINT unit_id_uniq		UNIQUE (ID),
	CONSTRAINT unit_label_uniq	UNIQUE (label)
) WITHOUT OIDS;
INSERT INTO unit values (0, '',			'',		'',			false);
INSERT INTO unit values (1, 'ampere',	'A',	'',			true);
INSERT INTO unit values (2, 'm/s',		'm/s',	'',			false);
INSERT INTO unit values (3, 'dB',		'dB',	'',			false);
INSERT INTO unit values (4, 'time4',	'',		'99:99',	false);
INSERT INTO unit values (5, 'time6',	'',		'99:99:99',	false);

--
-- Treetype table
-- 
-- Define states/lables for trees that indicate the phase of the
-- lifecycle the tree is in.
CREATE TABLE treetype (
	ID			INT2			NOT NULL,
	name		VARCHAR(10)		NOT NULL,

	CONSTRAINT treetype_uniq	UNIQUE(ID)
)WITHOUT OIDS;
INSERT INTO treetype VALUES (10, 'hardware');
INSERT INTO treetype VALUES (20, 'VItemplate');
INSERT INTO treetype VALUES (30, 'configure');
INSERT INTO treetype VALUES (40, 'schedule');
INSERT INTO treetype VALUES (50, 'queued');
INSERT INTO treetype VALUES (60, 'active');
INSERT INTO treetype VALUES (70, 'finished');
INSERT INTO treetype VALUES (80, 'obsolete');

--
-- Campaign table
--
-- Used for assigning observation(tree)s to a campaign.
--
-- TODO: Remove records with ID's other than 0.
--
CREATE TABLE campaign (
	ID			INT2			NOT NULL,
	name		VARCHAR(30)		NOT NULL,
	
	CONSTRAINT	campaign_id_uniq	UNIQUE(ID)
) WITHOUT OIDS;
INSERT INTO campaign VALUES (0, 'no campaign');
INSERT INTO campaign VALUES (1, 'my campaign');
INSERT INTO campaign VALUES (2, 'your campaign');

--
-- Operator table
--
-- Names and ID of the operater allowed to manage the trees.
--
CREATE TABLE operator (
	ID			INT2			NOT NULL,
	name		VARCHAR(30)		NOT NULL,
	telephone	VARCHAR(10)		NOT NULL,
	
	CONSTRAINT	operator_id_uniq	UNIQUE (ID),
	CONSTRAINT	operator_name_uniq	UNIQUE (name)
) WITHOUT OIDS;
INSERT INTO operator VALUES (1, 'eucalypta', '0612345678');
INSERT INTO operator VALUES (2, 'gargamel', '0123456789');
