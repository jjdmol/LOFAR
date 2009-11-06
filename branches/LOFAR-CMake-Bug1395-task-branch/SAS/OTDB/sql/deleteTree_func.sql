--
--  deleteTree.sql: Delete a complete tree
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
-- deleteTree (authToken, treeID)
--
-- Delete a complete tree
--
-- Authorisation: yes
--
-- Tables:	otdbtree		delete
--			VICtemplate		delete [optional]
--			VIChierarchy	delete [optional]
--			PIChierarchy	delete [optional]
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION deleteTree(INT4, INT4)
  RETURNS VOID AS '
	DECLARE
		vFunction		INT2 := 1;
		TThardware 		CONSTANT INT2 := 10;
		TTtemplate 		CONSTANT INT2 := 20;
		TSactive		CONSTANT INT2 := 600;
		vOldTree		RECORD;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, -)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
		END IF;

		-- Get info from old tree and check type
		SELECT	*
		INTO	vOldTree
		FROM	OTDBtree
		WHERE	treeID = $2;
		IF vOldTree.state = TSactive THEN
		  RAISE EXCEPTION \'Active trees may not be deleted\';
		END IF;

		-- delete state history
		DELETE FROM StateHistory
		WHERE  treeID = $2;

		-- delete tree		
		IF vOldTree.treetype = TThardware THEN
		  DELETE FROM PIChierarchy
		  WHERE	 treeID = $2;
		ELSIF vOldTree.treetype = TTtemplate THEN
		  DELETE FROM VICtemplate
		  WHERE	 treeID = $2;
		ELSE
		  -- TODO: DELETE KVTs ALSO
		  DELETE FROM VIChierarchy
		  WHERE	 treeID = $2;
		END IF;

		-- Finally delete tree entry
		DELETE FROM OTDBtree
		WHERE  treeID = $2;

		RETURN;
	END;
' LANGUAGE plpgsql;
