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
-- newTree (authToken, orgTree, classif, treetype, campaign)
--
-- Creates a new tree record and returns the treeID of this new tree.
--
-- Authorisation: yes
--
-- Tables:	otdbtree	insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION newTree(INT4, INT4, INT2, INT2, INT4)
  RETURNS INT4 AS '
	DECLARE
		vNewTreeID		OTDBtree.treeID%TYPE;
		vCreatorID		OTDBtree.creator%TYPE;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation
		vIsAuth := FALSE;
		-- TBW: function number
		-- authToken, func, treetype
		SELECT isAuthorized(vAuthToken, 1::int2, $4::int4) 
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
							  classif,
							  treetype,
							  creator,
							  campaign,
							  owner)
	    VALUES (vNewTreeID,
				$2,				-- orgTree
				$3,				-- classif
				$4,				-- treeType
				vCreatorID,
				$5,				-- campaign
				vCreatorID);

		IF NOT FOUND THEN
		  RETURN 0;
		ELSE
		  RETURN vNewTreeID;
		END IF;
	END;
' LANGUAGE plpgsql;

