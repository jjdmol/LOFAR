--
--  getDefaultTemplate.sql: function for getting treeinfo from the OTDB
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
--  $Id: getTreeList_func.sql 9997 2007-03-01 12:21:46Z overeem $
--

--
-- getDefaultTemplates ()
-- 
-- Get a list of treeID's of the default template trees.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--			campaign	read
--
-- Types:	templateInfo
--
CREATE OR REPLACE FUNCTION getDefaultTemplates()
  RETURNS SETOF templateInfo AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vRecord		RECORD;
		TSobsolete	CONSTANT	INT2 := 1200;

	BEGIN
	  -- do selection
	  FOR vRecord IN  
		SELECT treeID, 
			   name,
			   processType,
			   processSubtype,
			   strategy
		FROM   OTDBtree 
		WHERE  name IS NOT NULL
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
$$ LANGUAGE plpgsql;

--
-- assignTemplateName (auth, treeID, name)
-- 
-- Connect the given name to the given templateID
--
-- Authorisation: none
--
-- Tables: 	otdbtree	write
--			otdbuser	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION assignTemplateName(INT4, INT4, VARCHAR(32))
  RETURNS BOOLEAN AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		TSobsolete   		CONSTANT	INT2 := 1200;
		TTtemplate  		CONSTANT	INT2 := 20;
		TThierarchy 		CONSTANT	INT2 := 30;
		vFunction   		CONSTANT	INT2 := 1;
		vTreeType			OTDBtree.treetype%TYPE;
		vProcessType		OTDBtree.processType%TYPE;
		vprocessSubtype		OTDBtree.processSubtype%TYPE;
		vStrategy			OTDBtree.strategy%TYPE;
		vIsAuth				BOOLEAN;
		vDummy				INTEGER;
		vAuthToken			ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, tree, func, parameter)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
		  RAISE EXCEPTION 'Not authorized';
		  RETURN FALSE;
		END IF;

		-- get treetype
		SELECT	treetype, processType, processSubtype, strategy
		INTO	vTreeType, vProcessType, vprocessSubtype, vStrategy
		FROM	OTDBtree
		WHERE	treeID = $2;
		IF NOT FOUND THEN
		  RAISE EXCEPTION 'Tree % does not exist', $2;
		  RETURN FALSE;
		END IF;

		IF vTreeType != TTtemplate THEN
		  RAISE EXCEPTION 'Tree % is not a template tree', $2;
		  RETURN FALSE;
		END IF;

		-- check for double defaulttemplate entries
		IF $3 IS NOT NULL AND vProcessType != '' THEN
			SELECT	treeID
			INTO 	vDummy
			FROM	OTDBtree
			WHERE	processType = vProcessType
			AND		processSubtype = vprocessSubtype
			AND		strategy = vStrategy
			AND		name IS NOT NULL
			AND		state < TSobsolete;
			IF FOUND AND vDummy != $2 THEN
			  RAISE EXCEPTION 'There is already a defaultTemplate with the same processType setting.';
			  RETURN FALSE;
			END IF;
		END IF;

		UPDATE	OTDBtree
		SET		name = $3
		WHERE	treeID = $2;
		IF NOT FOUND THEN
		  RAISE EXCEPTION 'Name % can not be assigned to templateID %', $3, $2;
		  RETURN FALSE;
		END IF;

		RETURN TRUE;
	END;
$$ LANGUAGE plpgsql;

