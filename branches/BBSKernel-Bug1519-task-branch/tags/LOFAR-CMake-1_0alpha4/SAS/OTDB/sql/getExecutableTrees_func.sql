--
--  getExecutableTrees.sql: function for getting treeinfo from the OTDB
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
-- getExecutableTrees (state)
-- 
-- Get a list of trees the is ready for execution or
-- is already in execution.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--			campaign	read
--
-- Types:	treeInfo
--
CREATE OR REPLACE FUNCTION getExecutableTrees(INT2)
  RETURNS SETOF treeInfo AS '
	DECLARE
		vRecord			RECORD;
		TSscheduled		CONSTANT	INT2 := 400;
		TSactive		CONSTANT	INT2 := 600;
		TThierarchy		CONSTANT	INT2 := 30;

	BEGIN
	  -- do selection
	  FOR vRecord IN 
		SELECT t.treeID, 
			   t.momID,
			   t.classif, 
			   u.username, 
			   t.d_creation, 
			   t.treetype, 
			   t.state, 
			   t.originID, 
			   c.name, 
			   t.starttime, 
			   t.stoptime,
			   t.description
		FROM   OTDBtree t 
			   INNER JOIN OTDBuser u ON t.creator = u.userid
			   INNER JOIN campaign c ON c.ID = t.campaign
		WHERE  t.treetype = TThierarchy
		AND	   t.classif  = $1 
		AND	   t.state between TSscheduled AND TSactive
		AND	   t.stoptime > now()
		ORDER BY t.starttime, t.treeID
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;



