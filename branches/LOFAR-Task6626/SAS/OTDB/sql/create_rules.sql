--
--  create_rules.sql: Define update rules
--
--  Copyright (C) 2012
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
--  $Id: create_types.sql 18624 2011-07-27 15:34:51Z schoenmakers $
--

--
-- Creates a rule for updating the modificationDate of the OTDBtree table.
--

DROP RULE IF EXISTS ruleUpdateVIC on vichierarchy;
DROP RULE IF EXISTS ruleUpdateTemplate on victemplate;

CREATE RULE ruleUpdateVIC AS ON UPDATE
	TO vichierarchy WHERE NEW.value <> OLD.value
	DO ALSO UPDATE otdbtree SET modificationDate = now() WHERE treeID = OLD.treeID;

CREATE RULE ruleUpdateTemplate AS ON UPDATE
	TO victemplate WHERE NEW.limits <> OLD.limits
	DO ALSO UPDATE otdbtree SET modificationDate = now() WHERE treeID = OLD.treeID;

