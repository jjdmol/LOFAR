--
--  getVTitemList.sql: function for a list of nodes on a name fragment.
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
-- getVTitemList (treeID, name fragment)
-- 
-- Get a list of items based on a name fragment.
--
-- Authorisation: none
--
-- Tables: 	victemplate		read
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION getVTitemList(INT4, VARCHAR(150))
  RETURNS SETOF OTDBnode AS '
	DECLARE
		vRecord		RECORD;

	BEGIN
	  FOR vRecord IN
	    SELECT t.nodeid,
			   t.parentid, 
			   t.originid,
			   t.name, 
			   t.index, 
			   t.leaf,
			   t.instances,
			   t.limits,
			   \'\'::text
--			   n.description	-- TODO: join depends on t.leaf
		FROM   VICtemplate t
--			   INNER JOIN VICnodedef n on n.nodeID = t.originID
		WHERE  t.treeID = $1
		AND	   t.name LIKE $2
		ORDER BY t.leaf, t.name, t.index
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;

	  RETURN;
	END
' LANGUAGE plpgsql;

