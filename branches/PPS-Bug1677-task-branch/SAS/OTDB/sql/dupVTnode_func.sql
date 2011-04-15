--
--  dupVTnode.sql: function for creating a copy of a node in a VIC template tree
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

-- helper function
-- attachVTparameters(orgNodeID, NewNodeID)
--
-- Gives the newNode the same parameters as the orgNode.
--
-- Tables:	VICtemplate	insert
--
CREATE OR REPLACE FUNCTION attachVTparameters(INT4, INT4)
  RETURNS VOID AS '
	DECLARE
	  vParam		RECORD;

	BEGIN
	  -- get childs of originNode
	  FOR vParam IN
		SELECT	*
		FROM	VICtemplate
		WHERE	parentID = $1
				AND leaf = TRUE
	  LOOP
		-- note: nodeid is generated automatically
		INSERT INTO VICtemplate (treeid, parentid, originid,
								 name, index, leaf, 
								 instances, limits)
		VALUES (vParam.treeID, $2, vParam.originID,
				vParam.name, vParam.index, vParam.leaf, 
				vParam.instances, vParam.limits);
	  END LOOP;
	  RETURN;
	END;
' LANGUAGE plpgsql;

-- helper function
-- dupVTleafNode(orgNodeID , newParentId, newIndex)
--
-- duplicates a VICtemplate leaf node inclusive the parameters
--
-- Tables:	VICtemplate	insert
--
CREATE OR REPLACE FUNCTION dupVTleafNode(INT4, INT4, INT2)
  RETURNS INT4 AS '
	DECLARE
		vNodeID		INT4;
		vOrgNode	RECORD;

	BEGIN
	  -- get fields of original node
	  SELECT	*
	  INTO		vOrgNode
	  FROM		VICtemplate
	  WHERE		nodeID = $1;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'Node % does not exist, cannot duplicate it\', $1;
		RETURN 0;
	  END IF;

	  -- check that parent exists
	  -- ... TODO or not ...

	  -- check that duplicate does not exist yet.
	  -- ... check it or not ...
	  SELECT	nodeID
	  INTO		vNodeID		-- dummy
	  FROM		VICtemplate
	  WHERE		parentID = $2
				AND name = vOrgNode.name
				AND index = $3;
	  IF FOUND THEN
		RAISE EXCEPTION \'Node % with index % already exists\',
						vOrgNode.name, $3;
	 	RETURN 0;
	  END IF;

	  -- dedicated nodes (index>=0) can only exist once
	  IF $3 >= 0 THEN
		vOrgNode.instances := 1;
	  END IF;

	  -- add node itself
	  vNodeID := nextval(\'VICtemplateID\');
	  INSERT INTO 
	  VICtemplate (treeid, nodeID, 
				   parentID, originID,
				   name, index, leaf, 
				   instances, limits)
	  VALUES 	  (vOrgNode.treeID, vNodeID, 
			  	   $2, vOrgNode.originID,
			  	   vOrgNode.name, $3, vOrgNode.leaf, 
			  	   vOrgNode.instances, vOrgNode.limits);

	  -- and add its parameters
	  PERFORM attachVTparameters($1, vNodeID);
	  RETURN vNodeID;
	END;
' LANGUAGE plpgsql;

-- helper function
-- dupVTsubTree(orgNodeID, newParentID, newIndex)
--
-- duplicates a VICtemplate leaf node inclusive the parameters
--
-- Tables:	VICtemplate	insert
--
CREATE OR REPLACE FUNCTION dupVTsubTree(INT4, INT4, INT2)
  RETURNS INT4 AS '
	DECLARE
		vNodeID		INT4;
		vChild		RECORD;
		vDummy		INT4;
--		vIndex		INT2;

	BEGIN
		-- duplicate node and its own parameters
		-- dupVTleafNode(orgNodeID , newParentId, newIndex)
		vNodeID := dupVTleafNode($1, $2, $3);

		-- for all its children
		FOR vChild IN
		  SELECT nodeID, index
		  FROM	 VICtemplate
		  WHERE	 parentID = $1
		  		 AND leaf = FALSE
		LOOP
		  vDummy := dupVTsubTree(vChild.nodeID, vNodeID, vChild.index);
		END LOOP;

		RETURN vNodeID;
	END;
' LANGUAGE plpgsql;

--
-- dupVTnode (authToken, treeID, nodeID, index)
--
-- Creates a new node record including its parameters and childnodes
-- and returns the nodeID of this new node.
--
-- Authorisation: yes
--
-- Tables:	VICtemplate	insert
--			OTDBtree	select
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION dupVTnode(INT4, INT4, INT4, INT2)
  RETURNS INT4 AS '
	DECLARE
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vParentID		VICtemplate.parentID%TYPE;
		vNewNodeID		INT4;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, -)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
			RETURN $3;
		END IF;

		-- get ParentID of node to duplicate
		SELECT	parentID
		INTO	vParentID
		FROM	VICtemplate
		WHERE	nodeID = $3;
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Node with id % does not exist!\', $3;
		  RETURN 0;
		END IF;

		-- duplicate node and its own parameters
		-- dupVTsubTree(orgNodeID, newParentID, newIndex)
		vNewNodeID := dupVTsubTree($3, vParentID, $4);

		RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;

