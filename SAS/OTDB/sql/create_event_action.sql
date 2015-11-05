--
--  create_event_action.sql: Creates the event and action tables.
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


-- creates the OTDB event and action tables

DROP TABLE OTDBevent    CASCADE;
DROP TABLE OTDBaction   CASCADE;
DROP TABLE eventStatus  CASCADE;
DROP TABLE actionStatus CASCADE;
DROP SEQUENCE	OTDBeventID;
DROP SEQUENCE	OTDBactionID;


--
-- Event status table
--
-- Defines the status values of an event
-- Note: the values are single-bit values so that they can be ORed
--
-- TODO: Check if table contents is complete
--
CREATE TABLE eventStatus (
	status		INT2			NOT NULL,
	name		VARCHAR(20)		NOT NULL,

	CONSTRAINT event_status_uniq		UNIQUE (status)
) WITHOUT OIDS;
INSERT INTO eventStatus VALUES (  1, 'idle');
INSERT INTO eventStatus VALUES (  2, 'busy');
INSERT INTO eventStatus VALUES (  4, 'suspicious');
INSERT INTO eventStatus VALUES (  8, 'defect');
INSERT INTO eventStatus VALUES ( 16, 'off-line');
INSERT INTO eventStatus VALUES ( 32, 'maintenance');


--
-- Action status table
--
-- Defines the status values of an action
-- Note: the values are single-bit values so that they can be ORed
--
-- TODO: Check if table contents is complete
--
CREATE TABLE actionStatus (
	status		INT2			NOT NULL,
	name		VARCHAR(20)		NOT NULL,

	CONSTRAINT action_status_uniq		UNIQUE (status)
) WITHOUT OIDS;
INSERT INTO actionStatus VALUES (  1, 'none');
INSERT INTO actionStatus VALUES (  2, 'assigned');
INSERT INTO actionStatus VALUES (  4, 'to be defined');
INSERT INTO actionStatus VALUES (  8, 'confirmed defect');
INSERT INTO actionStatus VALUES ( 16, 'off-line request');
INSERT INTO actionStatus VALUES ( 32, 'closed');


--
-- Event table
--
-- Defines the event table
-- 
CREATE SEQUENCE	OTDBeventID;

CREATE TABLE OTDBevent (
	eventID		INT4			NOT NULL DEFAULT nextval('OTDBeventID'),
	nodename	VARCHAR(80)		NOT NULL,
	status		INT2			NOT NULL REFERENCES eventStatus(status),
	eventTime	timestamp		DEFAULT now(),

	CONSTRAINT eventID_uniq		UNIQUE(eventID)	-- for reference by action
) WITHOUT OIDS;

-- example
-- INSERT INTO OTDBevent (nodename, status) VALUES ('LOFAR_LCU1_RACK2_CPU3', 4);


--
-- Action table
--
-- Defines the action table
--
CREATE SEQUENCE	OTDBactionID;

CREATE TABLE OTDBaction (
	actionID	INT4			NOT NULL DEFAULT nextval('OTDBactionID'),
	eventID		INT4			NOT NULL REFERENCES OTDBevent(eventID),
	userID		INT4			NOT NULL REFERENCES OTDBuser(userID),
	status		INT2			NOT NULL REFERENCES actionStatus(status),
	eventTime	timestamp		DEFAULT now(),
	description	TEXT

) WITHOUT OIDS;


