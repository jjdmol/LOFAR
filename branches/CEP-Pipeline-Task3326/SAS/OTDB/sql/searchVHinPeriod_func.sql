--
--  searchInPeriod.sql: Search values of subtrees witihn a period
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
-- searchInPeriod (treeID, topNode, depth, begindate, enddate)
-- 
-- Get a list of values.
--
-- Authorisation: none
--
-- Tables: 	otdbtree		read
--			vichierarchy	read
--
-- Types:	OTDBvalue
--
CREATE OR REPLACE FUNCTION searchVHinPeriod(INT4, INT4, INT4, TIMESTAMP, TIMESTAMP)
  RETURNS SETOF OTDBvalue AS '
	DECLARE
		vRecord		RECORD;
		vFullname	VARCHAR(150);
		vNodename	VARCHAR(150);
		vLeaf		VIChierarchy.leaf%TYPE;
		vParamRefID	VIChierarchy.paramRefID%TYPE;
		vQuery		TEXT;

	BEGIN
--RAISE WARNING \'searchVH:%,%,%,%,%\', $1, $2, $3, $4, $5;
	  -- Is topNode a node or a parameter?
	  SELECT leaf, paramRefID, name
	  INTO	 vLeaf, vParamRefID, vFullname
	  FROM	 VIChierarchy
	  WHERE	 nodeID = $2
	  AND	 treeID = $1;

	  IF NOT FOUND THEN
		RAISE EXCEPTION \'Parameter % not found in tree %\', $2, $1;
	  END IF;

	  IF vLeaf = TRUE AND $3 = 0 THEN
		vQuery := \'=\' || chr(39) || vFullname || chr(39);
	  ELSE
	    -- construct query
		vFullname = replace(vFullname, \'[\', \'\\\\\\\\[\');
		vFullname = replace(vFullname, \']\', \'\\\\\\\\]\');
	    vQuery := \'similar to \' || chr(39) || vFullname;
	    FOR i in 1..$3 LOOP
		  vQuery := vQuery || \'.[^\\\\\\\\.]+\';
	    END LOOP;
	    vQuery := vQuery || chr(39);
	  END IF;

	  -- append query with one or two time limits
	  IF $5 IS NULL
	  THEN
	    vQuery := vQuery || \' AND k.time > \' || chr(39) || $4 || chr(39);
	  ELSE
	    vQuery := vQuery || \' AND k.time BETWEEN \' 
						 || chr(39) || $4 || chr(39) 
				   		 || \' AND \' || chr(39) || $5 || chr(39);
	  END IF;
--RAISE WARNING \'Query:%\', vQuery;
	  -- get record in paramref table
	  FOR vRecord IN EXECUTE \'
	    SELECT h.nodeID,
			   k.paramname,
			   k.value,
			   k.time
		FROM   VICkvt k
			   INNER JOIN VIChierarchy h ON k.paramname = h.name
	    WHERE  k.paramname \' || vQuery || \'
		AND	   k.treeID = \' || $1 || \'
		ORDER BY h.nodeID, k.time\'
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;


