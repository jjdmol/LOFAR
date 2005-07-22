--
--  addKVT.sql: Add a parametervalue to the KVT tables
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
-- Add the given KVT triple to the KVT tables.
--
-- Authorisation: no
-- 
-- Tables:	pickvt			insert
--			vickvt			insert
--			picparamref		read
--
-- Types:	none
--
-- TODO: IMPLEMENT VIC PART
--
CREATE OR REPLACE FUNCTION addKVT (VARCHAR(120), VARCHAR(120), VARCHAR(20))
  RETURNS BOOLEAN AS '
	DECLARE
		vParRefID	PICparamref.paramID%TYPE;
		vTime		timestamp := NULL;

	BEGIN
	  -- convert timestamp
	  IF LENGTH($3) > 0 THEN
		  vTime := to_timestamp($3, \'YYYY-Mon-DD HH24:MI:SS.US\');
	  END IF;
	  IF vTime IS NULL THEN
		vTime := now();
	  END IF;

	  -- Is it a PIC param?
	  vParRefID := 0;
	  SELECT paramid 
	  INTO	 vParRefID
	  FROM   PICparamRef
	  WHERE  PVSSname = $1
	  LIMIT  1;

	  IF FOUND THEN
		-- its a PIC parameter
	    INSERT INTO PICkvt(paramID, value, time)
		VALUES (vParRefID, $2, vTime);
		RETURN TRUE;
	  END IF;

	  -- its a VIC parameter
	  vParRefID := hVICsearchParamID($1);
	  IF vParRefID = 0 THEN
		-- not found? probably a shared param, store on name
		INSERT INTO VICkvt (paramName, nodeID, value, time)
		VALUES ($1, 0, $2, vTime);
	  ELSE
	    INSERT INTO VICkvt(nodeID, value, time)
	    VALUES (vParRefID, $2, vTime);
	  END IF;

	  RETURN TRUE;
	END;
' LANGUAGE plpgsql;

