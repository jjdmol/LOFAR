--
--  SetSchedule.sql: function for changing the scheduling times of a tree.
--
--  Copyright (C) 2006
--  ASTRON (Netherlands Foundation for Research in Astronomy)
--  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

-- -- setSchedule (authToken, treeID, aStartTime, aStopTime)
--
-- Authorisation: yes
--
-- Tables:	otdbtree	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION setSchedule(INT4, INT4, VARCHAR(20), VARCHAR(20))
  RETURNS BOOLEAN AS $$
    --  $Id$
	DECLARE
		vFunction				INT2 := 1;
		TSactive				CONSTANT INT2 := 600;
		TThierarchy				CONSTANT INT2 := 30;
		vIsAuth					BOOLEAN;
		vAuthToken				ALIAS FOR $1;
		vCampaignID				campaign.id%TYPE;
		vTreeType				OTDBtree.treetype%TYPE;
		vState					OTDBtree.state%TYPE;
		vStartTime				timestamp;
		vStopTime				timestamp;

	BEGIN
		-- check Timestamps
		IF $3 = '' THEN
			vStartTime := NULL;
		ELSE
			vStartTime := $3;
		END IF;
		IF $4 = '' THEN
			vStopTime := NULL;
		ELSE
			vStopTime := $4;
		END IF;
		-- check authorisation(authToken, treeID, func, none)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION 'Not authorized.';
			RETURN FALSE;
		END IF;
		
		-- Only non-active VH trees can be scheduled.
		SELECT 	treetype, state
		INTO   	vTreeType, vState
		FROM	OTDBtree
		WHERE	treeID = $2;

        IF vTreeType <> TThierarchy  THEN
		  RAISE EXCEPTION 'Only VH trees can be scheduled.';
		END IF;

		IF vState = TSactive THEN
		  RAISE EXCEPTION 'Tree may not be active';
		END IF;

		-- Finally update tree
		UPDATE	OTDBtree
		SET		starttime = vStartTime,
				stoptime  = vStopTime
		WHERE	treeID = $2;

		UPDATE  vicHierarchy
		SET	  	value = vStartTime
		WHERE   treeID = $2
		AND	  	name LIKE '%.Observation.startTime';
			
		UPDATE  vicHierarchy
		SET	  	value = vStopTime
		WHERE   treeID = $2
		AND	  	name LIKE '%.Observation.stopTime';

		RETURN TRUE;
	END;
$$ LANGUAGE plpgsql;

-- -- setSchedule (authToken, treeID, aStartTime, aStopTime, insideTreeAlso)
--
-- Authorisation: yes
--
-- Tables:	otdbtree	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION setSchedule(INT4, INT4, VARCHAR(20), VARCHAR(20), BOOLEAN)
  RETURNS BOOLEAN AS $$
    --  $Id$
	DECLARE
		vFunction				INT2 := 1;
		TSactive				CONSTANT INT2 := 600;
		TThierarchy				CONSTANT INT2 := 30;
		vIsAuth					BOOLEAN;
		vAuthToken				ALIAS FOR $1;
		vCampaignID				campaign.id%TYPE;
		vTreeType				OTDBtree.treetype%TYPE;
		vState					OTDBtree.state%TYPE;
		vStartTime				timestamp;
		vStopTime				timestamp;

	BEGIN
		-- check Timestamps
		IF $3 = '' THEN
			vStartTime := NULL;
		ELSE
			vStartTime := $3;
		END IF;
		IF $4 = '' THEN
			vStopTime := NULL;
		ELSE
			vStopTime := $4;
		END IF;
		-- check authorisation(authToken, treeID, func, none)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION 'Not authorized.';
			RETURN FALSE;
		END IF;
		
		-- Only non-active VH trees can be scheduled.
		SELECT 	treetype, state
		INTO   	vTreeType, vState
		FROM	OTDBtree
		WHERE	treeID = $2;

        IF vTreeType <> TThierarchy  THEN
		  RAISE EXCEPTION 'Only VH trees can be scheduled.';
		END IF;

		IF vState = TSactive THEN
		  RAISE EXCEPTION 'Tree may not be active';
		END IF;

		-- Finally update tree
		UPDATE	OTDBtree
		SET		starttime = vStartTime,
				stoptime  = vStopTime
		WHERE	treeID = $2;

		IF $5 = true THEN
		  UPDATE  vicHierarchy
		  SET	  value = vStartTime
		  WHERE   treeID = $2
		  AND	  name LIKE '%.Observation.startTime';
			
		  UPDATE  vicHierarchy
		  SET	  value = vStopTime
		  WHERE   treeID = $2
		  AND	  name LIKE '%.Observation.stopTime';
		END IF;

		RETURN TRUE;
	END;
$$ LANGUAGE plpgsql;

