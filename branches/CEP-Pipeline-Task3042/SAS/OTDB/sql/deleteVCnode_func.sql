--
--  deleteVCnode.sql: function for deleting a (subtree) of VICtemplate nodes.
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
-- removeVCNode (authToken, nodeID)
--
-- Removes a node record including its parameters 
--
-- Authorisation: yes
--
-- Tables:	VICnodedef		delete
--			VICparamdef		delete
--			victemplate		search
--			vichierarchy	search
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION removeVCnode(INT4, INT4)
  RETURNS BOOLEAN AS '
	DECLARE
		vFunction		INT2 := 1;
		vIsAuth			BOOLEAN;
		vCount			INT4;
		vAuthToken		ALIAS FOR $1;

	BEGIN
		-- check authorisation(authToken, treeID, func, -)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, 0, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION \'Not authorized\';
			RETURN FALSE;
		END IF;

		-- check usage of nodes
		SELECT	count(t.nodeid) from victemplate t, vicnodedef n
		INTO	vCount
		WHERE	n.nodeID=$2 AND t.originID = n.nodeID AND t.leaf = false;
		IF vCount <> 0 THEN
			RAISE EXCEPTION \'There are template trees that use this component\';
			RETURN FALSE;
		END IF;

		SELECT	count(h.paramrefid) from vichierarchy h, vicnodedef n
		INTO	vCount
		WHERE	n.nodeID=$2 AND h.paramRefID = n.nodeID AND h.leaf = false;
		IF vCount <> 0 THEN
			RAISE EXCEPTION \'There are runtime trees that use this component\';
			RETURN FALSE;
		END IF;

		-- check usage of params 
		SELECT	count(t.nodeid) from victemplate t, vicparamdef p
		INTO	vCount
		WHERE	p.nodeid=$2 and t.originID = p.paramID AND t.leaf = true;
		IF vCount <> 0 THEN
			RAISE EXCEPTION \'There are template trees that use its parameters\';
			RETURN FALSE;
		END IF;

		SELECT	count(h.paramrefid) from vichierarchy h, vicparamdef p
		INTO	vCount
		WHERE	p.nodeid=$2 and h.paramRefID = p.paramID AND h.leaf = true;
		IF vCount <> 0 THEN
			RAISE EXCEPTION \'There are runtime trees that use its parameters \';
			RETURN FALSE;
		END IF;

		-- finally remove parameters and node
		DELETE FROM vicparamdef
		WHERE	nodeID=$2;

		DELETE FROM	vicnodedef
		WHERE	nodeID=$2;

		RETURN TRUE;
	END;
' LANGUAGE plpgsql;

