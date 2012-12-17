--
--  exportResultTree.sql: makes an usenet format export of a (sub)tree.
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
--  $Id: exportTree_func.sql 20032 2012-02-07 07:10:34Z overeem $
--

--
-- recursive helper function
-- exportMDSubTree (treeID, topNodeID, prefixlength)
--
-- Makes a key-value list of a (sub)tree in usenet format.
--
-- Authorisation: no
--
-- Tables:	VIChierarchy	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportMDSubTree(INT4, INT4, INT4)
  RETURNS TEXT AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
	  vResult		TEXT := '';
	  vRow			RECORD;
	  vValue		TEXT;
	  vLastTable	VARCHAR(50) := '';
	  vDefinition	TEXT := '';

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
		SELECT value INTO vValue FROM VICkvt WHERE treeID=$1 AND paramname=vRow.name ORDER BY time DESC LIMIT 1;
		IF FOUND THEN
		  vRow.value := vValue;
--RAISE WARNING 'MDPARAM: % = %', vRow.name, vValue;
		END IF;
		vResult := vResult || substr(vRow.name,$3) || '=' || vRow.value || chr(10);
	  END LOOP;

	  -- call myself for all the children
	  FOR vRow IN
	    SELECT	nodeID, name, recordID, tablename
	    FROM	VIChierarchy
	    WHERE	treeID = $1
	    AND	 	parentID = $2
		AND		leaf = false
		ORDER BY name
	  LOOP
--RAISE WARNING 'NODE: %, %, %', vRow.nodeID, vRow.name, vRow.tablename;
		IF vRow.tablename != '' THEN
			-- export definition before first record
			SELECT value INTO vValue FROM VICkvt WHERE treeID=$1 AND paramname=vRow.name ORDER BY time DESC LIMIT 1;
			IF NOT FOUND THEN
			  EXECUTE 'SELECT * FROM export' || vRow.tablename || '(' || vRow.recordID || ')' INTO vValue;
			END IF;
			vResult := vResult || substr(vRow.name,$3) || '=' || vValue || chr(10);
		END IF;
		vResult := vResult || exportMDSubTree($1, vRow.nodeID, $3);
	  END LOOP;

	  RETURN vResult;
	END;
$$ LANGUAGE plpgsql;

--
-- exportResultTree (authToken, treeID, topNodeID)
--
-- Makes a key-value list of a (sub)tree in usenet format.
--
-- Authorisation: yes
--
-- Tables:	VIChierarchy	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportResultTree(INT4, INT4, INT4)
  RETURNS TEXT AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vResult			TEXT;
		vName			VIChierarchy.name%TYPE;
		vParent			VICtemplate.parentid%TYPE;
		vPrefixLen		INTEGER;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, treetype)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION 'Not authorized';
		END IF;

		-- get name of topNode
		SELECT name
		INTO   vName
		FROM   VIChierarchy
		WHERE  treeID = $2
		AND	   nodeID = $3;
	    IF NOT FOUND THEN
	      RAISE EXCEPTION 'Node % is not a node in VIC tree %', $3, $2;
		END IF;

		vPrefixLen = length(vName);
		vResult := 'prefix=' || vName || '.' || chr(10);

		-- construct entries for all nodes from here on.
		vResult := vResult || exportMDSubTree($2, $3, vPrefixLen+2) || exportProcessType($2, vPrefixLen+2) || exportCampaign($2, vPrefixLen+2);
		RETURN vResult;
	END;
$$ LANGUAGE plpgsql;

