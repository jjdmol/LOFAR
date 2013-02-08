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
-- groupType = 0: get all trees that have to be scheduled
--             1: get all trees that are scheduled to start in the period: now till now+period
--             2: get all trees with starttime <=now and stoptime > now ; period param is ignored
--             3: get all trees with stoptime in the period: now-period till now
--             4: get all trees with stoptime < now and have state >= FINISHED
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
  RETURNS SETOF treeInfo AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vRecord			RECORD;
		TSapproved		CONSTANT	INT2 := 300;
		TSscheduled		CONSTANT	INT2 := 400;
		TSqueued		CONSTANT	INT2 := 500;
--		TScompleting	CONSTANT	INT2 := 900;
		TSfinished		CONSTANT	INT2 := 1000;
		TThierarchy		CONSTANT	INT2 := 30;
		TCoperational	CONSTANT	INT2 := 3;
		vQuery			TEXT;
		vSortOrder		TEXT;

	BEGIN
	  vQuery := '';
	  vSortOrder := 't.starttime, t.treeID';
	  IF $1 = 0 THEN
		vQuery := ' AND (t.stoptime > now() OR t.stoptime IS NULL) ';
	  ELSE
	    IF $1 = 1 THEN
		  vQuery := ' AND (t.state = ' || TSscheduled || ' OR t.state = ' || TSqueued || ') ';
--		  vQuery := vQuery || ' AND t.starttime < now()+interval ' || chr(39) || $2 || ' minutes' || chr(39);
		  vQuery := vQuery || ' AND t.starttime >= now() AND t.starttime < now()+interval ' || chr(39) || $2 || ' minutes' || chr(39);
	    ELSE 
	  	  IF $1 = 2 THEN
		    vQuery := ' AND t.starttime <= now() AND t.stoptime > now() ';
		    vQuery := vQuery || ' AND t.state > ' || TSscheduled;
		    vQuery := vQuery || ' AND t.state < ' || TSfinished;
	      ELSE 
            IF $1 = 3 THEN
              vQuery := ' AND t.state >= ' || TSfinished;
              vQuery := vQuery || ' AND t.stoptime > now()-interval ' || chr(39) || $2 || ' minutes' || chr(39);
              vSortOrder := 't.stoptime, t.treeID';
            ELSE
		      IF $1 = 4 THEN
		  	    vQuery := ' AND t.state >= ' || TSapproved;
			    vQuery := vQuery || ' AND t.stoptime < now() ';
			    vSortOrder := 't.treeID';
		      ELSE
			    RAISE EXCEPTION 'groupType must be 0,1,2,3 or 4 not %', $1;
              END IF;
		    END IF;
		  END IF;
	    END IF;
	  END IF;

	  -- do selection
	  FOR vRecord IN EXECUTE '
		SELECT t.treeID, 
			   t.momID,
			   t.groupID,
			   t.classif, 
			   u.username, 
			   t.d_creation, 
			   t.modificationdate, 
			   t.treetype, 
			   t.state, 
			   t.originID, 
			   c.name, 
			   t.starttime, 
			   t.stoptime,
			   t.processType,
			   t.processSubtype,
			   t.strategy,
			   t.description
		FROM   OTDBtree t 
			   INNER JOIN OTDBuser u ON t.creator = u.userid
			   INNER JOIN campaign c ON c.ID = t.campaign
		WHERE  t.treetype = 30
		AND	   t.classif  = 3 
		' || vQuery || '
		ORDER BY ' || vSortOrder 
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
$$ LANGUAGE plpgsql;

