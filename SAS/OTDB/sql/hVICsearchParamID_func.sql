--
--  hVICsearchParamID.sql: search parameterID in hierarchical VICtree.
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
-- hVICsearchParamID (paramname): paramID
--
-- Tries to resolve a parametername like a.b.c.d to an entry in an
-- hierarchical VIC tree.
--
-- Authorisation: none
--
-- Tables:	VIChierarchy read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION hVICsearchParamID(VARCHAR(150))
  RETURNS INT4 AS '
	DECLARE
	  vDotPos		INT4;
	  vFieldnr		INT4;
	  vNodeID		INT4;
	  vParentID		INT4;
	  vNodename		VARCHAR(150);
	  vKey			VARCHAR(150);

	BEGIN
	  vKey      := $1;
	  vFieldnr  := 1;
	  vParentID := 0;
	  vNodeID   := 0;
	  LOOP
		vNodename := split_part(vKey, \'.\', vFieldnr);
		EXIT WHEN length(vNodename) <= 0;

		-- TODO: this will not work for shared VIs (does not start at top)
		SELECT	nodeID
		INTO  	vNodeID
		FROM	VIChierarchy
		WHERE	parentID = vParentID
		AND		name     = vNodename;
		IF NOT FOUND THEN
		  RETURN 0;
		END IF;

		vFieldnr := vFieldnr + 1;
		vParentID:= vNodeID;
	  END LOOP;
		
	  RETURN vNodeID;
	END;
' LANGUAGE plpgsql;


