--
--  add_ref_coord_func.sql : insert or replace a reference coordinate
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
-- add_ref_coord(stationname, objecttype, objectnr, x, y, z, sx, sy, sz, refsys, refframe, method, date, pers1, pers2, pers3, absref, derived, comment)

--
CREATE OR REPLACE FUNCTION add_ref_coord(VARCHAR(10), VARCHAR(10), INT4, FLOAT8, FLOAT8, FLOAT8, FLOAT8, FLOAT8, FLOAT8, VARCHAR(10), VARCHAR(10), VARCHAR(10), TIMESTAMP, VARCHAR(30), VARCHAR(30), VARCHAR(30), VARCHAR(20), TEXT, TEXT)
  RETURNS BOOLEAN AS $$
	DECLARE
		aStation	ALIAS FOR $1;
		aType		ALIAS FOR $2;
		aNumber		ALIAS FOR $3;
		aX			ALIAS FOR $4;
		aY			ALIAS FOR $5;
		aZ			ALIAS FOR $6;
		aSX			ALIAS FOR $7;
		aSY			ALIAS FOR $8;
		aSZ			ALIAS FOR $9;
		aRefSys		ALIAS FOR $10;
		aRefFrame	ALIAS FOR $11;
		aMethod		ALIAS FOR $12;
		aDate		ALIAS FOR $13;
		aPerson1	ALIAS FOR $14;
		aPerson2	ALIAS FOR $15;
		aPerson3	ALIAS FOR $16;
		aAbsRef		ALIAS FOR $17;
		aDerived	ALIAS FOR $18;
		aComment	ALIAS FOR $19;
		vObjID		object.ID%TYPE;
		vDummy		VARCHAR(30);
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

		-- check reference system
		SELECT	name
	 	INTO	vDummy
		FROM	reference_system
		WHERE	name = aRefSys
		LIMIT	1;

		IF NOT FOUND THEN
			RAISE EXCEPTION 'ReferenceSystem % is unknown', aRefSys;
			RETURN FALSE;
		END IF;

		-- check reference frame
		SELECT	name
	 	INTO	vDummy
		FROM	reference_frame
		WHERE	name = aRefFrame
		LIMIT	1;

		IF NOT FOUND THEN
			RAISE EXCEPTION 'ReferenceFrame % is unknown', aRefFrame;
			RETURN FALSE;
		END IF;

		-- check method
		SELECT	name
	 	INTO	vDummy
		FROM	measurement_method
		WHERE	name = aMethod
		LIMIT	1;

		IF NOT FOUND THEN
			RAISE EXCEPTION 'Measurementmethod % is unknown', aMethod;
			RETURN FALSE;
		END IF;

		-- check Person1
		SELECT	name
	 	INTO	vDummy
		FROM	personnel
		WHERE	name = aPerson1
		LIMIT	1;

		IF NOT FOUND THEN
			RAISE EXCEPTION 'Person % is unknown', aPerson1;
			RETURN FALSE;
		END IF;

		-- check Person2
		IF LENGTH(aPerson2) > 0 THEN
			SELECT	name
			INTO	vDummy
			FROM	personnel
			WHERE	name = aPerson2
			LIMIT	1;

			IF NOT FOUND THEN
				RAISE EXCEPTION 'Person % is unknown', aPerson2;
				RETURN FALSE;
			END IF;
		END IF;

		-- check Person3
		IF LENGTH(aPerson3) > 0 THEN
			SELECT	name
			INTO	vDummy
			FROM	personnel
			WHERE	name = aPerson3
			LIMIT	1;

			IF NOT FOUND THEN
				RAISE EXCEPTION 'Person % is unknown', aPerson3;
				RETURN FALSE;
			END IF;
		END IF;

		-- All references are ok, insert or replace record
		SELECT	ID
		INTO	vDummyID
		FROM	reference_coord
		WHERE	ID = vObjID AND measure_date = aDate
		LIMIT	1;

		IF NOT FOUND THEN
			INSERT INTO reference_coord
			VALUES		( vObjID, aX, aY, aZ, aSX, aSY, aSZ, aRefSys, aRefFrame, aMethod, aDate, aAbsRef, aDerived, aPerson1, aPerson2, aPerson3, aComment );
		ELSE
			UPDATE reference_coord
			SET		X = aX, Y = aY, Z = aZ, sigma_X = aSX, sigma_Y = aSY, sigma_Z = aSZ, ref_system = aRefSys, ref_frame = aRefFrame, method = aMethod, abs_reference = aAbsRef, derived_from = aDerived, person1 = aPerson1, person2 = aPerson2, person3 = aPerson3, comment = aComment
			WHERE  ID = vObjID AND measure_date = aDate;
		END IF;
		RETURN TRUE;
	END;
$$ language plpgsql;
