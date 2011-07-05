--
--  addComponentToVT.sql: create a VIC template from the component database
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
-- instanciateVTleafNode(orgNodeID, newTreeID, newParentID, newName):newNodeID
--
-- Constructs a VT node from the given VC nodeID.
-- 
-- Authorisation: none
--
-- Tables: 	VICnodedef		read
--			VICtemplate		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVTleafNode(INT4, INT4, INT4, VARCHAR(150))
  RETURNS INT4 AS '
	DECLARE
		vNode		RECORD;
		vNewNodeID	VICtemplate.nodeID%TYPE;

	BEGIN
	  SELECT	nodeID, constraints
	  INTO		vNode
	  FROM 		VICnodeDef
	  WHERE		nodeID = $1;

	  vNewNodeID := nextval(\'VICtemplateID\');
	  INSERT 
	  INTO	 VICtemplate(treeID, nodeID, parentID, originID, 
						 name, leaf, instances, limits)
	  VALUES ($2, vNewNodeID, $3, vNode.nodeID,  
			  $4, false, 1, vNode.constraints);
	  -- note: nodeId and index are defaulted.

	  PERFORM instanciateVTparams($1, $2, vNewNodeID);

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;


-- addComponentToVT(authToken, orgNodeID, newTreeID, newParentID, newName): newNodeID
-- 
-- Add the given component under the given parentNode of a template
-- tree.
--
-- Authorisation: none
--
-- Tables: 	VICnodedef		read
--			VICparamdef		read
--			VICtemplate		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION addComponentToVT(INT4, INT4, INT4, INT4, VARCHAR(150))
  RETURNS INT4 AS '
	DECLARE
	  vFunction  		CONSTANT INT2 := 1;
	  TTtemplate 		CONSTANT INT2 := 20;
	  vIsAuth			BOOLEAN;
	  vAuthToken		ALIAS FOR $1;
	  vTreeType			OTDBtree.treeType%TYPE;
	  vLeaf				BOOLEAN;
	  vNewNodeID		VICtemplate.nodeID%TYPE;
	  vNodeName			VICtemplate.name%TYPE;
	  vVersion			VICnodedef.version%TYPE;
	  vParentRefID		VICtemplate.originID%TYPE;
	  vDummy			VICparamDef.paramID%TYPE;
	  vNewName			VARCHAR(150);

	BEGIN
	  -- check authorisation(authToken, treeID, func, dummy)
	  vIsAuth := FALSE;
	  SELECT isAuthorized(vAuthToken, $3, vFunction, 0)
	  INTO	 vIsAuth;
	  IF NOT vIsAuth THEN
		RAISE EXCEPTION \'Not authorized\';
	  END IF;

	  -- check tree type
	  SELECT treeType
	  INTO	 vTreeType
	  FROM	 OTDBtree
	  WHERE	 treeID = $3;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'Tree % does not exist\', $3;
	  END IF;
	  IF vTreeType <> TTtemplate THEN
		RAISE EXCEPTION \'Tree % is not a template tree\', $3;
	  END IF;

	  -- check if parent node exist when not adding top node
	  IF $4 <> 0 THEN
	    SELECT leaf
	    INTO   vLeaf
	    FROM   VICtemplate
	    WHERE  treeID = $3
	    AND	   nodeID = $4;
	    IF NOT FOUND THEN
		  RAISE EXCEPTION \'Node % does not exist in tree %\', $4, $3;
	    END IF;
	    IF vLeaf = TRUE THEN
		  RAISE EXCEPTION \'Node % is a parameter, not a node.\', $4;
	    END IF;
	  END IF;

	  -- check if orgNode exists
	  SELECT name, version
	  INTO	 vNodeName, vVersion
	  FROM	 VICnodeDef
	  WHERE	 nodeid = $2;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'Original node not found in components.\';
	  END IF;

	  -- check if this node may be a child from the parent.
	  IF $4 <> 0 THEN
	    -- first get definition of parent
		SELECT originID
		INTO   vParentRefID
		FROM   VICtemplate
		WHERE  nodeID = $4;
		-- we assume it exists, because it was added before!

		SELECT paramid
		INTO   vDummy
		FROM   VICparamDef
		WHERE  nodeID = vParentRefID
		AND	   name = childNodeName(vNodeName, vVersion);
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Node % cannot be a child from parent %\', 
							vNodeName, $4;
		END IF;
	  END IF;

	  -- if no newname was specified use existing one
	  IF length($5) < 2 THEN
		vNewName = vNodeName;
	  ELSE
		vNewName = $5;
	  END IF;
	  
	  -- finally copy the node (orgNode, tree, parent, newname)
	  vNewNodeID := instanciateVTleafnode($2, $3, $4, vNewName);

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;

