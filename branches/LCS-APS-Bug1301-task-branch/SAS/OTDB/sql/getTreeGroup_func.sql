--
--  getTreesInPeriod.sql: function for getting treeinfo from the OTDB
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
--  $Id: getExecutableTrees_func.sql 8438 2006-05-18 19:16:37Z overeem $
--

--
-- getTreesInPeriod (groupType, periodInMinutes)
-- 
-- groupType = 1: get all trees that are scheduled to start in the period: now till now+period
--             2: get all trees with starttime <=now and stoptime > now ; period param is ignored
--             3: get all trees with stoptime in the period: now-period till now
--
-- With this function we can get the planned, active or finished trees from the database.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--			campaign	read
--
-- Types:	treeInfo
--
CREATE OR REPLACE FUNCTION getTreeGroup(INT, INT)
  RETURNS SETOF treeInfo AS '
	DECLARE
		vRecord			RECORD;
		TSscheduled		CONSTANT	INT2 := 400;
		TSqueued		CONSTANT	INT2 := 500;
		TSfinished		CONSTANT	INT2 := 1000;
		TThierarchy		CONSTANT	INT2 := 30;
		TCoperational	CONSTANT	INT2 := 3;
		vQuery			TEXT;

	BEGIN
	  vQuery := \'\';
	  IF $1 = 1 THEN
		vQuery := \' AND (t.state = \' || TSscheduled || \' OR t.state = \' || TSqueued || \') \';
--		vQuery := vQuery || \' AND t.starttime < now()+interval \' || chr(39) || $2 || \' minutes\' || chr(39);
		vQuery := vQuery || \' AND t.starttime >= now() AND t.starttime < now()+interval \' || chr(39) || $2 || \' minutes\' || chr(39);
	  ELSE 
		IF $1 = 2 THEN
		  vQuery := \' AND t.starttime <= now() AND t.stoptime > now() \';
		  vQuery := vQuery || \' AND t.state > \' || TSscheduled;
		  vQuery := vQuery || \' AND t.state < \' || TSfinished;
	    ELSE 
		  IF $1 = 3 THEN
			vQuery := \' AND t.state >= \' || TSfinished;
			vQuery := vQuery || \' AND t.stoptime > now()-interval \' || chr(39) || $2 || \' minutes\' || chr(39);
		  ELSE
			RAISE EXCEPTION \'groupType must be 1,2 or 3 not %\', $1;
		  END IF;
		END IF;
	  END IF;

	  -- do selection
	  FOR vRecord IN EXECUTE \'
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
		WHERE  t.treetype = 30
		AND	   t.classif  = 3 
		\' || vQuery || \'
		ORDER BY t.starttime, t.treeID \'
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;



