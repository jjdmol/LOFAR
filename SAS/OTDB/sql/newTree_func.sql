--
--  newTree.sql: function for creating a new entry in the OTDBtree table.
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
-- newTree (authToken, orgTree, momID, classif, treetype, state, campaign)
--
-- Creates a new tree record and returns the treeID of this new tree.
--
-- Authorisation: yes
--
-- Tables:	otdbtree	insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION newTree(INT4, INT4, INT4, INT2, INT2, INT2, INT4)
  RETURNS INT4 AS '
	DECLARE
		vFunction		INT2 := 1;
		vNewTreeID		OTDBtree.treeID%TYPE;
		vCreatorID		OTDBtree.creator%TYPE;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, treetype)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, $5::int4) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RETURN 0;
		END IF;

		-- authorized. Resolve creator
		SELECT whoIs(vAuthToken)
		INTO   vCreatorID;

		-- Finally create tree entry
		vNewTreeID := nextval(\'OTDBtreeID\');
		INSERT INTO OTDBtree (treeID,
							  originid,
							  momID,
							  classif,
							  treetype,
							  state,
							  creator,
							  campaign,
							  owner)
	    VALUES (vNewTreeID,
				$2,				-- orgTree
				$3,				-- momID
				$4,				-- classif
				$5,				-- treeType
				$6,				-- state
				vCreatorID,
				$7,				-- campaign
				vCreatorID);

		IF NOT FOUND THEN
		  RETURN 0;
		ELSE
		  PERFORM addTreeState(vNewTreeID, $3, $6, vCreatorID, \'\');
		  RETURN vNewTreeID;
		END IF;
	END;
' LANGUAGE plpgsql;

