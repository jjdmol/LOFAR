--
--  create_security.sql: Creates the security related tables.
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
-- TODO: Develop management system for easy maintaining this info.
--		 Possibly introduce usergroups as well.
--

DROP TABLE OTDBaccess CASCADE;
DROP TABLE OTDBuser   CASCADE;
DROP SEQUENCE	OTDBuserID;

--
-- The OTDBuser table contains some admistrative info about the user.
--
CREATE SEQUENCE	OTDBuserID;

CREATE TABLE OTDBuser (
	userID		INT4			NOT NULL DEFAULT nextval('OTDBuserID'),
	username	VARCHAR(20)		NOT NULL,
	password	VARCHAR(20)		NOT NULL,
	role		VARCHAR(30),
	lastlogin	timestamp,

	CONSTRAINT	userID_uniq		UNIQUE(userID),		-- for references
	CONSTRAINT	username_uniq	UNIQUE(username)
) WITHOUT OIDS;

--
-- create one default (super)user that can be used during development.
--
-- TODO: Remove this user.
--
INSERT INTO OTDBuser (username, password, role) 
			VALUES ('paulus', 'boskabouter', 'developer');



--
-- The access table contains which users may call what functions with
-- what arguments
--
CREATE TABLE OTDBaccess (
	userID		INT4			NOT NULL REFERENCES OTDBuser(userID),
	task		INT4			NOT NULL DEFAULT 0,
	value		INT4			NOT NULL DEFAULT 0
) WITHOUT OIDS;


