--
--  hPICsearchParamID.sql: search parameterID in hierarchical PICtree.
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
-- hPICsearchParamID (treeID, paramname): paramID
--
-- Tries to resolve a parametername like a.b.c.d to an entry in an
-- hierarchical PIC tree.
--
-- Authorisation: none
--
-- Tables:	PIChierarchy read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION hPICsearchParamID(INT4, VARCHAR(120))
  RETURNS INT4 AS '
	DECLARE
	  vDotPos		INT4;
	  vFieldnr		INT4;
	  vParamID		INT4;
	  vParentID		INT4;
	  vNodename		VARCHAR(40);
	  vKey			VARCHAR(120);

	BEGIN
	  vKey      := translate($2, \':_\', \'..\');		-- use uniform seperator
	  vFieldnr  := 1;
	  vParentID := 0;
	  vParamId  := 0;
	  LOOP
		vNodename := split_part(vKey, \'.\', vFieldnr);
		EXIT WHEN length(vNodename) <= 0;

		SELECT	paramID
		INTO  	vParamID
		FROM	PIChierarchy
		WHERE	treeID   = $1
		AND		parentID = vParentID
		AND		name     = vNodename;
		IF NOT FOUND THEN
		  RETURN 0;
		END IF;

		vFieldnr := vFieldnr + 1;
		vParentID:= vParamID;
	  END LOOP;
		
	  RETURN vParamID;
	END;
' LANGUAGE plpgsql;


