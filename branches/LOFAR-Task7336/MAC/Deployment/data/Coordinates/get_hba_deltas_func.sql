--
--  get_hba_deltas_func.sql : get normal vector of antenna type
--
--  Copyright (C) 2008
--  ASTRON (Netherlands Foundation for Research in Astronomy)
--  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
-- get_hba_deltas(<stationname>|all, <type>)
--
CREATE OR REPLACE FUNCTION get_hba_deltas(VARCHAR(10), VARCHAR(10))
  RETURNS SETOF genDeltas AS $$
	DECLARE
		aStation	ALIAS FOR $1;
		aType		ALIAS FOR $2;
		vObjID		object.ID%TYPE;
		vQuery		TEXT;
		vRecord		RECORD;

	BEGIN
		-- construct query
		IF upper(aStation) = 'ALL' THEN
		  	vQuery := 'WHERE o.type=' || aType;
		ELSE
			vQuery := 'WHERE o.stationname=' || chr(39) || aStation || chr(39);
			vQuery := vQuery || ' AND o.type=' || chr(39) || aType || chr(39);
		END IF;

		FOR vRecord in EXECUTE '
		  SELECT o.stationname,
				 o.type,
				 g.deltas
		  FROM	 object o
				 INNER JOIN generated_hba_deltas g ON g.id = o.id
		  ' || vQuery
		LOOP
		  RETURN NEXT vRecord;
		END LOOP;
		RETURN;
	END;
$$ language plpgsql;
