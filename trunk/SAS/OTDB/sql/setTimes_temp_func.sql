--
--  SetTimes.sql: TEST function for setting schedule times in OTDBtree
--				  and of the fields Observation.startTime/stopTime!
--  NOTE: function is for testing purposes and will NOT be available
--		  in the production environment!
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
-- setTimes (treeID, aStartTime, aStopTime)
--
-- Authorisation: no
--
-- Tables:	otdbtree		update
--			vichierarchy	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION setTimes(INT4, TIMESTAMP, TIMESTAMP)
  RETURNS BOOLEAN AS '
	DECLARE
		vBool					BOOLEAN;

	BEGIN
		SELECT * from setSchedule(1, $1, $2, $3)
		INTO	vBool;

		UPDATE	vicHierarchy
		SET		value = $2
		WHERE	treeID = $1
		AND		name LIKE \'%.Observation.startTime\';

		UPDATE	vicHierarchy
		SET		value = $3
		WHERE	treeID = $1
		AND		name LIKE \'%.Observation.stopTime\';

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

