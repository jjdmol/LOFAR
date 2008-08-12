--
--  getTreeList.sql: function for getting treeinfo from the OTDB
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
-- getTreeList (treeType, classification)
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
CREATE OR REPLACE FUNCTION getTreeList(INT2, INT2)
  RETURNS SETOF treeInfo AS '
	DECLARE
		vRecord		RECORD;
		vQuery		TEXT;

	BEGIN
	  -- Construct where clause
	  IF $1 = 0 AND $2 = 0 THEN
		-- parameters are both zero, select all records
	    vQuery := \'WHERE t.treeID != 0\';
	  ELSE
	    vQuery := \'WHERE \';
	    IF $1 > 0 THEN
		  -- add selection on treeType
	      vQuery := vQuery || \'t.treetype = \' || chr(39) || $1 || chr(39);
	      IF $2 > 0 THEN
		    vQuery := vQuery || \' AND \';
	      END IF;
	    END IF;
	    IF $2 > 0 THEN
		  -- add selection on classification
	      vQuery := vQuery || \'t.classif = \' || chr(39) || $2 || chr(39);
	    END IF;
	  END IF;

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
		ORDER BY t.treeID DESC\'
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;



