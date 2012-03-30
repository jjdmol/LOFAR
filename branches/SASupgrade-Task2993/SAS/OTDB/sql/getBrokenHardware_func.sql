--
--  getBrokenHardware.sql: function for getting hardware not suitable for operational use.
--
--  Copyright (C) 2011
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
--  $Id: getTreeList_func.sql 18624 2011-07-27 15:34:51Z schoenmakers $
--

--
-- getBrokenHardware ([timestamp])
-- 
-- Get a list KVTs from the pic of hardware that was 'broken' on the given date.
--
-- Authorisation: none
--
-- Tables:  pickvt    read
--
-- Types:   OTDBvalue
--

CREATE OR REPLACE VIEW pktemp AS SELECT DISTINCT ON (paramid) paramid, value, time  FROM pickvt WHERE value > '10' ORDER BY paramid,time DESC;

CREATE OR REPLACE FUNCTION nextPICkvt(int,timestamp) 
	RETURNS SETOF OTDBvalue AS $$ 
	SELECT   paramid,''::VARCHAR(150),value,time 
	FROM     pickvt 
	WHERE    paramid=$1 AND time>$2 
	ORDER BY TIME 
	LIMIT    1; 
$$ LANGUAGE SQL;

CREATE OR REPLACE FUNCTION getBrokenHardware(TIMESTAMP) 
	RETURNS SETOF OTDBvalue AS $$
	DECLARE
		vRecord		RECORD;
		vRecord2	RECORD;
		vStartTime	TIMESTAMP;

	BEGIN
		IF $1 IS NULL THEN
			vStartTime = now();
		ELSE
			vStartTime = $1;
		END IF;
		FOR vRecord IN 
			SELECT p.paramid,r.pvssname,p.value,p.time 
			FROM pktemp p
			LEFT JOIN PICparamref r ON r.paramid = p.paramid
			WHERE time < vStartTime
		LOOP
			FOR vRecord2 IN 
				SELECT p.paramid,r.pvssname,p.value,p.time 
				FROM nextPICkvt(vRecord.paramid,vRecord.time) p
				LEFT JOIN PICparamref r ON r.paramid = p.paramid
			LOOP
				RETURN NEXT vRecord2;
			END LOOP;
			RETURN NEXT vRecord;
		END LOOP;
		RETURN;
	END
$$ LANGUAGE plpgsql;

