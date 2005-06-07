--
--  addKVT.sql: Add a paramater to the KVT tables
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
-- addKVT (fullname, value, timestr)
--
-- Expected timeformat YYYY Mon DD HH24:MI:SS.MS
--
-- Adds the given parameter to the given hierarchical tree. 
--
-- Authorisation: no
-- 
-- Tables:	pickvt			insert
--			vickvt			insert
--			picparamref		read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION addKVT (VARCHAR(120), VARCHAR(120), VARCHAR(20))
  RETURNS BOOLEAN AS '
	DECLARE
		vParRefID	PICparamref.paramID%TYPE;

	BEGIN
	  -- Is it a PIC param?
	  vParRefID := 0;
	  SELECT paramid 
	  INTO	 vParRefID
	  FROM   PICparamRef
	  WHERE  PVSSname = $1
	  LIMIT  1;
	  IF NOT FOUND THEN
		-- its a VIC parameter
		-- TODO: implement VIC part
		RETURN FALSE;
	  ELSE
		-- its a PIC parameter
	    INSERT INTO PICkvt(paramID, value, time)
		VALUES (vParRefID, $2, to_timestamp($3, \'YYYY-Mon-DD HH24:MI:SS.US\'));
	  END IF;

	  RETURN TRUE;
	END;
' LANGUAGE plpgsql;

