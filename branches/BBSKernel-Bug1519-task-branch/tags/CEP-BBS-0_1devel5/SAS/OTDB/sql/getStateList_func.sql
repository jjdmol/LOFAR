--
--  getStateList.sql: function for getting state history from (a) tree(s)
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
-- getStateList (treeID, isMomID, begindate, enddate)
-- 
-- Get a list of statechanges.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--
-- Types:	treeState
--
CREATE OR REPLACE FUNCTION getStateList(INT4, BOOLEAN, TIMESTAMP, TIMESTAMP)
  RETURNS SETOF stateInfo AS '
	DECLARE
		vRecord		RECORD;
		vQuery		TEXT;
		vKeyField	TEXT;

	BEGIN
	  -- Construct where clause
	  IF $2 = TRUE THEN
		vKeyField := \'s.momID\';
	  ELSE
		vKeyField := \'s.treeID\';
	  END IF;
	
	  -- create query for treeID (when filled)
	  vQuery := \'\';
	  IF $1 > 0 THEN
	    -- add selection on treeID
	    vQuery := \'WHERE \' || vKeyField || \' =\' || chr(39) || $1 || chr(39);
	    IF $3 IS NOT NULL THEN
		    vQuery := vQuery || \' AND \';
	    END IF;
	  END IF;

	  -- append query with timestamp restriction(s)
	  IF $3 IS NOT NULL THEN
		IF vQuery = \'\' THEN
		  vQuery := \'WHERE \';
		END IF;
	    IF $4 IS NULL THEN
		  vQuery := vQuery || \'s.timestamp >=\' || chr(39) || $3 || chr(39);
		ELSE
		  vQuery := vQuery || \'s.timestamp >=\' || chr(39) || $3 || chr(39)
					   || \' AND s.timestamp <\' || chr(39) || $4 || chr(39);
		END IF;
	  END IF;

	  -- do selection
	  FOR vRecord IN  EXECUTE \'
		SELECT s.treeID, 
			   s.momID,
			   s.state, 
			   u.username, 
			   s.timestamp 
		FROM   StateHistory s 
			   INNER JOIN OTDBuser u ON s.userid = u.userid
		\' || vQuery 
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;



