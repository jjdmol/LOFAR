--
--  updateVTnode.sql: function updating a Node of a VIC template tree
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
-- updateVTnode (authToken, treeID, nodeID, instances, limits)
--
-- Saves the new values to the database
--
-- Authorisation: yes
--
-- Tables:	VICtemplate	update
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION updateVTnode(INT4, INT4, INT4, INT2, TEXT)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, tree, func, parameter)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
			RETURN FALSE;
		END IF;

		-- get ParentID of node to duplicate
		UPDATE	VICtemplate
		SET		instances = $4,
				limits    = $5
		WHERE	treeID = $2
		AND 	nodeID = $3;
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Node % could not be updated\', $3;
		  RETURN FALSE;
		END IF;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

