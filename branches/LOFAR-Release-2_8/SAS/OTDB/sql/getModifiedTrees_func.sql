--
--  getModifiedTrees.sql: function for getting treeinfo from the OTDB
--
--  Copyright (C) 2012
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
--  $Id: getTreeList_func.sql 18624 2011-07-27 15:34:51Z schoenmakers $
--

--
-- getModifiedTrees (after, [treeType])
-- 
-- Get a list of trees that is modified after given timestamp.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--			campaign	read
--
-- Types:	treeInfo
--
CREATE OR REPLACE FUNCTION getModifiedTrees(VARCHAR(20), INT2)
  RETURNS SETOF treeInfo AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vRecord		RECORD;
		vQuery		TEXT;

	BEGIN
	  -- Construct where clause
	  vQuery := 'WHERE t.modificationDate >' || chr(39) || $1 || chr(39);
      IF $2 > 0 THEN
	    vQuery := vQuery || ' AND t.treetype = ' || $2;
      END IF;

	  -- do selection
	  FOR vRecord IN  EXECUTE '
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
		' || vQuery || ' AND t.name IS NULL
		ORDER BY t.treeID DESC'
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
$$ LANGUAGE plpgsql;

