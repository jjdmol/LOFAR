--
--  copyVTtree.sql: create a full VIC tree from a template.
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
-- copyVTparams(orgNodeID, newTreeID, newNodeID)
-- 
-- Gives the newNodeID in the newTreeID the same parameters as the orgNode.
-- Note: new node must already exist.
--
-- Authorisation: none
--
-- Tables: 	VICtemplate		read
--			VICtemplate		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyVTparams(INT4, INT4, INT4)
  RETURNS VOID AS '
	DECLARE
		vParam	RECORD;

	BEGIN
	  FOR vParam IN 
		SELECT	originID, name, limits
		FROM 	VICtemplate
		WHERE	parentID = $1
		AND		leaf = TRUE
	  LOOP
		INSERT 
		INTO 	VICtemplate(treeID, parentID, originid, 
							name, index, leaf, 
							instances, limits)
	    VALUES 	($2, $3, vParam.originID, 
				 vParam.name, 1::int2, TRUE,
				 1::int2, vParam.limits);
		-- note: nodeId is defaulted.
	  END LOOP;
	  RETURN;
	END;
' LANGUAGE plpgsql;

--
-- helper function
-- copyVTleafNode(orgNodeID, newTreeID, newParentID):newNodeID
--
-- Duplicates a complete VT node to the VT tree
-- 
-- Authorisation: none
--
-- Tables: 	VICtemplate		read
--			VICtemplate		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyVTleafNode(INT4, INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
		vNode		RECORD;
		vNewNodeID	VICtemplate.nodeID%TYPE;

	BEGIN
	  SELECT *
	  INTO	 vNode
	  FROM	 VICtemplate
	  WHERE	 nodeID = $1;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'node % does not exist\', $1;
	  END IF;

	  vNewNodeID := NEXTVAL(\'VICtemplateID\');

	  INSERT
	  INTO	 VICtemplate (treeID, nodeID, parentID, 
						  originID, name, index, 
						  leaf, instances, limits)
	  VALUES ($2, vNewNodeID, $3, 
			  vNode.originID, vNode.name, vNode.index, 
			  vNode.leaf, vNode.instances, vNode.limits);

	  PERFORM copyVTparams($1, $2, vNewNodeID);

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;

--
-- helper function
-- copyVTsubTree(orgNodeID, newTreeID, newParentID): newNodeID
-- 
-- Duplicates a subtree starting at node orgNode to VT tree newTreeID
-- under node newParentID.
-- Recursive routine.
--
-- Authorisation: none
--
-- Tables: 	VICtemplate		read
--			VICtemplate		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyVTsubTree(INT4, INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
	  vNewNodeID		VICtemplate.nodeID%TYPE;
	  vDummy			VICtemplate.nodeID%TYPE;
	  vVTnode			RECORD;

	BEGIN
	  -- copy node itself
	  vNewNodeID := copyVTleafNode($1, $2, $3);

	  -- dive into the childen
	  FOR vVTnode IN 
	    SELECT	nodeID 
	    FROM	VICtemplate
	    WHERE	parentID = $1
	    AND		leaf = FALSE
	  LOOP
	    vDummy := copyVTsubTree(vVTnode.nodeID, $2, vNewNodeID);
	  END LOOP;

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;


