--
--  exportMetadata.sql: makes an usenet format export of all Metadata info from a tree
--
--  Copyright (C) 2013
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
--  $Id: exportTree_func.sql 20032 2012-02-07 07:10:34Z overeem $
--

--
-- exportMetadata (treeID)
--
-- Makes a key-value list of all kvt values of a VIC tree
--
-- Authorisation: no
--
-- Tables:	VICkvt	read
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION exportMetadata(INT4)
  RETURNS TEXT AS $$
    --  $Id: addComponentToVT_func.sql 19935 2012-01-25 09:06:14Z mol $
	DECLARE
		vResult			TEXT;
		vRecord			RECORD;
		vTreeID			ALIAS FOR $1;

	BEGIN
	  vResult := '';
	  FOR vRecord IN
		SELECT treeid,paramname,value,time
		FROM   VICkvt
		WHERE  treeID = vTreeID
		ORDER BY paramname, time ASC
		LOOP
		  vResult := vResult || vRecord.paramname || '=' || vRecord.value || chr(10);
		END LOOP;
		RETURN vResult;
	END;
$$ LANGUAGE plpgsql;

