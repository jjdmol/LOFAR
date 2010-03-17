--
--  create_CDB_tables.sql : Creates all the tables of the coordinates subsystem.
--
--  Copyright (C) 2008
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
--  $Id: $
--
DROP SCHEMA cdb CASCADE;
CREATE SCHEMA cdb;

--
-- object_type table
--
CREATE TABLE object_type (
	name		VARCHAR(10)		NOT NULL,
	CONSTRAINT	object_type_uniq	UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO object_type VALUES ('LBA');
INSERT INTO object_type VALUES ('HBA');
INSERT INTO object_type VALUES ('GPS');
INSERT INTO object_type VALUES ('marker');


--
-- reference_system
--
CREATE TABLE reference_system (
	name		VARCHAR(10)		NOT NULL,
	CONSTRAINT	reference_system_uniq	UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO reference_system VALUES ('ETRS89');
INSERT INTO reference_system VALUES ('ITRS');


--
-- reference_frame
--
CREATE TABLE reference_frame (
	name		VARCHAR(10)		NOT NULL,
	CONSTRAINT	reference_frame_uniq	UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO reference_frame VALUES ('ETRF89');
INSERT INTO reference_frame VALUES ('AFREF');
INSERT INTO reference_frame VALUES ('ITRF2000');
INSERT INTO reference_frame VALUES ('ITRF2005');
INSERT INTO reference_frame VALUES ('SNARF2006');
INSERT INTO reference_frame VALUES ('SNARF2007');


--
-- measurement_method
--
CREATE TABLE measurement_method (
	name		VARCHAR(10)		NOT NULL,
	CONSTRAINT	measurement_method_uniq	UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO measurement_method VALUES ('GPS');
INSERT INTO measurement_method VALUES ('diff.GPS');
INSERT INTO measurement_method VALUES ('triangle');
INSERT INTO measurement_method VALUES ('interfero');
INSERT INTO measurement_method VALUES ('derived');


--
-- personnel
--
CREATE TABLE personnel (
	name		VARCHAR(40)		NOT NULL,
	CONSTRAINT	personnel_uniq	UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO personnel VALUES ('');
INSERT INTO personnel VALUES ('Brentjens');
INSERT INTO personnel VALUES ('Overeem');


--
-- station
--
CREATE TABLE station (
	name		VARCHAR(10)		NOT NULL,
	location	VARCHAR(35)		NOT NULL,
	CONSTRAINT	station_uniq	UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO station VALUES ('CS001', 'Exloo');
INSERT INTO station VALUES ('CS002', 'Exloo');
INSERT INTO station VALUES ('CS003', 'Exloo');
INSERT INTO station VALUES ('CS004', 'Exloo');
INSERT INTO station VALUES ('CS005', 'Exloo');
INSERT INTO station VALUES ('CS006', 'Exloo');
INSERT INTO station VALUES ('CS007', 'Exloo');
INSERT INTO station VALUES ('CS008', 'Exloo');
INSERT INTO station VALUES ('CS009', 'Exloo');
INSERT INTO station VALUES ('CS010', 'Exloo');
INSERT INTO station VALUES ('CS011', 'Exloo');
INSERT INTO station VALUES ('CS012', 'Exloo');
INSERT INTO station VALUES ('CS013', 'Exloo');
INSERT INTO station VALUES ('CS014', 'Exloo');
INSERT INTO station VALUES ('CS015', 'Exloo');
INSERT INTO station VALUES ('CS016', 'Exloo');
INSERT INTO station VALUES ('CS017', 'Exloo');
INSERT INTO station VALUES ('CS018', 'Exloo');
INSERT INTO station VALUES ('CS019', 'Exloo');
INSERT INTO station VALUES ('CS020', 'Exloo');
INSERT INTO station VALUES ('CS021', 'Exloo');
INSERT INTO station VALUES ('CS022', 'Exloo');
INSERT INTO station VALUES ('CS023', 'Exloo');
INSERT INTO station VALUES ('CS024', 'Exloo');
INSERT INTO station VALUES ('CS025', 'Exloo');
INSERT INTO station VALUES ('CS026', 'Exloo');
INSERT INTO station VALUES ('CS027', 'Exloo');
INSERT INTO station VALUES ('CS028', 'Exloo');
INSERT INTO station VALUES ('CS029', 'Exloo');
INSERT INTO station VALUES ('CS030', 'Exloo');
INSERT INTO station VALUES ('CS031', 'Exloo');
INSERT INTO station VALUES ('CS032', 'Exloo');

INSERT INTO station VALUES ('CS101', 'Unknown');
INSERT INTO station VALUES ('CS102', 'Unknown');
INSERT INTO station VALUES ('CS103', 'Unknown');
INSERT INTO station VALUES ('CS201', 'Unknown');
INSERT INTO station VALUES ('CS301', 'Unknown');
INSERT INTO station VALUES ('CS302', 'Unknown');
INSERT INTO station VALUES ('CS401', 'Unknown');
INSERT INTO station VALUES ('CS501', 'Unknown');

INSERT INTO station VALUES ('RS104', 'Nieuw-Buinen');
INSERT INTO station VALUES ('RS105', 'Unknown');
INSERT INTO station VALUES ('RS106', 'Valthermond');
INSERT INTO station VALUES ('RS107', 'Sellingen');
INSERT INTO station VALUES ('RS108', 'Unknown');
INSERT INTO station VALUES ('RS109', 'Unknown');

INSERT INTO station VALUES ('RS202', 'Unknown');
INSERT INTO station VALUES ('RS203', 'Unknown');
INSERT INTO station VALUES ('RS204', 'Unknown');
INSERT INTO station VALUES ('RS205', 'Valthermond');
INSERT INTO station VALUES ('RS206', 't Haantje');
INSERT INTO station VALUES ('RS207', 'Klazienaveen');
INSERT INTO station VALUES ('RS208', 'Schoonebeek');
INSERT INTO station VALUES ('RS209', 'Unknown');
INSERT INTO station VALUES ('RS210', 'Unknown');

INSERT INTO station VALUES ('RS302', 'Borger');
INSERT INTO station VALUES ('RS303', 'Unknown');
INSERT INTO station VALUES ('RS304', 'Unknown');
INSERT INTO station VALUES ('RS305', 'Unknown');
INSERT INTO station VALUES ('RS306', 'Ellertshaar');
INSERT INTO station VALUES ('RS307', 'Oldeveen');
INSERT INTO station VALUES ('RS308', 'Wijster');
INSERT INTO station VALUES ('RS309', 'Stegeren');
INSERT INTO station VALUES ('RS310', 'Unknown');

INSERT INTO station VALUES ('RS402', 'Unknown');
INSERT INTO station VALUES ('RS403', 'Unknown');
INSERT INTO station VALUES ('RS404', 'Borger');
INSERT INTO station VALUES ('RS405', 'Unknown');
INSERT INTO station VALUES ('RS406', 'Gieten');
INSERT INTO station VALUES ('RS407', 'Gasteren');
INSERT INTO station VALUES ('RS408', 'Zuidvelde');
INSERT INTO station VALUES ('RS409', 'Hoomsterzwaag');
INSERT INTO station VALUES ('RS410', 'Unknown');
INSERT INTO station VALUES ('RS411', 'Unknown');
INSERT INTO station VALUES ('RS412', 'Unknown');
INSERT INTO station VALUES ('RS413', 'Unknown');

INSERT INTO station VALUES ('RS502', 'Unknown');
INSERT INTO station VALUES ('RS503', 'Buinen');
INSERT INTO station VALUES ('RS504', 'Unknown');
INSERT INTO station VALUES ('RS505', 'Unknown');
INSERT INTO station VALUES ('RS506', 'Vledderhuizen');
INSERT INTO station VALUES ('RS507', 'Alteveer');
INSERT INTO station VALUES ('RS508', 't Waar');
INSERT INTO station VALUES ('RS509', 'Oudeschip');

INSERT INTO station VALUES ('DE601', 'Effelsberg');
INSERT INTO station VALUES ('DE602', 'Garching');
INSERT INTO station VALUES ('DE603', 'Tautumberg');
INSERT INTO station VALUES ('DE604', 'Potsdam');
INSERT INTO station VALUES ('DE605', 'Juelich');
INSERT INTO station VALUES ('FR606', 'Nancy');
INSERT INTO station VALUES ('SE607', 'Onsala');
INSERT INTO station VALUES ('UK608', 'Chilbolton');

