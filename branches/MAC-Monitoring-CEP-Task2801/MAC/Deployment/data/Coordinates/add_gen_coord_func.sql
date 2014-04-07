--
--  add_gen_coord_func.sql : insert or replace a generated coordinate
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
-- add_gen_coord_func(from_frame, to_frame, target_date, Tx, Ty, Tz, sf, Rx, Ry, Rz, person1, person2, comment)

--
CREATE OR REPLACE FUNCTION add_gen_coord(VARCHAR(10), VARCHAR(10), FLOAT4, FLOAT8, FLOAT8, FLOAT8, FLOAT8, VARCHAR(10))
  RETURNS INT4 AS $$
	DECLARE
		aStation	ALIAS FOR $1;
		aType		ALIAS FOR $2;
		aNumber		ALIAS FOR $3;
		aX			ALIAS FOR $4;
		aY			ALIAS FOR $5;
		aZ			ALIAS FOR $6;
		aTargetDate	ALIAS FOR $7;
		aRefFrame   ALIAS FOR $8;
		vObjID		object.ID%TYPE;
		vDummy		VARCHAR(40);
		vDummyID	INT4;
		
	BEGIN
	-- check object
		SELECT	ID
		INTO	vObjID
		FROM	object
		WHERE	stationname = aStation AND type = aType AND number = aNumber
		LIMIT	1;

		IF NOT FOUND THEN
			RAISE EXCEPTION 'Object % % of station % is unknown', aType, aNumber, aStation;
			RETURN FALSE;
		END IF;
		
		-- All ok, insert or replace record
		-- check object
		SELECT	ID
		INTO	vDummyID
		FROM	generated_coord
		WHERE	ID = vObjID AND target_date = aTargetDate
		LIMIT	1;
		
		IF NOT FOUND THEN
			INSERT INTO generated_coord (ID, X, Y, Z, target_date, ref_frame)
			VALUES ( vObjID, aX, aY, aZ, aTargetDate, aRefFrame );
		ELSE
			UPDATE generated_coord
			SET		X = aX, Y = aY, Z = aZ, target_date = aTargetDate, ref_frame = aRefFrame
			WHERE  ID = vObjID AND target_date = aTargetDate;
		END IF;
		RETURN vObjID;
	END;
$$ language plpgsql;
