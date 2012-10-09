--
-- create_CDB_tables.sql : Creates  all   the   tables of the coordinates subsystem.
--
-- Copyright (C) 2008
-- ASTRON (Netherlands  Foundation for Research in Astronomy)
-- P.O.Box  2, 7990  AA Dwingeloo, The Netherlands, seg@astron.nl
--
-- This program is   free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as  published by
-- the   Free Software Foundation; either version 2 of the License, or
-- (at   your option) any later version.
--
-- This program is   distributed in the hope that it  will be  useful,
-- but   WITHOUT  ANY   WARRANTY; without even the implied warranty  of
-- MERCHANTABILITY   or FITNESS FOR A PARTICULAR   PURPOSE.  See the
-- GNU   General  Public License for more details.
--
-- You   should have received a copy   of the GNU General Public License
-- along with this   program; if not, write to the Free Software
-- Foundation, Inc., 59 Temple   Place, Suite 330, Boston, MA  02111-1307  USA
--
-- $Id: $
--
DROP SCHEMA cdb   CASCADE;
CREATE SCHEMA cdb;

--
-- object_type table
--
CREATE TABLE object_type (
   name     VARCHAR(10)    NOT   NULL,
   CONSTRAINT  object_type_uniq  UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO object_type VALUES ('LBA');
INSERT INTO object_type VALUES ('HBA');
INSERT INTO object_type VALUES ('HBA0');
INSERT INTO object_type VALUES ('HBA1');
INSERT INTO object_type VALUES ('CLBA');
INSERT INTO object_type VALUES ('CHBA0');
INSERT INTO object_type VALUES ('CHBA1');
INSERT INTO object_type VALUES ('CHBA');
INSERT INTO object_type VALUES ('GPS');
INSERT INTO object_type VALUES ('marker');


--
-- reference_system
--
CREATE TABLE reference_system (
   name     VARCHAR(10)    NOT   NULL,
   CONSTRAINT  reference_system_uniq   UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO reference_system VALUES ('ETRS89');
INSERT INTO reference_system VALUES ('ITRS');


--
-- reference_frame
--
CREATE TABLE reference_frame (
   name        VARCHAR(10)    NOT   NULL,
   Tx          FLOAT8         NOT   NULL,
   Ty          FLOAT8         NOT   NULL,
   Tz          FLOAT8         NOT   NULL,
   sf          FLOAT8         NOT   NULL,
   Rx          FLOAT8         NOT   NULL,
   Ry          FLOAT8         NOT   NULL,
   Rz          FLOAT8         NOT   NULL,
   CONSTRAINT  reference_frame_uniq UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO reference_frame   VALUES ('ETRF89','0.0','0.0','0.0','0.0','0.0','0.0','0.0');
INSERT INTO reference_frame   VALUES ('AFREF','0.0','0.0','0.0','0.0','0.0','0.0','0.0');
INSERT INTO reference_frame   VALUES ('ITRF1997','41E-3','41E-3','-49E-3','0.0','0.97E-9','2.42E-9','-3.15E-9');
INSERT INTO reference_frame   VALUES ('ITRF2000','54E-3','51E-3','-48E-3','0.0','0.39E-9','2.376E-9','-3.84E-9');
INSERT INTO reference_frame   VALUES ('ITRF2005','56E-3','48E-3','-37E-3','0.0','0.262E-9','2.511E-9','-3.786E-9');


--
-- measurement_method
--
CREATE TABLE measurement_method  (
   name     VARCHAR(10)    NOT   NULL,
   CONSTRAINT  measurement_method_uniq UNIQUE(name)
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
   name     VARCHAR(40)    NOT   NULL,
   CONSTRAINT  personnel_uniq UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO personnel VALUES ('');
INSERT INTO personnel VALUES ('Donker');
INSERT INTO personnel VALUES ('Brentjens');
INSERT INTO personnel VALUES ('Overeem');
INSERT INTO personnel VALUES ('Schoenmakers');


--
-- station
--
CREATE TABLE station (
   name     VARCHAR(10)    NOT   NULL,
   location VARCHAR(35)    NOT   NULL,
   CONSTRAINT  station_uniq   UNIQUE(name)
) WITHOUT OIDS;
INSERT INTO station VALUES ('CS001', 'Exloo (Beeksdijk)');
INSERT INTO station VALUES ('CS002', 'Exloo (Beeksdijk superterp)');
INSERT INTO station VALUES ('CS003', 'Exloo (Beeksdijk superterp)');
INSERT INTO station VALUES ('CS004', 'Exloo (Beeksdijk superterp)');
INSERT INTO station VALUES ('CS005', 'Exloo (Beeksdijk superterp)');
INSERT INTO station VALUES ('CS006', 'Exloo (Beeksdijk superterp)');
INSERT INTO station VALUES ('CS007', 'Exloo (Beeksdijk superterp)');
INSERT INTO station VALUES ('CS008', '??');
INSERT INTO station VALUES ('CS009', '??');
INSERT INTO station VALUES ('CS010', '??');
INSERT INTO station VALUES ('CS011', 'Exloo (Beeksdijk)');
INSERT INTO station VALUES ('CS012', '??');
INSERT INTO station VALUES ('CS013', 'Exloo (Beeksdijk)');
INSERT INTO station VALUES ('CS014', '??');
INSERT INTO station VALUES ('CS015', '??');
INSERT INTO station VALUES ('CS016', '??');
INSERT INTO station VALUES ('CS017', 'Exloo (Vosholtdijk)');
INSERT INTO station VALUES ('CS018', '??');
INSERT INTO station VALUES ('CS019', '??');
INSERT INTO station VALUES ('CS020', '??');
INSERT INTO station VALUES ('CS021', 'Exloo (Beeksdijk)');
INSERT INTO station VALUES ('CS022', '??');
INSERT INTO station VALUES ('CS023', '??');
INSERT INTO station VALUES ('CS024', 'Exloo (Beeksdijk)');
INSERT INTO station VALUES ('CS025', '??');
INSERT INTO station VALUES ('CS026', 'Exloo (Vosholtsdijk)');
INSERT INTO station VALUES ('CS027', '??');
INSERT INTO station VALUES ('CS028', 'Exloo (Nieuwedijk)');
INSERT INTO station VALUES ('CS029', '??');
INSERT INTO station VALUES ('CS030', 'Exloo (Beeksdijk)');
INSERT INTO station VALUES ('CS031', 'Exloo (Beeksdijk)');
INSERT INTO station VALUES ('CS032', 'Exloo (Achterste velddijk)');
INSERT INTO station VALUES ('CS101', 'Exloo (Nieuwedijk)');
INSERT INTO station VALUES ('CS102', '??');
INSERT INTO station VALUES ('CS103', 'Exloerveen (Zuiderstraat)');
INSERT INTO station VALUES ('CS201', 'Exloo (Vosholtsdijk)');
INSERT INTO station VALUES ('CS301', 'Exloo (Noordveensdijk)');
INSERT INTO station VALUES ('CS302', 'Exloo/Borger (Buinerweg)');
INSERT INTO station VALUES ('CS401', 'Exloo (Achterste velddijk)');
INSERT INTO station VALUES ('CS501', 'Exloo (Zuurdijk)');
INSERT INTO station VALUES ('RS104', '??');
INSERT INTO station VALUES ('RS105', '??');
INSERT INTO station VALUES ('RS106', 'Valthermond (Drentse Mondenweg)');
INSERT INTO station VALUES ('RS107', 'ter Wisch (Schaalbergerweg)');
INSERT INTO station VALUES ('RS108', '??');
INSERT INTO station VALUES ('RS109', '??');
INSERT INTO station VALUES ('RS202', '??');
INSERT INTO station VALUES ('RS203', '??');
INSERT INTO station VALUES ('RS204', '??');
INSERT INTO station VALUES ('RS205', 'Valthe (Exloerweg)');
INSERT INTO station VALUES ('RS206', 't Haantje (t Haantje)');
INSERT INTO station VALUES ('RS207', 'Nieuw Dordrecht (??)');
INSERT INTO station VALUES ('RS208', 'Schoonebeek (Veenschapsweg)');
INSERT INTO station VALUES ('RS209', '??');
INSERT INTO station VALUES ('RS210', 'Weerselo (??)');
INSERT INTO station VALUES ('RS303', '??');
INSERT INTO station VALUES ('RS304', '??');
INSERT INTO station VALUES ('RS305', 'Westdorp (??)');
INSERT INTO station VALUES ('RS306', 'Ellertshaar (Boerveensweg)');
INSERT INTO station VALUES ('RS307', 'Oldeveen (Oldeveen)');
INSERT INTO station VALUES ('RS308', 'Wijster (Emelangen)');
INSERT INTO station VALUES ('RS309', 'Stegeren (Karshoekweg)');
INSERT INTO station VALUES ('RS310', 'Onna (??)');
INSERT INTO station VALUES ('RS311', 'Dwingeloo (Oude Hoogeveensedijk)');
INSERT INTO station VALUES ('RS402', '??');
INSERT INTO station VALUES ('RS403', '??');
INSERT INTO station VALUES ('RS404', 'Borger (??)');
INSERT INTO station VALUES ('RS405', '??');
INSERT INTO station VALUES ('RS406', 'Gieten (Braamakkers)');
INSERT INTO station VALUES ('RS407', 'Oud Annerveen (Tolweg)');
INSERT INTO station VALUES ('RS408', 'Zuidvelde (De Fledders)');
INSERT INTO station VALUES ('RS409', 'Fochteloo (??)');
INSERT INTO station VALUES ('RS410', 'Joure (??)');
INSERT INTO station VALUES ('RS411', 'Taarlo (Osdijk)');
INSERT INTO station VALUES ('RS412', 'Westerbork (Schattenberg)');
INSERT INTO station VALUES ('RS413', 'Hoornsterzwaag (Kapellewei)');
INSERT INTO station VALUES ('RS502', '??');
INSERT INTO station VALUES ('RS503', 'Buinen (Koedijk)');
INSERT INTO station VALUES ('RS504', '??');
INSERT INTO station VALUES ('RS505', '??');
INSERT INTO station VALUES ('RS506', 'Onstwedde (Vledderhuizen)');
INSERT INTO station VALUES ('RS507', 'Alteveer (Barkelazwet)');
INSERT INTO station VALUES ('RS508', 'Nieuwolda (Niewolda)');
INSERT INTO station VALUES ('RS509', 'Roodeschool (Hooilandseweg)');
INSERT INTO station VALUES ('DE601', 'Effelsberg');
INSERT INTO station VALUES ('DE602', 'UnterWeilenbach (Garching)');
INSERT INTO station VALUES ('DE603', 'Tautenburg');
INSERT INTO station VALUES ('DE604', 'Potsdam');
INSERT INTO station VALUES ('DE605', 'Juelich');
INSERT INTO station VALUES ('FR606', 'Nancy');
INSERT INTO station VALUES ('SE607', 'Onsala');
INSERT INTO station VALUES ('UK608', 'Chillbolton');
INSERT INTO station VALUES ('FI609', 'Kaira');

