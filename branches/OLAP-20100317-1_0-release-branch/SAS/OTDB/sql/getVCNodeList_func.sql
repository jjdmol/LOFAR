--
--  getVCNodeList.sql: gets the topnode of the tree
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
-- getVCNodeList (namefragment, versionnr, toponly)
--
-- Get a list with the top components
--
-- Authorisation: no
--
-- Tables:	VICnodedef	read
--
-- Types:	OTDBnodeDef
--
CREATE OR REPLACE FUNCTION getVCNodeList(VARCHAR(150), INT4, BOOLEAN)
  RETURNS SETOF OTDBnodeDef AS '
	DECLARE
		vRecord			RECORD;
		vRestriction	VARCHAR(200);

	BEGIN
	  IF $2 <> 0 THEN
		vRestriction := \' AND version = \' || $2;
	  ELSE
		vRestriction := \'\';
	  END IF;

	  IF $3 = TRUE THEN
		vRestriction := \' EXCEPT SELECT n.* FROM VICnodeDEF n, VICparamDef p \' ||
						\' WHERE cleanNodeName(p.name) = n.name\';
	  END IF;

	  -- do selection
	  FOR vRecord IN EXECUTE \'
		SELECT nodeID,
			   name,
			   version,
			   classif,
			   constraints,
			   description
		FROM   VICnodedef 
		WHERE  name LIKE \' || chr(39) || $1 || chr(39) || vRestriction
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END;
' LANGUAGE plpgsql;

