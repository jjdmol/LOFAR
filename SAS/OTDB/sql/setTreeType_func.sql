--
--  SetTreeType.sql: function for changing the type of the tree
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
-- setTreeType (authToken, treeID, treeType)
--
-- Checks if the treetype is legal before assigning it.
--
-- Authorisation: yes
--
-- Tables:	otdbtree	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION setTreeType(INT4, INT4, INT2)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction				INT2 := 1;
		vTreeType				OTDBtree.treetype%TYPE;
		vIsAuth					BOOLEAN;
		vAuthToken				ALIAS FOR $1;
		TThardware CONSTANT		INT2 := 10;
		TTtemplate CONSTANT		INT2 := 20;
		TTobsolete CONSTANT		INT2 := 80;

	BEGIN
		-- check authorisation(authToken, treeID, func, treeType)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, $3::int4) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized.\';
			RETURN FALSE;
		END IF;

		-- check classification
		SELECT	id
		INTO	vTreeType
		FROM	treetype
		WHERE	id = $3;
		IF NOT FOUND THEN
			RAISE EXCEPTION \'Treetype % does not exist\', $3;
			RETURN FALSE;
		END IF;

		-- get current treetype. 
		-- Note: tree existance is checked during auth.check
		SELECT 	treetype
		INTO   	vTreeType
		FROM	OTDBtree
		WHERE	treeID = $2;

		-- restriction 1: hardware and template trees may only be changed
		-- to obsolete.
		IF vTreeType = TThardware OR vTreeType = TTtemplate THEN
		  IF $3 != TTobsolete THEN
			RAISE EXCEPTION \'Tree may only be changed to type obsolete.\';
			RETURN FALSE;
		  END IF;
		END IF;

		-- restriction 2: nothing can be changed into hardware of template
		IF $3 = TThardware OR $3 = TTtemplate THEN
		  RAISE EXCEPTION \'Tree can not be changed to this type.\';
		  RETURN FALSE;
		END IF;

		-- Finally update tree
		UPDATE	OTDBtree
		SET		treetype = $3
		WHERE	treeid = $2;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

