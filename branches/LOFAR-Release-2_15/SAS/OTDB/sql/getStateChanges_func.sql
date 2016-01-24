--
--  getStateChanges.sql: function for getting state changes in a given period
--
--  Copyright (C) 2015
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
--  $Id: getStateList_func.sql 30919 2015-02-05 15:26:22Z amesfoort $
--

--
-- getStateChanges(begindate, enddate)
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
DROP TYPE IF EXISTS	stateChange		CASCADE;
CREATE TYPE stateChange AS (
    --  $Id: create_types.sql 30919 2015-02-05 15:26:22Z amesfoort $
	treeID			INT4,			-- OTDBtree.treeID%TYPE,
	momID			INT4,			-- OTDBtree.momID%TYPE,
	state			INT2,			-- treestate.ID%TYPE,
	username		VARCHAR(20),	-- OTDBuser.username%TYPE,
	modtime			timestamp(0),
    creation		timestamp(6)
);

CREATE OR REPLACE FUNCTION getStateChanges(TIMESTAMP(6), TIMESTAMP(6))
  RETURNS SETOF stateChange AS '
    --  $Id: getStateList_func.sql 30919 2015-02-05 15:26:22Z amesfoort $
	DECLARE
		vRecord		RECORD;
		vQuery		TEXT;
		vKeyField	TEXT;

	BEGIN
	  -- create query for treeID (when filled)
	  vQuery := \'WHERE \';
	  IF $1 IS NOT NULL THEN
	    IF $2 IS NULL THEN
		  vQuery := vQuery || \'s.creation >\' || chr(39) || $1 || chr(39);
		ELSE
		  vQuery := vQuery || \'s.creation >\' || chr(39) || $1 || chr(39)
					   || \' AND s.creation <=\' || chr(39) || $2 || chr(39);
		END IF;
	  END IF;

	  -- do selection
	  FOR vRecord IN  EXECUTE \'
		SELECT s.treeID, 
			   s.momID,
			   s.state, 
			   u.username, 
			   s.timestamp,
               s.creation
		FROM   StateHistory s 
			   INNER JOIN OTDBuser u ON s.userid = u.userid
		\' || vQuery || \'
		ORDER BY s.creation ASC\'
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;



