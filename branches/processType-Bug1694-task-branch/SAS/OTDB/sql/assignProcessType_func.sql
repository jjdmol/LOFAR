--
--  assignProcessType.sql: Assign processType, processSubtype and category to a tree.
--
--  Copyright (C) 2011
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
--  $Id:$
--


--
-- assignProcessType (auth, treeID, processType, processSubtypes, strategy)
-- 
-- Assign the given values to the tree, make sure that the combination is unique for defaultTemplates.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	write
--			otdbuser	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION assignProcessType(INT4, INT4, VARCHAR(20), VARCHAR(120), VARCHAR(30))
  RETURNS BOOLEAN AS $$
	DECLARE
		TTtemplate  CONSTANT	INT2 := 20;
		vFunction   CONSTANT	INT2 := 1;
		vTreeType			OTDBtree.treetype%TYPE;
		vName				OTDBtree.name%TYPE;
		vIsAuth				BOOLEAN;
		vDummy				OTDBtree.treeID%TYPE;
		vNodeID				INTEGER;
		vRecord				RECORD;
		aAuthToken			ALIAS FOR $1;
		aTreeID				ALIAS FOR $2;
		aProcessType		ALIAS FOR $3;
		aProcessSubtypes	ALIAS FOR $4;
		aStrategy			ALIAS FOR $5;

	BEGIN
		-- check authorisation(authToken, tree, func, parameter)
		vIsAuth := FALSE;
		SELECT isAuthorized(aAuthToken, aTreeID, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION 'Not authorized to assign the processType to tree %', aTreeID;
		END IF;

		-- get treetype
		SELECT	treetype, name
		INTO	vTreeType, vName
		FROM	OTDBtree
		WHERE	treeID = aTreeID;
		IF NOT FOUND THEN
		  RAISE EXCEPTION 'Tree % does not exist', aTreeID;
		END IF;

		-- not applicable for PIC trees.
		IF vTreeType != TTtemplate THEN
		  RAISE EXCEPTION 'Process(sub)Types can only be assigned to (default) templates.';
		END IF;

		-- check for double defaulttemplate entries
		IF vName IS NOT NULL AND aProcessType != '' THEN
			SELECT	treeID
			INTO 	vDummy
			FROM	OTDBtree
			WHERE	processType = aProcessType
			AND		processSubtypes = aProcessSubtypes
			AND		strategy = aStrategy
			AND		name IS NOT NULL;
			IF FOUND AND vDummy != aTreeID THEN
			  RAISE EXCEPTION 'There is already a defaultTemplate with the values %, %, %', 
								aProcessType, aProcessSubtypes, aStrategy;
			END IF;
		END IF;

		-- check if combination is allowed
--		SELECT	*
--		INTO 	vRecord
--		FROM	processTypes
--		WHERE	processType = aProcessType
--		AND		processSubtypes = aProcessSubtypes
--		AND		strategy = aStrategy;
--		IF NOT FOUND THEN
--		  RAISE EXCEPTION 'The combination of %, %, % for processType, processSubtypes and strategy is not allowed', 
--								aProcessType, aProcessSubtypes, aStrategy;
--		END IF;

		-- finally update the metadata info
		UPDATE	OTDBtree
		SET		processType = aProcessType,
				processSubtypes = aProcessSubtypes,
				strategy = aStrategy
		WHERE	treeID = aTreeID;

		-- and copy the info into the tree
		SELECT	nodeID
		INTO	vNodeID
		FROM	getVTitem(aTreeID, 'Observation.processType');
		UPDATE	VICtemplate
		SET		limits = aProcessType
		WHERE	nodeID = vNodeID;

		SELECT	nodeID
		INTO	vNodeID
		FROM	getVTitem(aTreeID, 'Observation.processSubtypes');
		UPDATE	VICtemplate
		SET		limits = aProcessSubtypes
		WHERE	nodeID = vNodeID;

		SELECT	nodeID
		INTO	vNodeID
		FROM	getVTitem(aTreeID, 'Observation.strategy');
		UPDATE	VICtemplate
		SET		limits = aStrategy
		WHERE	nodeID = vNodeID;

		RETURN TRUE;
	END;
$$ LANGUAGE plpgsql;
