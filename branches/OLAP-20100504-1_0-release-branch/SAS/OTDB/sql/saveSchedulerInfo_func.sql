--
--  saveSchedulerInfo.sql: function updating a Node of a VIC template tree
--
--  Copyright (C) 2010
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
--  $Id: updateVTnode_func.sql 9104 2006-08-16 11:43:02Z overeem $
--

--
-- saveSchedulerInfo (authToken, treeID, nodeID, instances, limits)
--
-- Saves the new values to the database
--
-- Authorisation: yes
--
-- Tables:	OTDBtree		read
--			VIChierarchy	update
--
-- Types:	none
--
CREATE OR REPLACE FUNCTION saveSchedulerInfo(INT4, INT4, 
					TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT,
		TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, 
		TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, 
		TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT, TEXT)
  RETURNS BOOLEAN AS $$ 
	DECLARE
		TThierarchy CONSTANT	INT2 := 30;
		vFunction   CONSTANT	INT2 := 1;
		vTreeType		OTDBtree.treetype%TYPE;
		vIsAuth			BOOLEAN;
		vAuthToken		ALIAS FOR $1;
		vLimits			TEXT;
		vRecord			RECORD;
		vCounter		INT4;
		vQuery			TEXT;
	fieldList CURSOR FOR
      SELECT nodeId,limits FROM getvhitemlist($2, '%Observation.Scheduler.%') ORDER BY name ASC;

	BEGIN
		-- check authorisation(authToken, tree, func, parameter)
		vIsAuth := FALSE;
		SELECT isAuthorized(vAuthToken, $2, vFunction, 0) 
		INTO   vIsAuth;
		IF NOT vIsAuth THEN
			RAISE EXCEPTION 'Not authorized';
			RETURN FALSE;
		END IF;

		-- get treetype
		SELECT	treetype
		INTO	vTreeType
		FROM	OTDBtree
		WHERE	treeID = $2;
		IF NOT FOUND THEN
		  RAISE EXCEPTION 'Tree % does not exist', $1;
		END IF;

		IF NOT vTreeType = TThierarchy THEN
		  RAISE EXCEPTION 'Tree % is not a VIC tree', $2;
		END IF;

		-- Authorisation is checked, tree is checked, do the work.

        OPEN fieldList;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$3 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$4 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$5 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$6 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$7 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$8 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$9 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$10 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$11 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$12 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$13 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$14 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$15 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$16 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$17 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$18 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$19 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$20 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$21 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$22 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$23 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$24 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$25 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$26 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$27 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$28 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$29 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$30 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$31 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$32 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$33 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$34 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$35 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$36 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$37 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
                FETCH fieldList INTO vRecord;
                UPDATE VIChierarchy SET value=$38 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
		FETCH fieldList INTO vRecord;
		UPDATE VIChierarchy SET value=$39 WHERE treeID=$2 AND nodeID=vRecord.nodeID;
	    RETURN TRUE;
	END;
$$ LANGUAGE plpgsql;

