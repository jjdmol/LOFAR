--
--  addKVT.sql: Add a parametervalue to the KVT tables
--
--  Copyright (C) 2005
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
CREATE OR REPLACE FUNCTION addKVT (INT, VARCHAR(150), VARCHAR(150), VARCHAR(20))
  RETURNS BOOLEAN AS '
    --  $Id$
	DECLARE
		vParRefID	PICparamref.paramID%TYPE;
		vTreeType	OTDBtree.treetype%TYPE;
		vTime		timestamp := NULL;
		vLastValue	TEXT;
		TThierarchy	CONSTANT INT2 := 30;

	BEGIN
	  -- convert timestamp
	  IF LENGTH($4) > 0 THEN
		  vTime := to_timestamp($4, \'YYYY-Mon-DD HH24:MI:SS.US\');
	  END IF;
	  IF vTime IS NULL THEN
		vTime := now();
	  END IF;

	  -- Is it a PIC param?
	  vParRefID := 0;
	  SELECT paramid 
	  INTO	 vParRefID
	  FROM   PICparamRef
	  WHERE  PVSSname = $2
	  LIMIT  1;

	  IF FOUND THEN
		-- its a PIC parameter
		IF $3::integer <= 10 THEN	
		  -- plain ON/OFF only register if last one was a serious problem
		  vLastValue := 100;
		  SELECT value
		  INTO	 vLastValue
		  FROM   PICkvt
		  WHERE  paramID = vParRefID
		  ORDER BY time DESC
		  LIMIT 1;
	      IF vLastValue::integer <= 10 THEN
		  	RETURN FALSE;
		  END IF;
	    END IF;
	    INSERT INTO PICkvt(paramID, value, time)
		VALUES (vParRefID, $3, vTime);
		RETURN TRUE;
	  END IF;

	  -- it is probably a VIC parameter, just store on name.
	  SELECT	treetype
	  INTO		vTreeType
	  FROM		otdbtree
	  WHERE treeid=$1;
	  IF vTreeType = TThierarchy THEN
		  INSERT INTO VICkvt (treeid, paramName, value, time)
		  VALUES ($1, $2, $3, vTime);
	  -- ELSE its a PIC kvt without an entry in the PIC
	  END IF;

	  RETURN TRUE;
	END;
' LANGUAGE plpgsql;

