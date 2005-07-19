--
--  instanciateTree.sql: create a full VIC tree from a template.
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
-- instanciateVHparams(orgNodeID, newTreeID, newNodeID)
-- 
-- Gives the newNodeID in the newTreeID the same parameters as the orgNode.
-- Note: new node must already exist.
--
-- Authorisation: none
--
-- Tables: 	VICtemplate		read
--			VIChierarchy	insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVHparams(INT4, INT4, INT4)
  RETURNS VOID AS '
	DECLARE
		vParam	RECORD;

	BEGIN
perform logmsg(\'VHparams:\' || $1 || \',\' || $2 || \',\' || $3 );
	  FOR vParam IN 
		SELECT	originID, name, limits
		FROM 	VICtemplate
		WHERE	parentID = $1
		AND		leaf = TRUE
	  LOOP
		INSERT 
		INTO 	VIChierarchy(treeID, parentID, paramrefID, name, value)
	    VALUES 	($2, $3, vParam.originID, vParam.name, vParam.limits);
		-- note: nodeId, index and leaf are defaulted.
	  END LOOP;
	  RETURN;
	END;
' LANGUAGE plpgsql;

--
-- helper function
-- instanciateVHleafNode(orgNodeID, newTreeID, newParentID, index):newNodeID
--
-- Duplicates a complete VT node to the VH tree
-- 
-- Authorisation: none
--
-- Tables: 	VICtemplate		read
--			VIChierarchy	insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVHleafNode(INT4, INT4, INT4, INT2)
  RETURNS INT4 AS '
	DECLARE
		vNode		RECORD;
		vNewNodeID	VICtemplate.nodeID%TYPE;

	BEGIN
perform logmsg(\'VHleaf:\' || $1 || \',\' || $2 || \',\' || $3 || \',\' || $4);
	  SELECT originID, name
	  INTO	 vNode
	  FROM	 VICtemplate
	  WHERE	 nodeID = $1;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'node % with index 0 does not exist\', $1;
		RETURN 0;
	  END IF;

	  vNewNodeID := NEXTVAL(\'VIChierarchID\');

	  INSERT
	  INTO	 VIChierarchy(treeID, nodeID, parentID, 
						  paramrefID, name, index, leaf)
	  VALUES ($2, vNewNodeID, $3, 
			  vNode.originID, vNode.name, $4, FALSE);

	  PERFORM instanciateVHparams($1, $2, vNewNodeID);

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;

--
-- helper function
-- instanciateVHsubTree(orgNodeID, newTreeID, newParentID): newNodeID
-- 
-- Duplicates a subtree starting at node orgNode to VH tree newTreeID
-- under node newParentID.
-- Recursive routine.
--
-- Authorisation: none
--
-- Tables: 	VICtemplate		read
--			VIChierarchy	insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVHsubTree(INT4, INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
	  vNode				RECORD;
	  vVTnode			RECORD;
	  vIndexCounter		INT2;
	  vNewNodeID		VIChierarchy.nodeID%TYPE;
	  vNodeID			VIChierarchy.nodeID%TYPE;
	  vDummy			VIChierarchy.nodeID%TYPE;

	BEGIN
perform logmsg(\'VHsubtree:\' || $1 || \',\' || $2 || \',\' || $3 );
	  -- get orgnode (master record: index = 0)
	  SELECT parentID, name, instances
	  INTO	 vNode
	  FROM	 VICtemplate
	  WHERE	 nodeID = $1;
	  
	  -- loop through nr of instances
	  -- check each instance if is was specialized.
	  FOR vIndexCounter IN 1 .. vNode.instances LOOP
		SELECT  nodeID
		INTO	vNodeID
		FROM	VICtemplate
		WHERE	parentID = vNode.parentID
		AND		name = vNode.name
		AND		index = vIndexCounter;
		IF NOT FOUND THEN
		  -- no specialisation, use master node (index=0)
		  vNodeID := $1;
		END IF;

		-- instanciate node itself
	    vNewNodeID := instanciateVHleafNode(vNodeID, $2, $3, vIndexCounter::int2);

		-- dive into the childen
		FOR vVTnode IN 
		  SELECT nodeID 
		  FROM	 VICtemplate
		  WHERE	 parentID = vNodeID
		  AND	 index = 0
		  AND	 leaf = FALSE
		LOOP
		  vDummy := instanciateVHsubTree(vVTnode.nodeID, $2, vNewNodeID);
		END LOOP;
	  END LOOP;

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;


--
-- instanciateVHtree (authToken, treeID):newTreeID
-- 
-- Creates a full VH tree from the given folded VT tree.
--
-- Authorisation: yes
--
-- Tables: 	VICtemplate		read
--			VIChierarchy	insert
--			OTDBtree		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVHtree(INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
		vFunction  CONSTANT		INT2 := 1;
		TTschedule CONSTANT		INT2 := 40;
		vIsAuth					BOOLEAN;
	  	vClassif				OTDBtree.classif%TYPE;
		vCampaign				OTDBtree.campaign%TYPE;
		vVTnode					RECORD;
	  	vOrgNodeID				VICtemplate.nodeID%TYPE;
	  	vNewNodeID				VICtemplate.nodeID%TYPE;
		vNewTreeID				OTDBtree.treeID%TYPE;
		vAuthToken				ALIAS FOR $1;

	BEGIN
	  -- check authorisation(authToken, treeID, func, dummy)
	  vIsAuth := FALSE;
	  SELECT isAuthorized(vAuthToken, $2, vFunction, 0)
	  INTO	 vIsAuth;
	  IF NOT vIsAuth THEN
		RAISE EXCEPTION \'Not authorized\';
	  END IF;

	  -- get some info about the original tree
	  SELECT classif, campaign
	  INTO	 vClassif, vCampaign
	  FROM	 OTDBtree
	  WHERE	 treeID = $2;
	  -- note: tree exists, checked in authorisation check

	  -- create a new tree
	  SELECT newTree($1, $2, vClassif, TTschedule, vCampaign)
	  INTO	 vNewTreeID;
	  IF vNewTreeID = 0 THEN
		RAISE EXCEPTION \'Tree can not be created\';
	  END IF;

	  -- get topNode
	  SELECT nodeID
	  INTO	 vOrgNodeID
	  FROM	 VICtemplate
	  WHERE	 treeID = $2
	  AND 	 parentID = 0;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'Topnode of tree % unknown; tree empty?\', $2;
	  END IF;

	  -- recursively instanciate the tree
	  vNewNodeID := instanciateVHsubTree(vOrgNodeID, vNewTreeID, 0);	

	  RETURN vNewTreeID;
	END;
' LANGUAGE plpgsql;

