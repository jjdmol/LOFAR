--
--  SetMomInfo.sql: function for changing the Mom information of a tree
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
-- setMomInfo (authToken, treeID, MomID, campaign)
--
-- Authorisation: yes
--
-- Tables:	otdbtree	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION setMomInfo(INT4, INT4, INT4, TEXT)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction				INT2 := 1;
		vIsAuth					BOOLEAN;
		vAuthToken				ALIAS FOR $1;
		vCampaignID				campaign.id%TYPE;

	BEGIN
		-- check authorisation(authToken, treeID, func, none)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized.\';
			RETURN FALSE;
		END IF;

		-- Translate campaign information
		SELECT id
		FROM   campaign
		INTO   vCampaignID
		WHERE  name = $4;
		IF NOT FOUND THEN
		  INSERT INTO campaign(name)
		  VALUES	  ($4);
		  
		  SELECT id
		  FROM   campaign
		  INTO 	 vCampaignID
		  WHERE	 name = $4;

		  IF NOT FOUND THEN
			RAISE EXCEPTION \' Cannot add campaign information\';
			RETURN FALSE;
		  END IF;
		END IF;

		-- Finally update tree
		UPDATE	OTDBtree
		SET		momID = $3,
				campaign = vCampaignID
		WHERE	treeID = $2;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

