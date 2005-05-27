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
		newTreeID		OTDBtree.treeID%TYPE;
		creatorID		OTDBtree.creator%TYPE;
		isAuth			BOOLEAN;
		authToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation
		isAuth := FALSE;
		-- TBW: function number
		SELECT isAuthorized(authToken, 1::int2, $4::int4) -- authToken, func, treetype
		INTO   isAuth;

		IF NOT isAuth THEN
			RETURN 0;
		END IF;

		-- authorized. Resolve creator
		SELECT whoIs(authToken)
		INTO   creatorID;

		-- Finally create tree entry
		newTreeID := nextval(\'OTDBtreeID\');
		INSERT INTO OTDBtree (treeID,
							  originid,
							  classif,
							  treetype,
							  creator,
							  campaign,
							  owner)
	    VALUES (newTreeID,
				$2,				-- orgTree
				$3,				-- classif
				$4,				-- treeType
				creatorID,
				$5,				-- campaign
				creatorID);

		IF NOT FOUND THEN
		  RETURN 0;
		ELSE
		  RETURN newTreeID;
		END IF;
	END;
' LANGUAGE plpgsql;

