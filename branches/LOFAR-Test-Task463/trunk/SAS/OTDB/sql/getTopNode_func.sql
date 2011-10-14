--
--  getTopNode.sql: gets the topnode of the tree
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
-- getTopNode (treeID)
--
-- gets the topnode of the tree
--
-- Authorisation: no
--
-- Tables:	otdbtree	read
--
-- Types:	OTDBnode
--
CREATE OR REPLACE FUNCTION getTopNode(INT4)
  RETURNS OTDBnode AS '
	DECLARE
		TThardware CONSTANT	INT2 := 10;
		TTtemplate CONSTANT	INT2 := 20;
		vNode			RECORD;
		vTreeType		OTDBtree.treetype%TYPE;

	BEGIN
		-- get treetype
		SELECT	treetype
		INTO	vTreeType
		FROM	OTDBtree
		WHERE	treeID = $1;
		IF NOT FOUND THEN
		  RAISE EXCEPTION \'Tree % does not exist\', $1;
		END IF;

		IF vTreeType = TThardware THEN
		  SELECT h.nodeID,
				 h.parentID,
				 h.paramrefID,
				 h.name,
				 h.index,
				 h.leaf,
				 1::int2,
				 \'1\'::text,		-- limits
				 r.description
		  INTO	 vNode
		  FROM	 PIChierarchy h
				 INNER JOIN PICparamref r on r.paramid = h.paramrefID
		  WHERE	 h.treeID = $1
		  AND	 h.parentID = 0;

		ELSIF vTreeType = TTtemplate THEN
		  SELECT t.nodeID,
				 t.parentID,
				 t.originID,
				 t.name,
				 t.index,
				 t.leaf,
				 t.instances,
				 t.limits,
				 n.description
		  INTO	 vNode
		  FROM	 VICtemplate t
				 INNER JOIN VICnodedef n on n.nodeID = t.originID
		  WHERE	 t.treeID = $1
		  AND	 t.parentID = 0;

		ELSE
		  SELECT h.nodeID,
				 h.parentID,
				 h.paramRefID,
				 h.name,
				 h.index,
				 h.leaf,
				 1::int2,
				 \'1\'::text,		--	limits,
				 n.description
		  INTO	 vNode
		  FROM	 VIChierarchy h
				 INNER JOIN VICnodedef n on n.nodeID = h.paramRefID
		  WHERE	 h.treeID = $1
		  AND	 h.parentID = 0;

		END IF;

		RETURN vNode;
	END;
' LANGUAGE plpgsql;

