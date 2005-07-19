--
--  getPICitemList.sql: function for getting some layers of a PIC tree
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
-- getPICitemList (treeID, topNode, depth)
-- 
-- Get a list of items.
--
-- Authorisation: none
--
-- Tables: 	picparamref		read
--			pichierarchy	read
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION getPICitemList(INT4, INT4, INT4)
  RETURNS SETOF OTDBnode AS '
	DECLARE
		vRecord		RECORD;
		vFullname	VARCHAR(120);
		vNodename	VARCHAR(100);
		vQuery		VARCHAR(100);
		i			INTEGER;

	BEGIN
	  -- Find name of parameter
	  SELECT r.pvssname
	  INTO	 vFullname
	  FROM	 PICparamRef r, PIChierarchy h
	  WHERE	 h.nodeid = $2
	  AND	 r.paramid = h.paramRefID;
	  IF NOT FOUND THEN
		RETURN;
	  END IF;

	  -- take name of node
	  vNodename := split_part(vFullname, \'.\', 1);

	  -- construct query
	  vQuery := chr(39) || vNodename;
	  FOR i in 1..$3 LOOP
		vQuery := vQuery || \'[\\\\\\\\_][^\\\\\\\\_]+\';
	  END LOOP;
	  vQuery := vQuery || chr(39);

	  -- finally get result
	  FOR vRecord IN EXECUTE \'
	    SELECT h.nodeid,
			   h.parentid, 
			   h.paramrefid,
			   h.name, 
			   h.index, 
			   h.leaf,
			   1::int2,
			   \\\'\\\'::text,
			   r.description 
		FROM   PICparamref r
			   INNER JOIN PIChierarchy h ON r.paramid = h.paramrefID
		WHERE  h.treeID = \' || $1 || \'
	    AND	   r.pvssname similar to \' || vQuery 
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;



