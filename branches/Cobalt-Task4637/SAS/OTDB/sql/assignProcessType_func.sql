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
-- assignProcessType (auth, treeID, processType, processSubtype, strategy)
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
CREATE OR REPLACE FUNCTION assignProcessType(INT4, INT4, VARCHAR(20), VARCHAR(50), VARCHAR(30))
  RETURNS BOOLEAN AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
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
		aProcessSubtype		ALIAS FOR $4;
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
			AND		processSubtype = aProcessSubtype
			AND		strategy = aStrategy
			AND		name IS NOT NULL;
			IF FOUND AND vDummy != aTreeID THEN
			  RAISE EXCEPTION 'There is already a defaultTemplate with the values %, %, %', 
								aProcessType, aProcessSubtype, aStrategy;
			END IF;
		END IF;

		-- check if combination is allowed
--		SELECT	*
--		INTO 	vRecord
--		FROM	processTypes
--		WHERE	processType = aProcessType
--		AND		processSubtype = aProcessSubtype
--		AND		strategy = aStrategy;
--		IF NOT FOUND THEN
--		  RAISE EXCEPTION 'The combination of %, %, % for processType, processSubtype and strategy is not allowed', 
--								aProcessType, aProcessSubtype, aStrategy;
--		END IF;

		-- finally update the metadata info
		UPDATE	OTDBtree
		SET		processType = aProcessType,
				processSubtype = aProcessSubtype,
				strategy = aStrategy
		WHERE	treeID = aTreeID;

		RETURN TRUE;
	END;
$$ LANGUAGE plpgsql;

-- 
-- INTERNAL FUNCTION
-- Copy the processTypes from one tree to another.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	write
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION copyProcessType(INT4, INT4)
  RETURNS BOOLEAN AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vProcessType		OTDBtree.processType%TYPE;
		vProcessSubtype		OTDBtree.processSubtype%TYPE;
		vStrategy			OTDBtree.strategy%TYPE;
		vResult				BOOLEAN;
		aOrgTreeID			ALIAS FOR $1;
		aDestTreeID			ALIAS FOR $2;

	BEGIN
		-- get treetype
		SELECT	processType, processSubtype, strategy
		INTO	vProcessType, vProcessSubtype, vStrategy
		FROM	OTDBtree
		WHERE	treeID = aOrgTreeID;
		IF NOT FOUND THEN
		  RAISE EXCEPTION 'Tree % does not exist', aTreeID;
		END IF;

		-- update the metadata info
		UPDATE	OTDBtree
		SET		processType = vProcessType,
				processSubtype = vProcessSubtype,
				strategy = vStrategy
		WHERE	treeID = aDestTreeID;

		RETURN TRUE;
	END;
$$ LANGUAGE plpgsql;

-- 
-- INTERNAL FUNCTION
--
-- exportProcessType(treeID, prefixLen)
--
-- Return the processType values as a linefeed separated key-value list
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportProcessType(INT4, INT4)
  RETURNS TEXT AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vResult			    TEXT := '';
		vPrefix     		TEXT;
		vProcessType		OTDBtree.processType%TYPE;
		vProcessSubtype		OTDBtree.processSubtype%TYPE;
		vStrategy			OTDBtree.strategy%TYPE;
		aTreeID				ALIAS FOR $1;
		aPrefixLen			ALIAS FOR $2;

	BEGIN
		-- get processInfo
		SELECT	processType, processSubtype, strategy
		INTO	vProcessType, vProcessSubtype, vStrategy
		FROM	OTDBtree
		WHERE	treeID = aTreeID;
		IF NOT FOUND THEN
		  RAISE EXCEPTION 'Tree % does not exist', aTreeID;
		END IF;

		SELECT substr(name, aPrefixLen)
		INTO   vPrefix
		FROM   getVHitemList(aTreeID, '%.Observation');
		vResult := vResult || vPrefix || '.processType='    || vProcessType    || chr(10);
		vResult := vResult || vPrefix || '.processSubtype=' || vProcessSubtype || chr(10);
		vResult := vResult || vPrefix || '.strategy='       || vStrategy       || chr(10);

		RETURN vResult;
	END;
$$ LANGUAGE plpgsql;

