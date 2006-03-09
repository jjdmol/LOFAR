--
--  SetTreeState.sql: function for changing the State of the tree
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
-- setTreeState (authToken, treeID, treeState)
--
-- Checks if the treeState is legal before assigning it.
--
-- Authorisation: yes
--
-- Tables:	otdbtree	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION setTreeState(INT4, INT4, INT2)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction				INT2 := 1;
		vTreeState				OTDBtree.state%TYPE;
		vTreeID					OTDBtree.treeID%TYPE;
		vMomID					OTDBtree.momID%TYPE;
		vTreeType				OTDBtree.treeType%TYPE;
		vClassif				OTDBtree.classif%TYPE;
		vIsAuth					BOOLEAN;
		vAuthToken				ALIAS FOR $1;
		vUserID					INT4;
		TThardware CONSTANT		INT2 := 10;
		TSactive   CONSTANT		INT2 := 600;
		TSobsolete CONSTANT		INT2 := 1200;

	BEGIN
		-- check authorisation(authToken, treeID, func, treeState)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, $3::int4) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized.\';
			RETURN FALSE;
		END IF;

		-- check classification
		SELECT	id
		INTO	vTreeState
		FROM	treeState
		WHERE	id = $3;
		IF NOT FOUND THEN
			RAISE EXCEPTION \'TreeState % does not exist\', $3;
			RETURN FALSE;
		END IF;

		-- get current treetype. 
		-- Note: tree existance is checked during auth.check
		SELECT 	momID, treetype, classif
		INTO   	vMomID, vTreeType, vClassif
		FROM	OTDBtree
		WHERE	treeID = $2;

        -- changing PIC tree to operational?
        -- restriction: only 1 PIC may be active of each classification
        IF vTreeType = TThardware AND $3 = TSactive THEN
            SELECT  treeid
            INTO    vTreeID     -- dummy
            FROM    OTDBtree
            WHERE   treetype = TThardware
            AND     classif  = vClassif
            AND     state    = TSactive
			AND		treeID  <> $2;
            IF FOUND THEN
                RAISE EXCEPTION \'Already an active hardware tree of the same classification.\';
                RETURN FALSE;
            END IF;
        END IF;

		-- Finally update tree
		UPDATE	OTDBtree
		SET		state = $3
		WHERE	treeid = $2;

		SELECT  whoIs($1)
		INTO 	vUserID;
		PERFORM addTreeState ($2, vMomID, $3, vUserID, \'\');

		-- (temp?) add extra timeinfo on PIC trees.
		IF vTreeType = TThardware THEN
		  BEGIN
		  IF $3 = TSactive THEN
			UPDATE OTDBtree
			SET    starttime = now()
			WHERE  treeid    = $2;
		  END IF;
		  IF $3 = TSobsolete THEN
		    UPDATE OTDBtree
			SET	   stoptime = now()
			WHERE  treeid   = $2;
		  END IF;
		  END;
		END IF;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

