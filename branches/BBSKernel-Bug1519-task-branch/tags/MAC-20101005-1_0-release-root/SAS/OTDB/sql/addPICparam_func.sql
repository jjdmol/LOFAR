--
--  addPICparam.sql: Add a parameter to an PIC tree.
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
-- Tables:	
--		PICparamref 	insert
--		PicHierarchy	insert
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION addPICparam (INT4, VARCHAR(150), INT2)
  RETURNS INT4 AS '
	DECLARE
		vParRefID	PICparamref.paramID%TYPE;
		vNodeID		PIChierarchy.nodeID%TYPE;
		vParentID	PIChierarchy.parentID%TYPE;
		vParType	param_type.id%TYPE;
		vFullname	VARCHAR(150);
		vNodename	VARCHAR(150);
		vBasename	VARCHAR(150);
		vParamIndex	INT2;
		vFieldnr	INT4;
		vLeaf		BOOLEAN;

	BEGIN
	  -- convert pvss-type to param-type
	  SELECT m.id
	  INTO   vParType
	  FROM	 param_type m, pvss_type s
	  WHERE	 s.id = $3 and s.name = m.name;
	  IF NOT FOUND THEN
		RAISE EXCEPTION \'Parametertype %d can not be converted\', $3;
	  END IF;

	  -- be sure NODE exists in reference table.
	  -- name has format like xxx:aaa.bbb.ccc.ddd or xxx:aaa.bbb.ccc.ddd_eee
	  vNodename := rtrim($2, \'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_\');			-- xxx:aaa_bbb_ccc.
	  vNodename := rtrim(vNodename, \'.\');				-- xxx:aaa.bbb.ccc
	  vFullname := $2;									-- xxx:aaa.bbb.ccc.ddd_eee
	  IF length(vNodename) > 0 THEN
	    SELECT paramID 
	    INTO   vParRefID
	    FROM   PICparamRef
	    WHERE  PVSSname = vNodename
	    LIMIT  1;
	    IF NOT FOUND THEN
		  -- node not yet in reference table, add it.
	      INSERT INTO PICparamRef(PVSSname, par_type)
	      VALUES (vNodename, 0);	-- type=node
	    END IF;
	  END IF;

	  -- be sure PARAMETER exists in reference table.
	  vParRefID := 0;
	  SELECT paramID 
	  INTO	 vParRefID
	  FROM   PICparamRef
	  WHERE  PVSSname = vFullname
	  LIMIT  1;
	  IF NOT FOUND THEN
		-- param not yet in reference table, add it.
		-- TODO: add other fields also.
	    INSERT INTO PICparamRef(PVSSname, par_type)
	    VALUES (vFullname, vParType);
		-- and retrieve its ID
--	    SELECT paramID 
--	    INTO   vParRefID
--	    FROM   PICparamRef
--	    WHERE  PVSSname = vFullname;
	  END IF;

	  -- add record to the PIC hierachical tree
	  vBasename := \'\';
	  vFieldnr  := 1;
	  vParentID := 0;
	  vNodeID   := 0;
	  vParamIndex := -1;		-- TODO
	  LOOP
		vNodename := split_part(vFullname, \'.\', vFieldnr);
		EXIT WHEN length(vNodename) <= 0;
		IF vFieldnr != 1 THEN
		  vBasename := vBasename || \'.\';
		END IF;
		vBasename := vBasename || vNodename;

		SELECT	nodeID, paramRefID
		INTO  	vNodeID, vParRefID
		FROM	PIChierarchy
		WHERE	treeID   = $1
		AND		parentID = vParentID
		AND		name     = vBasename;
		IF NOT FOUND THEN
		  vNodeID  := nextval(\'PIChierarchID\');
		  IF length(split_part(vFullname, \'.\', vFieldnr+1)) <= 0 THEN
			vLeaf := TRUE;
		  ELSE
			vLeaf := FALSE;
		  END IF;
		  -- get id of original parameter
	      SELECT paramID 
	      INTO   vParRefID
	      FROM   PICparamRef
	      WHERE  PVSSname = vBasename;
		  INSERT INTO PIChierarchy(treeID, nodeID, parentID, 
								   paramRefID, name, index, leaf)
		  VALUES ($1, vNodeID, vParentID, 
				  vParRefID, vBasename, vParamIndex, vLeaf);
		END IF;

		vFieldnr := vFieldnr + 1;
		vParentID:= vNodeID;
	  END LOOP;

	  RETURN vNodeID;
	END;
' LANGUAGE plpgsql;

