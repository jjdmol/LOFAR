--
--  get_ref_objects_func.sql : get the objects of one station
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
-- get_ref_objects(<stationname>|all, lba|hba|all)
--
CREATE OR REPLACE FUNCTION get_ref_objects(VARCHAR(10), VARCHAR(10))
  RETURNS SETOF genCoord AS $$
	DECLARE
		aStation	ALIAS FOR $1;
		aType		ALIAS FOR $2;
		vObjID		object.ID%TYPE;
		vQuery		TEXT;
		vRecord		RECORD;

	BEGIN
		-- construct query
		IF upper(aStation) = 'ALL' THEN
		  IF upper(aType) = 'ALL' THEN
			vQuery := '';
		  ELSE
			vQuery := 'WHERE o.type=' || chr(39) || aType || chr(39);
		  END IF;
		ELSE
		  IF upper(aType) = 'ALL' THEN
			vQuery := 'WHERE o.stationname=' || chr(39) || aStation || chr(39);
		  ELSE
			vQuery := 'WHERE o.type=' || chr(39) || aType || chr(39);
			vQuery := vQuery || ' AND o.stationname=' || chr(39) || aStation || chr(39);
		  END IF;
		END IF;

		FOR vRecord in EXECUTE '
		  SELECT o.stationname,
				 o.type,
				 o.number,
				 r.ref_frame,
				 r.x,
				 r.y,
				 r.z
		  FROM	 object o
				 INNER JOIN reference_coord r ON r.id = o.id
		  ' || vQuery
		LOOP
		  RETURN NEXT vRecord;
		END LOOP;
		RETURN;
	END;
$$ language plpgsql;
