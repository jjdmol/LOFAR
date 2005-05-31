--
--  addPICparam.sql: Add a paramater to an PIC tree.
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
-- addPICparam (treeID, PVSSname, parType)
--
-- Adds the given parameter to the given hierarchical tree. Assures that 
-- the parameter is also known in the reference table.
--
-- Authorisation: no
-- 
-- Tables:	PICparamref PicHierarchy
--
-- Types:	none
--
DROP FUNCTION addPICparam (INT4, VARCHAR(120), INT2);
CREATE OR REPLACE FUNCTION addPICparam (INT4, VARCHAR(120), INT2)
  RETURNS INT4 AS '
	DECLARE
		vdummy		PICparamref.paramID%TYPE;
		vParamID	PIChierarchy.paramID%TYPE;
		vParentID	PIChierarchy.parentID%TYPE;
		vFullname	VARCHAR(120);
		vNodename	VARCHAR(40);
		vParamIndex	INT2;
		vFieldnr	INT4;

	BEGIN
	  -- be sure parameter exists in reference table.
	  SELECT paramid 
	  INTO	 vdummy
	  FROM   PICparamRef
	  WHERE  PVSSname = $2
	  LIMIT  1;
	  IF NOT FOUND THEN
		-- param not yet in reference table, add it.
		-- TODO: add other fields also.
	    INSERT INTO PICparamRef(PVSSname, par_type)
	    VALUES ($2, $3);
	  END IF;

	  -- add record to the PIC hierachical tree
	  -- PVSSname has format like xxx:aaa_bbb_ccc.ddd
	  vFullname := translate($2, \':_\', \'..\');
	  vFieldnr  := 1;
	  vParentID := 0;
	  vParamId  := 0;
	  vParamIndex := 0;		-- TODO
	  LOOP
		vNodename := split_part(vFullname, \'.\', vFieldnr);
		EXIT WHEN length(vNodename) <= 0;

		SELECT	paramID
		INTO  	vParamID
		FROM	PIChierarchy
		WHERE	treeID   = $1
		AND		parentID = vParentID
		AND		name     = vNodename;
		IF NOT FOUND THEN
		  vParamID  := nextval(\'PIChierarchID\');
		  INSERT INTO PIChierarchy(treeID, paramID, parentID, name, index)
		  VALUES ($1, vParamID, vParentID, vNodename, vParamIndex);
		END IF;

		vFieldnr := vFieldnr + 1;
		vParentID:= vParamID;
	  END LOOP;

	  RETURN vParamID;
	END;
' LANGUAGE plpgsql;

