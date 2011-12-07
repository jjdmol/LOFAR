--
--  add_object_func.sql : inserts new object if not already inserted.
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
-- object table
--
CREATE OR REPLACE FUNCTION add_object(VARCHAR(10), VARCHAR(10), INT4)
  RETURNS VOID AS $$
	DECLARE
		aStation	ALIAS FOR $1;
		aType		ALIAS FOR $2;
		aNumber		ALIAS FOR $3;
		vObjID		object.ID%TYPE;

	BEGIN
		SELECT	ID
		INTO	vObjID
		FROM	object
		WHERE	stationname = $1 AND type = $2 AND number = $3
		LIMIT	1;

		IF NOT FOUND THEN
			INSERT INTO object(stationname, type, number)
			VALUES		($1, $2, $3);
		END IF;
	END;
$$ language plpgsql;
