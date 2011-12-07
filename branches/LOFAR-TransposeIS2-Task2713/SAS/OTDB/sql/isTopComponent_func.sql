--
--  isTopComponent.sql: gets the topnode of the tree
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
-- isTopComponent (componentID): bool
--
-- returns if a component is a topcomponent.
--
-- Authorisation: no
--
-- Tables:	VICnodedef	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION isTopComponent(INT)
  RETURNS BOOLEAN AS '
	DECLARE
		vNodeID		VICnodeDEF.nodeID%TYPE;

	BEGIN
	  -- do selection
	  SELECT nodeID
	  INTO	 vNodeID
	  FROM   VICnodedef 
	  WHERE  nodeID = $1
	  EXCEPT SELECT n.nodeID FROM VICnodeDEF n, VICparamDef p
			 WHERE cleanNodeName(p.name) = n.name;
	  IF FOUND THEN
		RETURN TRUE;
	  END IF;

	  RETURN FALSE;
	END;
' LANGUAGE plpgsql;

