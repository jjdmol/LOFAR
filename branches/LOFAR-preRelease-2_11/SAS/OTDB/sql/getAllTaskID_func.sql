--
--  getAllTaskID.sql: function for getting treeinfo from the OTDB
--
--  Copyright (C) 2010
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
--  $Id:$

CREATE OR REPLACE FUNCTION getAllTaskID()
  RETURNS SETOF INTEGER AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vRecord		RECORD;

	BEGIN
	  FOR vRecord IN 
	    SELECT CAST(value AS INTEGER) 
	    FROM VICHIERARCHY 
	    WHERE NAME = 'LOFAR.ObsSW.Observation.Scheduler.taskID' 
	    ORDER BY VALUE 
	    LOOP
	  	  RETURN NEXT vRecord.value;
	    END LOOP;
		RETURN;
    END;
$$ language plpgsql;
