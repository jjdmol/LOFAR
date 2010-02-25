--
--  getCampaign.sql: gets the topnode of the tree
--
--  Copyright (C) 2010
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
--  $Id: getNode_func.sql 6634 2005-10-07 10:20:03Z overeem $
--

--
-- getCampaign(campaignID)
--
-- Gets a campaign by its internal ID
--
-- Authorisation: no
--
-- Tables:	campaign	read
--
-- Types:	campaign
--
CREATE OR REPLACE FUNCTION getCampaign(INT4)
  RETURNS campaignInfo AS '
	DECLARE
		vCampaign	RECORD;

	BEGIN
		SELECT	ID, name, title, PI, CO_I, contact
		INTO	vCampaign
		FROM	campaign
		WHERE	ID = $1;
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Campaign % does not exist\', $1;
		END IF;

		RETURN vCampaign;
	END;
' LANGUAGE plpgsql;

--
-- getCampaign(campaignname)
--
-- Gets a campaign by its name
--
-- Authorisation: no
--
-- Tables:	campaign	read
--
-- Types:	campaign
--
CREATE OR REPLACE FUNCTION getCampaign(VARCHAR(20))
  RETURNS campaignInfo AS '
	DECLARE
		vCampaign	RECORD;

	BEGIN
		SELECT	ID, name, title, PI, CO_I, contact
		INTO	vCampaign
		FROM	campaign
		WHERE	name = $1;
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Campaign % does not exist\', $1;
		END IF;

		RETURN vCampaign;
	END;
' LANGUAGE plpgsql;

--
-- getCampaignList ()
-- 
-- Get a list of campaign.
--
-- Authorisation: none
--
-- Tables:  campaign    read
--
-- Types:   campaign
--
CREATE OR REPLACE FUNCTION getCampaignList()
  RETURNS SETOF campaignInfo AS '
    DECLARE
        vRecord     RECORD;

    BEGIN
      FOR vRecord IN  
        SELECT ID, name, title, PI, CO_I, contact
        FROM   campaign 
        ORDER BY ID ASC
      LOOP
        RETURN NEXT vRecord;
      END LOOP;
      RETURN;
    END
' LANGUAGE plpgsql;

--
-- saveCampaign (ID, name, title, PI, CO_I, contact)
--
-- Saves the new values to the database
--
-- Authorisation: no
--
-- Tables:	campaign	insert/update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION saveCampaign(INT4, VARCHAR(30), VARCHAR(100), VARCHAR(80), VARCHAR(80), VARCHAR(120))
  RETURNS INT4 AS '
	DECLARE
		vID			campaign.ID%TYPE;
		vName		TEXT;
		vTitle		TEXT;
		vPI			TEXT;
		vCO_I		TEXT;
		vContact	TEXT;

	BEGIN
		-- remove single quotes
		vName    := replace($2, \'\\\'\', \'\');
		vTitle   := replace($3, \'\\\'\', \'\');
		vPI      := replace($4, \'\\\'\', \'\');
		vCO_I    := replace($5, \'\\\'\', \'\');
		vContact := replace($6, \'\\\'\', \'\');

		-- check if node exists
		IF $1 = 0 THEN
		  SELECT ID
		  INTO   vID
		  FROM   campaign
		  WHERE  name = vName;
		ELSE
		  SELECT	ID
		  INTO	vID
		  FROM	campaign
		  WHERE	ID = $1;
		END IF;
		IF (NOT FOUND) THEN
		  -- create new node
		  vID := nextval(\'campaignID\');
		  INSERT INTO campaign (id, name, title, PI, CO_I, contact)
		  VALUES	(vID, $2, $3, $4, $5, $6);
		ELSE
		  -- update node
		  UPDATE campaign
		  SET	 name = $2,
				 title = $3,
				 PI = $4,
				 CO_I = $5,
				 contact = $6
		  WHERE	 ID = vID;
		END IF;

		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Node % could not be saved\', $4;
		  RETURN 0;
		END IF;

		RETURN vID;
	END;
' LANGUAGE plpgsql;
