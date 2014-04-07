--
--  getMomID2treeID.sql: function for changing the Mom information of a tree
--
--  Copyright (C) 2012
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
--  $Id: setMomInfo_func.sql 18624 2011-07-27 15:34:51Z schoenmakers $
--

--
-- getMomID2treeID()
--
-- Authorisation: no
--
-- Tables:	otdbtree	read
--
-- Types:	none
--

CREATE TYPE treeIDmomID AS (treeID INTEGER, momID INTEGER);

CREATE OR REPLACE FUNCTION getMomID2treeID()
  RETURNS SETOF treeIDmomID AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vRecord		RECORD;

	BEGIN
		FOR vRecord IN SELECT treeID,MomID FROM OTDBtree WHERE momID != '0' ORDER BY momID
		LOOP
		  RETURN NEXT vRecord;
		END LOOP;
		RETURN;
	END
$$ LANGUAGE plpgsql;

