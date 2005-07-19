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
-- isAuthorized (authToken, treeID, function, value)
--
-- checks if the function may be executed with the given value.
--
-- Authorisation: n/a
--
-- Tables:	otdbuser	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION isAuthorized(INT4, INT4, INT2, INT4)
  RETURNS BOOLEAN AS '
	DECLARE
		vTreeType		OTDBtree.treetype%TYPE;
		vCallerID		INT2 := 0;

	BEGIN
		-- get treetype and owner for authorisation
		IF $2 != 0 THEN
		  SELECT	treetype
		  INTO	vTreeType
		  FROM	OTDBtree
		  WHERE	treeID = $2;
		  IF NOT FOUND THEN
		    RAISE EXCEPTION \'Tree % does not exist!\', $2;
		    RETURN FALSE;
		  END IF;
		END IF;

		-- Resolve creator and check it.
		SELECT whoIs($1)
		INTO   vCallerID;
		IF NOT FOUND OR vCallerID = 0 THEN
			RAISE EXCEPTION \'Illegal authorisation token\';
			RETURN FALSE;
		END IF;

		-- TODO: search auth tables 
		-- SELECT .. vCallerID, vTreeType, $3, $4

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


