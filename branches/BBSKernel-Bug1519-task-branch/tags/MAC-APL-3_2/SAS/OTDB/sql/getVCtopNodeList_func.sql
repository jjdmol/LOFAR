--
--  getVCtopNodeList.sql: gets the topnode of the tree
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
-- getVCtopNodeList (namefragment)
--
-- Get a list with the top components
--
-- Authorisation: no
--
-- Tables:	VICnodedef	read
--
-- Types:	OTDBnodeDef
--
CREATE OR REPLACE FUNCTION getVCtopNodeList(VARCHAR(40))
  RETURNS SETOF OTDBnodeDef AS '
	DECLARE
		vRecord			RECORD;

	BEGIN
	  -- do selection
	  FOR vRecord IN 
		SELECT nodeID,
			   name,
			   version,
			   classif,
			   constraints,
			   description
		FROM   VICnodedef 
		WHERE  name LIKE $1
		EXCEPT
		  SELECT	n.*
		  FROM		VICnodeDef n, VICparamDef p
		  WHERE		substr(p.name,2) = n.name
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END;
' LANGUAGE plpgsql;

