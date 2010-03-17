--
--  updateVTnode.sql: function updating a Node of a VIC template tree
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
-- updateVTnode (authToken, treeID, nodeID, instances, limits)
--
-- Saves the new values to the database
--
-- Authorisation: yes
--
-- Tables:	VICtemplate		update
--			VIChierarchy	update
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION updateVTnode(INT4, INT4, INT4, INT2, TEXT)
  RETURNS BOOLEAN AS '
	DECLARE
		TTtemplate  CONSTANT	INT2 := 20;
		TThierarchy CONSTANT	INT2 := 30;
		vFunction   CONSTANT	INT2 := 1;
		vTreeType		OTDBtree.treetype%TYPE;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;
		vLimits			TEXT;

	BEGIN
		-- check authorisation(authToken, tree, func, parameter)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
			RETURN FALSE;
		END IF;

		-- get treetype
		SELECT	treetype
		INTO	vTreeType
		FROM	OTDBtree
		WHERE	treeID = $2;
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Tree % does not exist\', $1;
		END IF;

		IF vTreeType = TTtemplate THEN
			-- get ParentID of node to duplicate
			vLimits := replace($5, \'\\\'\', \'\');
			UPDATE	VICtemplate
			SET		instances = $4,
					limits    = vLimits
			WHERE	treeID = $2
			AND 	nodeID = $3;
			IF NOT FOUND THEN
			  RAISE EXCEPTION \'Node % of template-tree could not be updated\', $3;
			  RETURN FALSE;
			END IF;
		ELSEIF vTreeType = TThierarchy THEN
			vLimits := replace($5, \'\\\'\', \'\');
			UPDATE	VIChierarchy
			SET		value = vLimits
			WHERE	treeID = $2
			AND 	nodeID = $3;
			IF NOT FOUND THEN
			  RAISE EXCEPTION \'Node % of VIC-tree could not be updated\', $3;
			  RETURN FALSE;
			END IF;
		ELSE
			RAISE EXCEPTION \'Nodes of PIC trees can not be updated\';
		END IF;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

