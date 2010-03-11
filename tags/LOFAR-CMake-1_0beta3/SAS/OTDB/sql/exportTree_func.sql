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
-- exportVICSubTree (treeID, topNodeID, prefixlength)
--
-- Makes a key-value list of a (sub)tree in usenet format.
--
-- Authorisation: no
--
-- Tables:	VIChierarchy	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportVICSubTree(INT4, INT4, INT4)
  RETURNS TEXT AS '
	DECLARE
	  vResult		TEXT := \'\';
	  vRow			RECORD;

	BEGIN
	  -- first dump own parameters
	  FOR vRow IN
	    SELECT	name, value
	    FROM	VIChierarchy
	    WHERE	treeID = $1
	    AND	 	parentID = $2
		AND		leaf = true
		ORDER BY name
	  LOOP
		vResult := vResult || substr(vRow.name,$3) || \'=\' 
													|| vRow.value || chr(10);
	  END LOOP;

	  -- call myself for all the children
	  FOR vRow IN
	    SELECT	nodeID, name
	    FROM	VIChierarchy
	    WHERE	treeID = $1
	    AND	 	parentID = $2
		AND		leaf = false
		ORDER BY name
	  LOOP
		vResult := vResult || exportVICSubTree($1, vRow.nodeID, $3);
	  END LOOP;

	  RETURN vResult;
	END;
' LANGUAGE plpgsql;

--
-- recursive helper function
-- exportPICSubTree (treeID, topNodeID, prefixlength)
--
-- Makes a key-value list of a (sub)tree in usenet format.
--
-- Authorisation: no
--
-- Tables:	PIChierarchy	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportPICSubTree(INT4, INT4, INT4)
  RETURNS TEXT AS '
	DECLARE
	  vResult		TEXT := \'\';
	  vRow			RECORD;

	BEGIN
	  -- first dump own parameters
	  FOR vRow IN
	    SELECT	name     --, value
	    FROM	PIChierarchy
	    WHERE	treeID = $1
	    AND	 	parentID = $2
		AND		leaf = true
		ORDER BY name
	  LOOP
		vResult := vResult || substr(vRow.name,$3) || chr(10); 
--		vResult := vResult || substr(vRow.name,$3) || \'=\' 
--													|| vRow.value || chr(10);
	  END LOOP;

	  -- call myself for all the children
	  FOR vRow IN
	    SELECT	nodeID, name
	    FROM	PIChierarchy
	    WHERE	treeID = $1
	    AND	 	parentID = $2
		AND		leaf = false
		ORDER BY name
	  LOOP
		vResult := vResult || exportPICSubTree($1, vRow.nodeID, $3);
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
		vResult			TEXT;
		vName			VIChierarchy.name%TYPE;
		vPrefixLen		INTEGER;
		vIsPicTree		BOOLEAN;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, treetype)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
		END IF;

		-- get name of topNode
		vIsPicTree := FALSE;
		SELECT name
		INTO   vName
		FROM   VIChierarchy
		WHERE  treeID = $2
		AND	   nodeID = $3;
		IF NOT FOUND THEN
		  vIsPicTree := TRUE;
		  SELECT name
		  INTO   vName
		  FROM   PIChierarchy
		  WHERE  treeID = $2
		  AND	 nodeID = $3;
		  IF NOT FOUND THEN
		    RAISE EXCEPTION \'Node % does not exist in tree %\', $3, $2;
	      END IF;
		END IF;

		vPrefixLen = length(vName);
		vResult := \'prefix=\' || vName || \'.\' || chr(10);

		-- construct entries for all nodes from here on.
		IF vIsPicTree THEN
		  vResult := vResult || exportPICSubTree($2, $3, vPrefixLen+2);
		ELSE
		  vResult := vResult || exportVICSubTree($2, $3, vPrefixLen+2) || exportCampaign($2, vPrefixLen+2);
		END IF;
		RETURN vResult;
	END;
' LANGUAGE plpgsql;

