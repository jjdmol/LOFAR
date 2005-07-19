--
--  getVTnode.sql: function for getting one node from a VICtemplate tree
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
-- getVTnode (treeID, nodeID)
-- 
-- Get a list of items.
--
-- Authorisation: none
--
-- Tables: 	victemplate		read
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION getVTnode(INT4, INT4)
  RETURNS OTDBnode AS '
	DECLARE
		vRecord		RECORD;

	BEGIN
	    SELECT t.nodeid,
			   t.parentid, 
			   t.originid,
			   t.name, 
			   t.index, 
			   t.leaf,
			   t.instances,
			   t.limits,
			   \'\'::text 
		INTO   vRecord
		FROM   VICtemplate t
		WHERE  t.treeID = $1
		AND	   t.nodeID = $2;

		IF NOT FOUND THEN
			RAISE EXCEPTION \'node % not in tree %\', $1, $2;
		END IF;

		RETURN vRecord;
	END
' LANGUAGE plpgsql;

