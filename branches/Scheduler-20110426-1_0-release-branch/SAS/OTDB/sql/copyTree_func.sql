--
--  copyTree.sql: Copy a complete tree
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
-- copyTree (authToken, orgTree)
--
-- Copy a complete tree
--
-- Authorisation: yes
--
-- Tables:	otdbtree		insert
--			VICtemplate		insert [optional]
--			VIChierarchy	insert [optional]
--			PIChierarchy	insert [optional]
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyTree(INT4, INT4)
  RETURNS INT4 AS '
	DECLARE
		vFunction		INT2 := 1;
		TThardware 		CONSTANT INT2 := 10;
		TTtemplate 		CONSTANT INT2 := 20;
		vNewTreeID		OTDBtree.treeID%TYPE;
		vCreatorID		OTDBtree.creator%TYPE;
		vIsAuth			BOOLEAN;
		vOldTree		RECORD;
		vTopNode		VICtemplate.nodeid%TYPE;
		vNewTopNode		VICtemplate.nodeid%TYPE;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, -)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RETURN 0;
		END IF;

		-- authorized. Resolve creator
		SELECT whoIs(vAuthToken)
		INTO   vCreatorID;

		-- Get info from old tree and check type
		SELECT	*
		INTO	vOldTree
		FROM	OTDBtree
		WHERE	treeid = $2;
		IF vOldTree.treetype = TThardware THEN
		  RAISE EXCEPTION \'PIC trees cannot be copied\';
		END IF;

		-- make new tree entry, dont copy momID
		vNewTreeID := 0;
		SELECT  newTree(vAuthToken, $2, 0, vOldTree.classif,
					    vOldTree.treetype, vOldTree.state,
					    vOldTree.campaign)
		INTO    vNewTreeID;
		IF vNewTreeID = 0 THEN
		  RAISE EXCEPTION \'Creating of new treeEntry failed\'\;
		END IF;

		-- also copy the timestamps
		UPDATE	OTDBTree
		SET		starttime = vOldTree.starttime,
				stoptime  = vOldTree.stoptime
		WHERE	treeID	  = vNewTreeID;

		-- copy tree
		IF vOldTree.treetype = TTtemplate THEN
		  SELECT nodeID
		  INTO	 vTopNode
		  FROM	 VICtemplate
		  WHERE	 treeID = vOldTree.treeID
		  AND	 parentID = 0;
		  IF NOT FOUND THEN
			RAISE EXCEPTION \'Tree to be copied is empty!\';
		  END IF;

		  vNewTopNode := copyVTsubTree(vTopNode, vNewTreeID, 0);	
		ELSE
		  SELECT nodeID
		  INTO	 vTopNode
		  FROM	 VIChierarchy
		  WHERE	 treeID = vOldTree.treeID
		  AND	 parentID = 0;
		  IF NOT FOUND THEN
			RAISE EXCEPTION \'Tree to be copied is empty!\';
		  END IF;

		  vNewTopNode := copyVHsubTree(vTopNode, vNewTreeID, 0);	
		END IF;

		RETURN vNewTreeID;
	END;
' LANGUAGE plpgsql;

