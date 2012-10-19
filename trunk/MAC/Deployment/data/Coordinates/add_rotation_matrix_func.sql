--
--  add_rotation_matrix_func.sql : insert or replace a generated coordinate
--
--  Copyright (C) 2008
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
--  $Id: $
--

--
-- add_rotation_matrix_func()

--
CREATE OR REPLACE FUNCTION add_rotation_matrix(VARCHAR(10), VARCHAR(10), FLOAT8[3][3])
  RETURNS INT4 AS $$
	DECLARE
		aStation	ALIAS FOR $1;
		aType		ALIAS FOR $2;
		aMatrix		ALIAS FOR $3;
		vObjID		object.ID%TYPE;
		vDummy		VARCHAR(40);
		vDummyID	INT4;
		
	BEGIN
	-- check object
		SELECT	ID
		INTO	vObjID
		FROM	object
		WHERE	stationname = aStation AND type = aType
		LIMIT	1;

		IF NOT FOUND THEN
			RAISE EXCEPTION 'Object % of station % is unknown', aType, aStation;
			RETURN FALSE;
		END IF;
		
		-- All ok, insert or replace record
		-- check object
		SELECT	ID
		INTO	vDummyID
		FROM	rotation_matrices
		WHERE	ID = vObjID
		LIMIT	1;
		
		IF NOT FOUND THEN
			INSERT INTO rotation_matrices (ID, matrix)
			VALUES ( vObjID, aMatrix );
		ELSE
			UPDATE rotation_matrices
			SET		matrix = aMatrix
			WHERE  ID = vObjID;
		END IF;
		RETURN vObjID;
	END;
$$ language plpgsql;
