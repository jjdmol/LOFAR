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
  RETURNS SETOF templateInfo AS '
	DECLARE
		vRecord					RECORD;
		TSTemplate CONSTANT		INT2 := 10;

	BEGIN
	  -- do selection
	  FOR vRecord IN  
		SELECT treeID, 
			   name
		FROM   OTDBtree where state=TSTemplate AND name IS NOT NULL
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;

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
  RETURNS BOOLEAN AS '
	DECLARE
		TTtemplate  CONSTANT	INT2 := 20;
		TThierarchy CONSTANT	INT2 := 30;
		vFunction   CONSTANT	INT2 := 1;
		vTreeType		OTDBtree.treetype%TYPE;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, tree, func, parameter)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
			RETURN FALSE;
		END IF;

		-- get treetype
		SELECT	treetype
		INTO	vTreeType
		FROM	OTDBtree
		WHERE	treeID = $2;
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Tree % does not exist\', $2;
		END IF;

		IF vTreeType = TTtemplate THEN
			UPDATE	OTDBtree
			SET		name = $3
			WHERE	treeID = $2;
			IF NOT FOUND THEN
			  RAISE EXCEPTION \'Name % can not be assigned to templateID %\', $3, $2;
			  RETURN FALSE;
			END IF;
		ELSE
			RAISE EXCEPTION \'Tree % is not a template tree\', $2;
		END IF;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

