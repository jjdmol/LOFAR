--
--  security_func.sql: Some security related functions
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

-- OTDBlogin (username, passwd)
-- Check is to given combination of username and password is valid
CREATE OR REPLACE FUNCTION OTDBlogin(VARCHAR(80), VARCHAR(80))
  RETURNS INT4 AS '
	DECLARE
		vUserNr		INT4;

	BEGIN
	  -- determine parameterID
	  SELECT userID
	  INTO	 vUserNr
	  FROM   OTDBuser
	  WHERE  username = $1
	  AND    password = $2;
	  IF NOT FOUND THEN
		RETURN 0;
	  END IF;

	  UPDATE OTDBuser
	  SET	 lastLogin = \'now\'
	  WHERE	 username = $1;

	  RETURN vUserNr;
	  -- TODO: return authToken instead.
	END;
' LANGUAGE plpgsql;


-- OTDBauthenticate (userID, task, value)
-- Check if the user is allowed to perform a task
CREATE OR REPLACE FUNCTION OTDBauthenticate(INTEGER, INTEGER, INTEGER)
  RETURNS INT4 AS '
	DECLARE

	BEGIN
	  -- determine parameterID
	  SELECT userID
	  FROM   OTDBaccess
	  WHERE  userID = $1
	  AND    task   = $2
	  AND    value  = $3;
	  IF NOT FOUND THEN
		RETURN 0;
	  END IF;

	  RETURN 1;
	END;
' LANGUAGE plpgsql;

