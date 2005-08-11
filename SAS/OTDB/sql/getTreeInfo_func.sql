--
--  getTreeInfo.sql: function for getting treeinfo from the OTDB
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
-- getTreeInfo (treeID)
-- 
-- Get info of one tree
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--			campaign	read
--
-- Types:	treeInfo
--
CREATE OR REPLACE FUNCTION getTreeInfo(INT4)
  RETURNS treeInfo AS '
	DECLARE
		vRecord		RECORD;

	BEGIN
		SELECT	t.treeID, 
				t.classif, 
				u.username, 
				t.d_creation, 
				t.treetype, 
				t.state, 
				t.originID, 
				c.name, 
				t.starttime, 
				t.stoptime
		INTO	vRecord
		FROM	OTDBtree t 
				INNER JOIN OTDBuser u ON t.creator = u.userid
				INNER JOIN campaign c ON c.ID = t.campaign
		WHERE	t.treeID = $1;
		
		IF NOT FOUND THEN
			RAISE EXCEPTION \'Tree % does not exist\', $1;
		END IF;

	  	RETURN vRecord;
	END
' LANGUAGE plpgsql;



