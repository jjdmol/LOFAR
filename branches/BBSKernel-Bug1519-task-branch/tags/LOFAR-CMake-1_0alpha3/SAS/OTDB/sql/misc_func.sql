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
		vState			OTDBtree.state%TYPE;
		vCallerID		INT2 := 0;

	BEGIN
		-- get treetype and owner for authorisation
		IF $2 != 0 THEN
		  SELECT treetype, state
		  INTO	 vTreeType, vState
		  FROM	 OTDBtree
		  WHERE	 treeID = $2;
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


--
-- VersionNrString (versionnr)
--
-- Returns a string being the formatted versionnr
-- 
-- Authorisation: n/a
--
-- Tables:	none
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION VersionNrString(INT4)
  RETURNS VARCHAR(20) AS '
	BEGIN
		RETURN $1/10000 || \'.\' || ($1/100)%100 || \'.\' || $1%100;
	END;
' LANGUAGE plpgsql IMMUTABLE;

--
-- VersionNrValue(versionnr_string)
--
-- Returns the integer value of the versionnumberstring
-- 
-- Authorisation: n/a
--
-- Tables:	none
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION VersionNrValue(VARCHAR(50))
  RETURNS INT4 AS '
	DECLARE
		vRelease		INT4;
		vUpdate			INT4;
		vPatch			INT4;
				
	BEGIN
		vRelease := substring($1 from \'([0-9]+)\.[0-9]+\.[0-9]+\');
		vUpdate  := substring($1 from \'[0-9]+\.([0-9]+)\.[0-9]+\');
		vPatch   := substring($1 from \'[0-9]+\.[0-9]+\.([0-9]+)\');
		
		RETURN vRelease * 10000 + (vUpdate%100)*100 + vPatch%100;
	END;
' LANGUAGE plpgsql IMMUTABLE;

--
-- getVersionNr (nodename)
--
-- Returns a string being the formatted versionnr
-- 
-- Authorisation: n/a
--
-- Tables:	none
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION getVersionNr(VARCHAR(150))
  RETURNS INT4 AS '
	BEGIN
		RETURN VersionNrValue(substring($1 from \'%{#"%#"}\' for \'#\'));
	END;
' LANGUAGE plpgsql IMMUTABLE;

--
-- childNodeName (name, versionnr)
--
-- Returns the child node name:
-- # is added of at begin and versionnr is appended
-- 
-- Authorisation: n/a
--
-- Tables:	none
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION childNodeName(VARCHAR(150), INT4)
  RETURNS VARCHAR(150) AS '
	BEGIN
		RETURN \'#\' || $1 || \'{\' || VersionNrString($2) || \'}\';
	END;
' LANGUAGE plpgsql IMMUTABLE;

--
-- cleanNodeName (name)
--
-- Returns the basic node name:
-- # is stripped of at begin and trailing {xx} is removed.
-- 
-- Authorisation: n/a
--
-- Tables:	none
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION cleanNodeName(VARCHAR(150))
  RETURNS TEXT AS '
	BEGIN
		IF substr($1, length($1)) = \'}\' THEN
			RETURN ltrim(substr($1, 1, strpos($1,\'{\')-1), \'#\');
		ELSE
			RETURN ltrim($1,\'#\');
		END IF;
	END;
' LANGUAGE plpgsql IMMUTABLE;


--
-- isReference (name)
--
-- Authorisation: n/a
--
-- Tables:	none
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION isReference(TEXT)
  RETURNS BOOLEAN AS '
	BEGIN
		RETURN substr($1, 1, 2) = \'>>\';
	END;
' LANGUAGE plpgsql IMMUTABLE;

--
-- calcArraySize (arraystring)
--
-- Authorisation: n/a
--
-- Tables:	none
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION calcArraySize(TEXT)
  RETURNS TEXT AS '
	DECLARE
		vSize		INTEGER;
		vArray		TEXT;

	BEGIN
		vArray := ltrim($1,\'[ \');
		vArray := rtrim(vArray,\'] \');
		vSize := 0;
		WHILE length(vArray) > 0 LOOP
			-- remove element
			vArray := ltrim(vArray, \'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_ "[]{}+%<>.\');
			vSize := vSize + 1;					-- count element
			vArray := ltrim(vArray, \',\');		-- strip comma
		END LOOP;
		RETURN vSize;
	END;
' LANGUAGE plpgsql IMMUTABLE;
