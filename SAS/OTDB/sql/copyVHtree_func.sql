--
--  copyVHtree.sql: copies a complete VH tree.
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
-- helper function
-- copyVHparams(orgNodeID, newTreeID, newNodeID)
-- 
-- Gives the newNodeID in the newTreeID the same parameters as the orgNode.
-- Note: new node must already exist.
--
-- Authorisation: none
--
-- Tables: 	VIChierarchy		read
--			VIChierarchy		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyVHparams(INT4, INT4, INT4)
  RETURNS VOID AS '
	DECLARE
		vParam	RECORD;

	BEGIN
	  FOR vParam IN 
		SELECT	paramRefID, name, index, leaf, value
		FROM 	VIChierarchy
		WHERE	parentID = $1
		AND		leaf = TRUE
	  LOOP
	    INSERT INTO VIChierarchy (treeID, parentID, paramRefID,
								name, index, leaf, value)
	    VALUES	($2, $3, vParam.paramRefID,
				 vParam.name, vParam.index, vParam.leaf, vParam.value);
		-- note: nodeId is defaulted.
	  END LOOP;
	  RETURN;
	END;
' LANGUAGE plpgsql;

--
-- helper function
-- copyVHleafNode(orgNodeID, newTreeID, newParentID):newNodeID
--
-- Duplicates a complete VH node to the VH tree
-- 
-- Authorisation: none
--
-- Tables: 	VIChierarchy		read
--			VIChierarchy		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyVHleafNode(INT4, INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
		vRow		RECORD;
		vNewNodeID	VIChierarchy.nodeID%TYPE;

	BEGIN
	  SELECT paramRefID, name, index, leaf, value
	  INTO	 vRow
	  FROM	 VIChierarchy
	  WHERE	 nodeID = $1;
--	  IF NOT FOUND THEN
--		RAISE EXCEPTION \'node % does not exist\', $1;
--	  END IF;

	  vNewNodeID := NEXTVAL(\'VIChierarchID\');

	  INSERT INTO VIChierarchy (treeID, nodeID, parentID, paramRefID,
								name, index, leaf, value)
	  VALUES	($2, vNewNodeID, $3, vRow.paramRefID,
				 vRow.name, vRow.index, vRow.leaf, vRow.value);

	  PERFORM copyVHparams($1, $2, vNewNodeID);

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;

--
-- recursive helper function
-- copyVHsubTree (topNodeID, newTreeID, newParentID)
--
-- Makes a key-value list of a (sub)tree in usenet format.
--
-- Authorisation: no
--
-- Tables:	VIChierarchy	read
-- Tables:	VIChierarchy	insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyVHsubTree(INT4, INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
	  vNewNodeID	VIChierarchy.nodeID%TYPE;
	  vDummy		VIChierarchy.nodeID%TYPE;
	  vRow			RECORD;

	BEGIN
	  -- copy node itself
	  vNewNodeID := copyVHleafNode($1, $2, $3);

	  -- call myself for all the children
	  FOR vRow IN
	    SELECT	nodeID
	    FROM	VIChierarchy
	    WHERE	parentID = $1
	    AND		leaf = FALSE
	  LOOP
		vDummy := copyVHsubTree(vRow.nodeID, $2, vNewNodeID);
	  END LOOP;

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;

