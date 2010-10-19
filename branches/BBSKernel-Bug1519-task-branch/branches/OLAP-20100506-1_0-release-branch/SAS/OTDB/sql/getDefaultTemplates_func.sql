--
--  getDefaultTemplate.sql: function for getting treeinfo from the OTDB
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
--  $Id: getTreeList_func.sql 9997 2007-03-01 12:21:46Z overeem $
--

--
-- getDefaultTemplates ()
-- 
-- Get a list of treeID's of the default template trees.
--
-- Authorisation: none
--
-- Tables: 	otdbtree	read
--			otdbuser	read
--			campaign	read
--
-- Types:	templateInfo
--
CREATE OR REPLACE FUNCTION getDefaultTemplates()
  RETURNS SETOF templateInfo AS '
	DECLARE
		vRecord					RECORD;
		TSTemplate CONSTANT		INT2 := 10;

	BEGIN
	  -- do selection
	  FOR vRecord IN  
		SELECT treeID, 
			   name
		FROM   OTDBtree where state=TSTemplate
	  LOOP
		RETURN NEXT vRecord;
	  END LOOP;
	  RETURN;
	END
' LANGUAGE plpgsql;

