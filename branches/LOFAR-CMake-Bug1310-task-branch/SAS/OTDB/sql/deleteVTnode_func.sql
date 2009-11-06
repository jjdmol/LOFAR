--
--  deleteVTnode.sql: function for deleting a (subtree) of VICtemplate nodes.
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
-- removeVTparameters(nodeID)
--
-- Removes the parameters from the given node.
--
-- Tables:	VICtemplate	delete
--
CREATE OR REPLACE FUNCTION removeVTparameters(INT4)
  RETURNS VOID AS '
	BEGIN
		DELETE
		FROM	VICtemplate
		WHERE	parentID = $1
				AND leaf = TRUE;
	  RETURN;
	END;
' LANGUAGE plpgsql;

-- helper function
-- removeVTleafNode(nodeID)
--
-- Removes a VICtemplate leaf node inclusive the parameters
--
-- Tables:	VICtemplate	delete
--
CREATE OR REPLACE FUNCTION removeVTleafNode(INT4)
  RETURNS VOID AS '
	BEGIN
		-- remove parameters
		PERFORM removeVTparameters($1);

		-- remove node itself
		DELETE
		FROM	VICtemplate
		WHERE	nodeID = $1;
	
		RETURN;
	END;
' LANGUAGE plpgsql;

-- recursive helper function
-- removeVTsubTree(nodeID)
--
-- remove a complete VICtemplate subtree
--
-- Tables:	VICtemplate	delete
--
CREATE OR REPLACE FUNCTION removeVTsubTree(INT4)
  RETURNS VOID AS '
	DECLARE
		vChild		RECORD;

	BEGIN
		-- for all its children
		FOR vChild IN
		  SELECT nodeID
		  FROM	 VICtemplate
		  WHERE	 parentID = $1
		  		 AND leaf = FALSE
		LOOP
		  PERFORM removeVTsubTree(vChild.nodeID);
		END LOOP;

		PERFORM removeVTleafNode($1);

		RETURN;
	END;
' LANGUAGE plpgsql;

--
-- removeNode (authToken, treeID, nodeID)
--
-- Removes a node record including its parameters and childnodes
--
-- Authorisation: yes
--
-- Tables:	VICtemplate	delete
--			OTDBtree	select
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION removeVTnode(INT4, INT4, INT4)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, -)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
			RETURN FALSE;
		END IF;

		-- remove node (its subtree)
		PERFORM removeVTsubTree($3);

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

