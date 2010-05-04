--
--  searchTreeInPeriod.sql: function for getting treeinfo from the OTDB
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
--  $Id: getTreeList_func.sql 9997 2007-03-01 12:21:46Z overeem $
--

--
-- getTreesInPeriod (treeType, startDate, endDate)
-- 
-- Get a list of trees.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--			campaign	read
--
-- Types:	treeInfo
--
CREATE OR REPLACE FUNCTION getTreesInPeriod(INT2, TIMESTAMP, TIMESTAMP)
  RETURNS SETOF treeInfo AS '
	DECLARE
		vRecord		RECORD;
		vQuery		TEXT;

	BEGIN
	  -- Construct where clause
      vQuery := \'WHERE t.treetype = \' || chr(39) || $1 || chr(39) || \' AND ((t.starttime <= \';
	  vQuery := vQuery || chr(39) || $3 || chr(39) || \' AND t.stoptime >= \' || chr(39) || $2 || chr(39) || \')\';
	  vQuery := vQuery || \' OR t.starttime IS NULL OR t.stoptime IS NULL)\';
	  -- do selection
	  FOR vRecord IN  EXECUTE \'
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
		\' || vQuery || \'
		ORDER BY t.treeID ASC\'
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;

