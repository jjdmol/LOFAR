--
--  create_Dataproduct_table.sql: creates the DataProduct table and types
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
--  $Id: create_VIC_tables.sql 16357 2010-09-21 12:10:05Z schoenmakers $
--

DROP TABLE DataproductTable 	CASCADE;
DROP SEQUENCE DataproductID;

--
--
CREATE SEQUENCE	DataproductID;

CREATE TABLE DataproductTable (
	recordID		INT4			NOT NULL DEFAULT nextval('DataproductID'),
	filename		VARCHAR(80)		DEFAULT '',
	nameMask		VARCHAR(80)		DEFAULT '',
	dirMask			VARCHAR(80)		DEFAULT '',
	mountpoint		VARCHAR(80)		DEFAULT '',
	location		VARCHAR(80)		DEFAULT '',
	retentiontime	INT4			NOT NULL DEFAULT 14,
	percWritten		INT4			NOT NULL DEFAULT 0,
	deleted			BOOLEAN			DEFAULT FALSE,
	enabled			BOOLEAN			DEFAULT FALSE,

	CONSTRAINT  Dataproduct_PK     		PRIMARY KEY(recordID)
) WITHOUT OIDS;
CREATE INDEX	Dataproduct_name	ON DataproductTable (filename);

DROP TYPE	Dataproduct		CASCADE;
CREATE TYPE Dataproduct AS (
	recordID		INT4,
	filename		VARCHAR(80),
	nameMask		VARCHAR(80),
	dirMask			VARCHAR(80),
	mountpoint		VARCHAR(80),
	location		VARCHAR(80),
	retentiontime	INT4,
	percWritten		INT4,
	deleted			BOOLEAN,
	enabled			BOOLEAN
);

CREATE OR REPLACE FUNCTION getDataProduct(INT)
RETURNS Dataproduct AS $$
	DECLARE
		vRecord		RECORD;

	BEGIN
		SELECT * INTO vRecord FROM DataproductTable WHERE recordID = $1;
		IF NOT FOUND THEN
			RAISE EXCEPTION 'Dataproduct with recordID=\'%\' not found', $1;
		END IF;
		RETURN vRecord;
	END;
$$ language plpgsql;

CREATE OR REPLACE FUNCTION getDataProduct(VARCHAR(80))
RETURNS Dataproduct AS $$
	DECLARE
		vRecord		RECORD;

	BEGIN
		SELECT * INTO vRecord FROM DataproductTable WHERE filename = $1;
		IF NOT FOUND THEN
			RAISE EXCEPTION 'Dataproduct with filename=\'%\' not found', $1;
		END IF;
		RETURN vRecord;
	END;
$$ language plpgsql;

CREATE OR REPLACE FUNCTION saveDataProduct(Dataproduct)
RETURNS VOID AS $$
	DECLARE
		vRecord		RECORD;

	BEGIN
		SELECT recordID INTO vRecord FROM DataproductTable WHERE recordID = $1.recordID;
		IF NOT FOUND THEN
			RAISE EXCEPTION 'Dataproduct with filename=\'%\' not found', $1;
		END IF;
		RETURN;
	END;
$$ language plpgsql;

-- exportDataProductDefinition()
CREATE OR REPLACE FUNCTION exportDataProductDefinition()
RETURNS TEXT AS $$
	DECLARE
		vResult		TEXT;

	BEGIN
		vResult := 'DataProduct<recordID,filename,nameMask,dirMask,mountpoint,location,retentiontime,percWritten,deleted,enabled>';
		RETURN vResult;
	END;
$$ language plpgsql IMMUTABLE;

-- exportDataProduct(recordnr)
CREATE OR REPLACE FUNCTION exportDataProduct(INT4)
RETURNS TEXT AS $$
    DECLARE
        vRec        RECORD;
        vResult     TEXT;

    BEGIN
        SELECT * INTO vRec FROM DataproductTable WHERE recordID = $1;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Dataproduct with recordnr=\'%\' not found', $1;
        END IF;
        vResult := '[' || text(vRec.recordID) || ',' || textValue(vRec.filename) || ',' || textValue(vRec.nameMask);
        vResult := vResult || ',' || textValue(vRec.dirMask) || ',' || textValue(vRec.mountpoint) || ',' || textValue(vRec.location);
        vResult := vResult || ',' || text(vRec.retentiontime) || ',' || text(vRec.percWritten) || ',' || textValue(vRec.deleted);
        vResult := vResult || ',' || textValue(vRec.enabled) || ']';
        RETURN vResult;
    END;
$$ language plpgsql;

