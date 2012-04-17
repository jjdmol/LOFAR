--
--  setDescription.sql: function for changing the description of a tree
--
--  Copyright (C) 2006
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
-- setDescription (authToken, treeID, description)
--
-- Authorisation: yes
--
-- Tables:	otdbtree	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION setDescription(INT4, INT4, TEXT)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction				INT2 := 1;
		vIsAuth					BOOLEAN;
		vAuthToken				ALIAS FOR $1;
		vCampaignID				campaign.id%TYPE;
		vDescription			TEXT;

	BEGIN
		-- check authorisation(authToken, treeID, func, none)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized.\';
			RETURN FALSE;
		END IF;

		-- update the tree
		vDescription := replace($3, \'\\\'\', \'\');
		UPDATE	OTDBtree
		SET		description = vDescription
		WHERE	treeID = $2;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;
