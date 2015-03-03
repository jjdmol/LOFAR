--
--  addTreeState.sql: Add a state change to the history
--
--  Copyright (C) 2005
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

--
-- addTreeState (treeID, momID, newState, userID, timestr)
--
-- Expected timeformat YYYY Mon DD HH24:MI:SS.MS
--
-- Authorisation: no
-- 
-- Tables:	StateHistory	insert
--			user			read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION addTreeState (INT4, INT4, INT2, INT4, VARCHAR(20))
  RETURNS VOID AS '
    --  $Id$
	DECLARE
		vTime		timestamp := NULL;

	BEGIN
	  -- convert timestamp
	  IF LENGTH($5) > 0 THEN
		  vTime := to_timestamp($5, \'YYYY-Mon-DD HH24:MI:SS.US\');
	  END IF;
	  IF vTime IS NULL THEN
		vTime := now();
	  END IF;

	  -- add value
	  INSERT INTO StateHistory(treeID, momID, state, userID, timestamp)
  	  VALUES ($1, $2, $3, $4, vTime);
	  RETURN;
	END;
' LANGUAGE plpgsql;

