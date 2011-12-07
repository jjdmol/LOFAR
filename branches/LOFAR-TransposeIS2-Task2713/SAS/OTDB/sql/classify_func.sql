--
--  classify.sql: function for classifying a tree.
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
-- classify (authToken, treeID, classification)
--
-- Checks is the classification is legal before assigning it.
--
-- Authorisation: yes
--
-- Tables:	otdbtree	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION classify(INT4, INT4, INT2)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction				INT2 := 1;
		vTreeID					OTDBtree.treeID%TYPE;
		vClassif				OTDBtree.classif%TYPE;
		vTreeType				OTDBtree.treetype%TYPE;
		vIsAuth					BOOLEAN;
		vAuthToken				ALIAS FOR $1;
		TThardware CONSTANT		INT2 := 10;
		TSactive   CONSTANT		INT2 := 600;

	BEGIN
		-- check authorisation(authToken, treeID, func, classification)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, $3::int4) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized.\';
			RETURN FALSE;
		END IF;

		-- check classification
		SELECT	id
		INTO	vClassif
		FROM	classification
		WHERE	id = $3;
		IF NOT FOUND THEN
			RAISE EXCEPTION \'Classification % does not exist\', $3;
			RETURN FALSE;
		END IF;

		-- get current treetype. 
		-- Note: tree existance is checked during auth.check
		SELECT 	treetype
		INTO   	vTreeType
		FROM	OTDBtree
		WHERE	treeID = $2;

		-- changing PIC tree to operational?
		-- restriction: only 1 PIC may be active of each classification
		IF vTreeType = TThardware THEN
			SELECT	treeid
			INTO	vTreeID		-- dummy
			FROM	OTDBtree
			WHERE	treetype = TThardware
			AND		classif  = $3
			AND		state    = TSactive;
			IF FOUND THEN
				RAISE EXCEPTION \'Already an active hardware tree with same classification.\';
				RETURN FALSE;
			END IF;
		END IF;

		-- Finally update tree
		UPDATE	OTDBtree
		SET		classif = $3
		WHERE	treeid = $2;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

