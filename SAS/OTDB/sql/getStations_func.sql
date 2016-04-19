--
--  getStations.sql: function for getting a list of station from the active PIC tree.
--
--  Copyright (C) 2016
--  ASTRON (Netherlands Foundation for Research in Astronomy)
--  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
--  $Id: getTreeList_func.sql 9997 2007-03-01 12:21:46Z overeem $
--

--
-- getStations ()
-- 
-- Get a list of stations from the active PIC tree.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	 	read
--			pichierarchy	read
--
-- Types:	None
--
CREATE OR REPLACE FUNCTION getStations()
  RETURNS SETOF TEXT AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vRecord		RECORD;
		vTreeType	OTDBtree.treetype%TYPE;
		vPicID		OTDBtree.treeid%TYPE;
        TTPic	    CONSTANT INT2 := 10;
        TSactive	CONSTANT INT2 := 600;

	BEGIN
	  -- do selection
	  SELECT treeid
      INTO   vPicID
      FROM   OTDBtree
      WHERE  treetype = TTPic
      AND    state = TSactive;
      IF NOT FOUND THEN
		RAISE EXCEPTION 'No active PIC tree found!';
      END IF;

	  FOR vRecord IN  
		SELECT name
		FROM   PIChierarchy 
		WHERE  treeid = vPicID
        AND    name similar to 'LOFAR.PIC.[A-Za-z]+.[A-Za-z0-9]+' 
        AND    leaf is False
	  LOOP
		RETURN NEXT vRecord.name;
	  END LOOP;
	  RETURN;
	END
$$ LANGUAGE plpgsql;
