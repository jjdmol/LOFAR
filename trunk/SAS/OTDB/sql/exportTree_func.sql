--
--  exportTree.sql: makes an usenet format export of a (sub)tree.
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
-- recursive helper function
-- exportSubTree (treeID, topNodeID, basename)
--
-- Makes a key-value list of a (sub)tree in usenet format.
--
-- Authorisation: no
--
-- Tables:	VIChierarchy	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportSubTree(INT4, INT4, TEXT)
  RETURNS TEXT AS '
	DECLARE
	  vResult		TEXT := \'\';
	  vOwnName		VIChierarchy.name%TYPE;
	  vBaseName		VIChierarchy.name%TYPE;
	  vRow			RECORD;

	BEGIN
	  -- construct basename for own parameters
	  vBaseName := $3;
	  IF length(vBasename) != 0 THEN
		vBaseName := vBaseName || \'.\';
	  END IF;

	  -- first dump own parameters
	  FOR vRow IN
	    SELECT	name, value
	    FROM	VIChierarchy
	    WHERE	treeID = $1
	    AND	 	parentID = $2
		AND		leaf = true
	  LOOP
		vResult := vResult || vBasename || 
				              vRow.name || \'=\' || vRow.value || chr(10);
	  END LOOP;

	  -- call myself for all the children
	  FOR vRow IN
	    SELECT	nodeID, name
	    FROM	VIChierarchy
	    WHERE	treeID = $1
	    AND	 	parentID = $2
		AND		leaf = false
	  LOOP
		vResult := vResult || exportSubTree($1, vRow.nodeID, 
											vBaseName || vRow.name);
	  END LOOP;

	  RETURN vResult;
	END;
' LANGUAGE plpgsql;

--
-- exportTree (authToken, treeID, topNodeID)
--
-- Makes a key-value list of a (sub)tree in usenet format.
--
-- Authorisation: yes
--
-- Tables:	VIChierarchy	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportTree(INT4, INT4, INT4)
  RETURNS TEXT AS '
	DECLARE
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vBaseName		TEXT;
		vResult			TEXT;
		vFullName		TEXT;
		vName			VIChierarchy.name%TYPE;
		vParentID		VIChierarchy.parentID%TYPE;
		vnodeID			VIChierarchy.nodeID%TYPE;

		vNewTreeID		OTDBtree.treeID%TYPE;
		vCreatorID		OTDBtree.creator%TYPE;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, treetype)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
		END IF;


		-- construct name from treetop till topNode
		vBaseName := \'\';
		vParentID := -1;
		vNodeID   := $3;
		WHILE vParentID != 0 LOOP
		  SELECT name, nodeID, parentID
		  INTO	 vName, vNodeID, vParentID
		  FROM	 VIChierarchy
		  WHERE	 treeID = $2
		  AND	 nodeID = vNodeID;
		  IF NOT FOUND THEN
		    RAISE EXCEPTION \'Node % does not exist in tree %\', $3, $2;
		  END IF;
		  vBaseName := vName || \'.\' || vBaseName;
		  vNodeID   := vParentID;
		END LOOP;
		vResult := \'prefix=\' || vBasename || chr(10);

		-- construct entries for all nodes from here on.
		vResult := vResult || exportSubTree($2, $3, \'\'::text);
		RETURN vResult;
	END;
' LANGUAGE plpgsql;

