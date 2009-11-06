--
--  buildTemplateTree.sql: create a VIC template from the component database
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
-- instanciateVTparams(orgNodeID, newTreeID, newNodeID)
-- 
-- Gives the newNodeID in the newTreeID the same parameters as the orgNode.
-- Note: new node must already exist.
--
-- Authorisation: none
--
-- Tables: 	VICparamdef		read
--			VICtemplate		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVTparams(INT4, INT4, INT4)
  RETURNS VOID AS '
	DECLARE
		vParam		RECORD;
		dfltValue	VARCHAR(200);

	BEGIN
--RAISE WARNING \'params:%,%,%\', $1, $2, $3;
	  FOR vParam IN 
		SELECT	paramID, name, limits, par_type
		FROM 	VICparamDef
		WHERE	nodeID = $1
		AND		name NOT like \'#%\'
	  LOOP
		IF vParam.par_type >= 300 THEN	-- popup parameter? leave limits fiels empty.
		  dfltValue := substring(vParam.limits from \'[a-zA-Z0-9_.,<>]+;([a-zA-Z0-9_.,<>]+)\');
		  IF dfltValue IS NULL THEN
			dfltValue := \'\';
		  END IF;
--RAISE WARNING \'X:%,%\', vParam.name, dfltValue;
		  INSERT
		  INTO 	 VICtemplate(treeID, parentID, originID, name, instances, limits)
		  VALUES ($2, $3, vParam.paramID, vParam.name, 1, dfltValue);
		  -- note: nodeId, index and leaf are defaulted.
		ELSE
		  INSERT
		  INTO 	 VICtemplate(treeID, parentID, originID, name, instances, limits)
		  VALUES ($2, $3, vParam.paramID, vParam.name, 1, vParam.limits);
		  -- note: nodeId, index and leaf are defaulted.
		END IF;
	  END LOOP;
	  RETURN;
	END;
' LANGUAGE plpgsql;

--
-- helper function
-- instanciateVTleafNode(orgNodeID, newTreeID, newParentID):newNodeID
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
CREATE OR REPLACE FUNCTION instanciateVTleafNode(INT4, INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
		vNode		RECORD;
		vNewNodeID	VICtemplate.nodeID%TYPE;

	BEGIN
--RAISE WARNING \'leafNode:%,%,%\', $1, $2, $3;
	  SELECT	nodeID, name, constraints
	  INTO		vNode
	  FROM 		VICnodeDef
	  WHERE		nodeID = $1;

	  vNewNodeID := nextval(\'VICtemplateID\');
	  INSERT 
	  INTO	 VICtemplate(treeID, nodeID, parentID, originID, 
						 name, leaf, instances, limits)
	  VALUES ($2, vNewNodeID, $3, vNode.nodeID,  
			  vNode.name, false, 1, vNode.constraints);
	  -- note: nodeId and index are defaulted.

	  PERFORM instanciateVTparams($1, $2, vNewNodeID);

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;


-- helper function
-- instanciateVTsubTree(orgNodeID, newTreeID, newParentID): newNodeID
-- 
-- Duplicates a subtree starting at node orgNode to VT tree newTreeID
-- under node newParentID.
-- Recursive routine.
--
-- Authorisation: none
--
-- Tables: 	VICnodedef		read
--			VICparamdef		read
--			VICtemplate		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVTsubTree(INT4, INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
	  vNode				RECORD;
	  vNodeID			VICnodeDef.nodeID%TYPE;
	  vNewNodeID		VICtemplate.nodeID%TYPE;
	  vDummy			VICtemplate.nodeID%TYPE;
	  vVersionNr		INT4;

	BEGIN
--RAISE WARNING \'subTree:%,%,%\', $1, $2, $3;

	  -- copy node itself
	  vNewNodeID := instanciateVTleafNode($1, $2, $3);

	  -- loop through children
	  FOR vNode IN
		SELECT  name
		FROM	VICparamDef
		WHERE	nodeID = $1
		AND		name like \'#%\'
	  LOOP
		vVersionNr := getVersionNr(vNode.name);
		vNode.name := cleanNodeName(vNode.name);
--RAISE WARNING \'subTree2:%,%\', vVersionNr, vNode.name;
		-- translate name and versionnumber into node
		SELECT nodeID
		INTO   vNodeID
		FROM   VICnodeDef
		WHERE  name = vNode.name
		AND	   version = vVersionNr;
		vDummy := instanciateVTsubTree(vNodeID, $2, vNewNodeID);
	  END LOOP;

	  RETURN vNewNodeID;
	END;
' LANGUAGE plpgsql;


--
-- instanciateVTtree (authToken, componentID, classif):newTreeID
-- 
-- Creates a full VT tree from the given top component
--
-- Authorisation: yes
--
-- Tables: 	VICnodedef		read
--			VICparamdef		read
--			VICtemplate		insert
--			OTDBtree		insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION instanciateVTtree(INT4, INT4, INT2)
  RETURNS INT4 AS '
	DECLARE
		vFunction   CONSTANT	INT2 := 1;
		TTtemplate  CONSTANT	INT2 := 20;
		TSbeingspec CONSTANT	INT2 := 100;
		vIsAuth					BOOLEAN;
	  	vOrgNodeID				VICtemplate.nodeID%TYPE;
	  	vNewNodeID				VICtemplate.nodeID%TYPE;
		vNewTreeID				OTDBtree.treeID%TYPE;
		vAuthToken				ALIAS FOR $1;

	BEGIN
	  -- check authorisation(authToken, treeID, func, dummy)
	  vIsAuth := FALSE;
	  SELECT isAuthorized(vAuthToken, 0, vFunction, 0)
	  INTO	 vIsAuth;
	  IF NOT vIsAuth THEN
		RAISE EXCEPTION \'Not authorized\';
	  END IF;

	  -- create a new tree(auth, ..., classif, treetype, campaign)
	  SELECT newTree($1, 0, 0, $3, TTtemplate, TSbeingspec, 0)
	  INTO	 vNewTreeID;
	  IF vNewTreeID = 0 THEN
		RAISE EXCEPTION \'Tree can not be created\';
	  END IF;

	  -- get topNode
	  SELECT nodeID
	  INTO	 vOrgNodeID
	  FROM	 VICnodeDef
	  WHERE	 nodeID = $2;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'Component % is unknown\', $2;
	  END IF;

	  -- recursively instanciate the tree
	  vNewNodeID := instanciateVTsubTree($2, vNewTreeID, 0);	

	  RETURN vNewTreeID;
	END;
' LANGUAGE plpgsql;

