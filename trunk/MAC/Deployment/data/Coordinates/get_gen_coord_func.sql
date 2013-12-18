--
--  get_gen_coord_func.sql : get the generated coordinates
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
-- get_gen_coord(<stationname>|all, <date>)
--
CREATE OR REPLACE FUNCTION get_gen_coord(VARCHAR(10), FLOAT8)
  RETURNS SETOF genCoord AS $$
	DECLARE
		aStation	ALIAS FOR $1;
		aDate		ALIAS FOR $2;
		vObjID		object.ID%TYPE;
		vQuery		TEXT;
		vRecord		RECORD;

	BEGIN
		-- construct query
		IF upper(aStation) = 'ALL' THEN
		  	vQuery := 'WHERE g.target_date=' || aDate;
		ELSE
			vQuery := 'WHERE o.stationname=' || chr(39) || aStation || chr(39);
			vQuery := vQuery || ' AND g.target_date=' || aDate;
		END IF;

		FOR vRecord in EXECUTE '
		  SELECT o.stationname,
				 o.type,
				 o.number,
				 g.ref_frame,
				 g.x,
				 g.y,
				 g.z
		  FROM	 object o
				 INNER JOIN generated_coord g ON g.id = o.id
		  ' || vQuery
		LOOP
		  RETURN NEXT vRecord;
		END LOOP;
		RETURN;
	END;
$$ language plpgsql;
