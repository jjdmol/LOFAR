--
--  saveVCnode.sql: function saving a node in the VC table
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
-- saveVCnode (authToken, nodeID, name, version, 
--				classif, constraints, description)
--
-- Saves the new values to the database
--
-- Authorisation: yes
--
-- Tables:	VICnodeDef	insert/update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION saveVCnode(INT4, INT4, VARCHAR(150), INT4, 
										INT2, TEXT, TEXT)
  RETURNS INT4 AS '
	DECLARE
		vNodeID			VICnodedef.nodeID%TYPE;
		vName			VICnodedef.name%TYPE;
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;
		vConstraints	TEXT;
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

		vName := rtrim(translate($3, \'.\', \' \'));	-- replace dot w space
		vConstraints := replace($6, \'\\\'\', \'\');	-- remove single quotes
		vDescription := replace($7, \'\\\'\', \'\');

		-- check if node exists
		SELECT	nodeID
		INTO	vNodeID
		FROM	VICnodedef
		WHERE	name = $3
		AND		version = $4
		AND		classif = $5;
		IF NOT FOUND THEN
		  vNodeID := nextval(\'VICnodedefID\');
		  -- create new node
		  INSERT INTO VICnodedef
		  VALUES	(vNodeID, vName, $4, $5, vConstraints, vDescription);
		ELSE
		  -- update node
		  UPDATE VICnodedef
		  SET	 constraints = vConstraints,
				 description = vDescription
		  WHERE	 nodeID = vNodeID;
		END IF;

		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Node % could not be saved\', $4;
		  RETURN 0;
		END IF;

		RETURN vNodeID;
	END;
' LANGUAGE plpgsql;

