--
--  getVICnodedef.sql: function for getting one node from the VIC comp. table
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
-- getVICnodedef (nodeID)
-- 
-- Get one node definition on nodeID
--
-- Authorisation: none
--
-- Tables: 	vicnodedef		read
--
-- Types:	OTDBnodeDef
--
CREATE OR REPLACE FUNCTION getVICnodedef(INT4)
  RETURNS OTDBnodeDef AS '
	DECLARE
		vRecord		RECORD;

	BEGIN
	    SELECT nodeid,
			   name, 
			   version,
			   classif, 
			   constraints, 
			   description
		INTO   vRecord
		FROM   VICnodeDef
		WHERE  nodeID = $1;

		IF NOT FOUND THEN
			RAISE EXCEPTION \'Component % does not exist.\', $1;
		END IF;

		RETURN vRecord;
	END
' LANGUAGE plpgsql;

--
-- getVICnodedef (name, version, classif)
-- 
-- Get one node definition on name,version and classif
--
-- Authorisation: none
--
-- Tables: 	vicnodedef		read
--
-- Types:	OTDBnodeDef
--
CREATE OR REPLACE FUNCTION getVICnodedef(VARCHAR(40), INT4, INT2)
  RETURNS OTDBnodeDef AS '
	DECLARE
		vRecord		RECORD;

	BEGIN
	    SELECT nodeid,
			   name, 
			   version,
			   classif, 
			   constraints, 
			   description
		INTO   vRecord
		FROM   VICnodeDef
		WHERE  name = rtrim(translate($1, \'.\', \' \'))
		AND	   version = $2
		AND	   classif = $3;

		IF NOT FOUND THEN
			RAISE EXCEPTION \'Component %,%,% does not exist.\', $1, $2, $3;
		END IF;

		RETURN vRecord;
	END
' LANGUAGE plpgsql;

