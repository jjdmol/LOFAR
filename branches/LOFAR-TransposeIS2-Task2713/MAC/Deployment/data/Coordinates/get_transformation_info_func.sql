--
--  get_transformation_info_func.sql : get normal vector of antenna type
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
-- get_transformation_info(<stationname>|all, <type>)
--
CREATE OR REPLACE FUNCTION get_transformation_info(VARCHAR(10))
  RETURNS SETOF transInfo AS $$
	DECLARE
		aFrame	    ALIAS FOR $1;
		vObjID		object.ID%TYPE;
		vQuery		TEXT;
		vRecord		RECORD;

	BEGIN
		FOR vRecord in EXECUTE '
		  SELECT name,
				 Tx,
				 Ty,
				 Tz,
				 sf,
				 Rx,
				 Ry,
				 Rz
		  FROM reference_frame
		  WHERE name = ' || chr(39) || aFrame || chr(39)
		  
		LOOP
		  RETURN NEXT vRecord;
		END LOOP;
		RETURN;
	END;
$$ language plpgsql;
