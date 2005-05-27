--
--  misc_func.sql: miscelaneous functions.
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
-- isAuthorized (authToken, function, value)
--
-- checks if the fucntion may be executed with the given value.
--
-- Authorisation: n/a
--
-- Tables:	otdbuser	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION isAuthorized(INT4, INT2, INT4)
  RETURNS BOOLEAN AS '
	BEGIN
		RETURN TRUE;		-- for now everthing is allowed
	END;
' LANGUAGE plpgsql;


--
-- whoIs (authToken)
--
-- Returns the userID based on the Auth Token.
--
-- Authorisation: n/a
--
-- Tables:	otdbuser	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION whoIs(INT4)
  RETURNS INT4 AS '
	BEGIN
		RETURN 1;		-- for now return userid 1
	END;
' LANGUAGE plpgsql;


