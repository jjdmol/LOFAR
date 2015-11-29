--
--  getTreeGroup.sql: function for getting treeinfo from the OTDB
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
--  $Id: getExecutableTrees_func.sql 8438 2006-05-18 19:16:37Z overeem $
--

--
-- getTreeGroup (groupType, periodInMinutes, cluster)
-- 
-- groupType = 0: get all trees that have to be scheduled
--             1: get all trees that are scheduled to start in the period: now till now+period
--             2: get all trees with starttime <=now and stoptime > now ; period param is ignored
--             3: get all trees with stoptime in the period: now-period till now
--             4: get all trees with stoptime < now and have state >= APPROVED
--
-- cluster : '' Does not include limitation to the selection.
--           'CEP2' Returns all PIPELINES that belong to the groupType and that are assigned to CEP2
--           '!CEP2' Returns all trees that belongs to the groupType EXCEPT the PIPELINES that are assigned to CEP2
--      Other values are also allowed like CEP4 en !CEP4
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
CREATE OR REPLACE FUNCTION getTreeGroup(INT, INT, VARCHAR(20))
  RETURNS SETOF treeInfo AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vRecord			RECORD;
		TSapproved		CONSTANT	INT2 := 300;
		TSscheduled		CONSTANT	INT2 := 400;
		TSqueued		CONSTANT	INT2 := 500;
		TScompleting	CONSTANT	INT2 := 900;
		TSfinished		CONSTANT	INT2 := 1000;
		TThierarchy		CONSTANT	INT2 := 30;
		TCoperational	CONSTANT	INT2 := 3;
		vWhere			TEXT;
		vQuery			TEXT;
		vSortOrder		TEXT;
        vExcept         TEXT;
        vCluster        TEXT;

	BEGIN
	  vWhere := '';
      vQuery := 'SELECT v.* FROM VICtrees v';
	  vSortOrder := 't.starttime, t.treeID';
      vExcept := '';
	  IF $1 = 0 THEN
		vWhere := ' AND (t.stoptime > now() OR t.stoptime IS NULL) ';
	  ELSE
	    IF $1 = 1 THEN
		  vWhere := ' AND (t.state = ' || TSscheduled || ' OR t.state = ' || TSqueued || ') ';
		  vWhere := vWhere || ' AND t.starttime >= now() AND t.starttime < now()+interval ' || chr(39) || $2 || ' minutes' || chr(39);
	    ELSE 
	  	  IF $1 = 2 THEN
		    vWhere := ' AND t.state > ' || TSscheduled || ' AND t.state < ' || TScompleting;
            vWhere := vWhere || ' AND t.stoptime>now()-interval ' || chr(39) || $2 || ' minutes' || chr(39);
	      ELSE 
            IF $1 = 3 THEN
              vWhere := ' AND t.state >= ' || TScompleting;
              vWhere := vWhere || ' AND t.stoptime > now()-interval ' || chr(39) || $2 || ' minutes' || chr(39);
              vSortOrder := 't.stoptime, t.treeID';
            ELSE
		      IF $1 = 4 THEN
		  	    vWhere := ' AND t.state >= ' || TSapproved;
			    vWhere := vWhere || ' AND t.stoptime < now() ';
			    vSortOrder := 't.treeID';
		      ELSE
			    RAISE EXCEPTION 'groupType must be 0,1,2,3 or 4 not %', $1;
              END IF;
		    END IF;
		  END IF;
	    END IF;
	  END IF;
      IF $3 != '' THEN
        vExcept := '
          SELECT x.* FROM VICtrees x INNER JOIN VIChierarchy h USING(treeid)
          WHERE  x.processtype = \'Pipeline\' 
                 AND h.name = \'LOFAR.ObsSW.Observation.Cluster.ProcessingCluster.clusterName\'
                 AND h.value='|| chr(39);

        IF LEFT($3,1) = '!' THEN
           vCluster := substring($3 from 2);
           vExcept := ' EXCEPT ' || vExcept || vCluster || chr(39);
        ELSE
           vCluster := $3;
           vQuery := vExcept || vCluster || chr(39);
           vExcept := '';
        END IF;
      END IF;

	  -- do selection
	  FOR vRecord IN EXECUTE '
        WITH VICtrees AS (
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
          AND    t.classif  = 3
		         ' || vWhere || '
		  ORDER BY ' || vSortOrder || ')
          ' || vQuery || vExcept 
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
$$ LANGUAGE plpgsql;

