--
--  create_log_system.sql: Creates a simple logsystem for debug purposes.
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

DROP TABLE log CASCADE;

CREATE TABLE log (
	msg		TEXT
) WITH OIDS;

CREATE OR REPLACE FUNCTION logmsg(TEXT)
  RETURNS VOID AS '
	BEGIN
		INSERT INTO log VALUES ($1);
		RETURN;
	END;
' language plpgsql;

CREATE OR REPLACE FUNCTION clearlog()
  RETURNS VOID AS '
	BEGIN
	  DELETE FROM log;
	  RETURN;
	END;
' language plpgsql;

