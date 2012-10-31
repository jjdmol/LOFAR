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
		vFullname	VARCHAR(150);
		vNodename	VARCHAR(150);
		vQuery		VARCHAR(150);
		vLeaf		PIChierarchy.leaf%TYPE;
		vParamRefID	PIChierarchy.paramRefID%TYPE;
		i			INTEGER;

	BEGIN
	  -- Is topNode a Node or a Parameter?
	  SELECT leaf, paramRefID, name
	  INTO	 vLeaf, vParamRefID, vFullname
	  FROM	 PIChierarchy
	  WHERE	 nodeID = $2;

	  IF vLeaf = TRUE AND $3 = 0 THEN
		vQuery := \'=\' || chr(39) || vFullname || chr(39)
					|| \' AND h.leaf=true \';
	  ELSE
	    -- construct query
	    vQuery := \'similar to \' || chr(39) || vFullname;
	    FOR i in 1..$3 LOOP
		  vQuery := vQuery || \'.[^\\\\\\\\.]+\';
	    END LOOP;
	    vQuery := vQuery || chr(39);
	  END IF;

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
		FROM   PIChierarchy h
			   INNER JOIN PICparamRef r ON r.paramid = h.paramrefID
		WHERE  h.treeID = \' || $1 || \'
	    AND	   h.name \' || vQuery 
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;

--
-- getPICitemList (treeID, namefragment)
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
CREATE OR REPLACE FUNCTION getPICitemList(INT4, VARCHAR(150))
  RETURNS SETOF OTDBnode AS '
	DECLARE
		vRecord		RECORD;

	BEGIN
	  FOR vRecord IN 
	    SELECT h.nodeid,
			   h.parentid, 
			   h.paramrefid,
			   h.name, 
			   h.index, 
			   h.leaf,
			   1::int2,
			   \'\'::text,
			   r.description 
		FROM   PICparamref r
			   INNER JOIN PIChierarchy h ON r.paramid = h.paramrefID
		WHERE  h.treeID = $1
	    AND	   r.pvssname like $2 
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;



