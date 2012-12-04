--
--  getVICParamDef.sql: function for getting parameterDefinition from the OTDB
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
-- getParamDef (paramID)
-- 
-- Get info of one parmeter
--
-- Authorisation: none
--
-- Tables: 	VICparamdef	read
--
-- Types:	OTDBparamDef
--
CREATE OR REPLACE FUNCTION getVICParamDef(INT4)
  RETURNS OTDBparamDef AS '
	DECLARE
		vRecord		RECORD;

	BEGIN
		SELECT	paramid, 
				nodeid, 
				name, 
				par_type, 
				unit, 
				pruning, 
				validmoment, 
				rtmod, 
				limits,
				description
		INTO	vRecord
		FROM	VICparamdef 
		WHERE	paramID = $1;
		
		IF NOT FOUND THEN
			RAISE EXCEPTION \'Parameter % does not exist\', $1;
		END IF;

	  	RETURN vRecord;
	END
' LANGUAGE plpgsql;


