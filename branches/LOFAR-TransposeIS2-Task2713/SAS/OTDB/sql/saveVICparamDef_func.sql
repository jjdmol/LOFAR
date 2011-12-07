--
--  saveVICparamDef.sql: function saving a node in the VC table
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
-- saveVICparamDef (authToken, nodeID, name, type, unit, pruning, 
--					valMoment, rtMod, limits, description)
--
-- Saves the new values to the database
--
-- Authorisation: yes
--
-- Tables:	VICparamDef	insert/update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION saveVICparamDef(INT4, INT4, VARCHAR(150), INT2, 
							INT2, INT2, INT2, BOOLEAN, TEXT, TEXT)
  RETURNS INT4 AS '
	DECLARE
		vParamID		VICparamDef.paramID%TYPE;
		vName			VICparamDef.name%TYPE;
		vVersionNr		VICNodeDef.version%TYPE;
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;
		vLimits			TEXT;
		vDescription	TEXT;

	BEGIN
		-- check authorisation(authToken, tree, func, parameter)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, 0, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
			RETURN FALSE;
		END IF;

		-- assure clean input.
		if substr($3,1,1) = \'#\' THEN
		  vVersionNr := getVersionNr($3);
		  vName		 := childNodeName(translate(cleanNodeName($3), \'.\', \' \'), vVersionNr);
		ELSE
		  vName        := rtrim(translate($3, \'.\', \' \'));	
		END IF;
	    vLimits      := replace($9, \'\\\'\', \'\');
		vDescription := replace($10, \'\\\'\', \'\');

		-- check if node exists
		SELECT	paramID
		INTO	vParamID
		FROM	VICparamDef
		WHERE	name = vName
		AND		nodeID = $2;
		IF NOT FOUND THEN
		  vParamID     := nextval(\'VICparamdefID\');
		  -- create new param
		  INSERT INTO VICparamdef
		  VALUES	(vParamID, $2, vName, $4, $5, $6, $7, $8, vLimits, vDescription);
		ELSE
		  -- update param
		  UPDATE VICparamdef
		  SET	 par_type = $4,
				 unit = $5, 
				 pruning = $6, 
				 validmoment = $7, 
				 RTmod = $8, 
				 limits = vLimits,
				 description = vDescription
		  WHERE	 paramID = vParamID;
		END IF;

		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Parameter % could not be saved\', $3;
		  RETURN 0;
		END IF;

		RETURN vParamID;
	END;
' LANGUAGE plpgsql;

