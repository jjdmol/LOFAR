--
--	create_CDB_tables.sql :	Creates	all	the	tables of the coordinates subsystem.
--
--	Copyright (C) 2008
--	ASTRON (Netherlands	Foundation for Research	in Astronomy)
--	P.O.Box	2, 7990	AA Dwingeloo, The Netherlands, seg@astron.nl
--
--	This program is	free software; you can redistribute	it and/or modify
--	it under the terms of the GNU General Public License as	published by
--	the	Free Software Foundation; either version 2 of the License, or
--	(at	your option) any later version.
--
--	This program is	distributed	in the hope	that it	will be	useful,
--	but	WITHOUT	ANY	WARRANTY; without even the implied warranty	of
--	MERCHANTABILITY	or FITNESS FOR A PARTICULAR	PURPOSE.  See the
--	GNU	General	Public License for more	details.
--
--	You	should have	received a copy	of the GNU General Public License
--	along with this	program; if	not, write to the Free Software
--	Foundation,	Inc., 59 Temple	Place, Suite 330, Boston, MA  02111-1307  USA
--
--	$Id: $
--

--
-- object table
--
CREATE SEQUENCE	object_type_ID;
CREATE TABLE object	(
	stationname		VARCHAR(10)		NOT	NULL REFERENCES	station(name),		-- CS001,...
	type			VARCHAR(10)		NOT	NULL REFERENCES	object_type(name),	-- LBA,HBA,..
	number			INT4			DEFAULT	0,
	ID				INT4			DEFAULT	nextval('object_type_ID'),
	comment			TEXT,
	CONSTRAINT	object_ID_uniq	UNIQUE(ID),
	CONSTRAINT	object_uniq		UNIQUE(stationname,	type, number)
) WITHOUT OIDS;


--
-- reference_coord
--
CREATE TABLE reference_coord (
	ID				INT4			NOT	NULL REFERENCES	object(ID),
	X				FLOAT8			NOT	NULL,
	Y				FLOAT8			NOT	NULL,
	Z				FLOAT8			NOT	NULL,
	sigma_X			FLOAT8			NOT	NULL,
	sigma_Y			FLOAT8			NOT	NULL,
	sigma_Z			FLOAT8			NOT	NULL,
	ref_system		VARCHAR(10)		NOT	NULL REFERENCES	reference_system(name),	  -- ETRS89,ITRS
	ref_frame		VARCHAR(10)		NOT	NULL REFERENCES	reference_frame(name),	  -- ITRF2005,...
	method			VARCHAR(10)		NOT	NULL REFERENCES	measurement_method(name), -- GPS,triangle,...
	measure_date	DATE			NOT	NULL,
	abs_reference	VARCHAR(20),
	derived_from	TEXT,
	person1			VARCHAR(40)		NOT	NULL REFERENCES	personnel(name),
	person2			VARCHAR(40)		REFERENCES personnel(name) MATCH SIMPLE,
	person3			VARCHAR(40)		REFERENCES personnel(name) MATCH SIMPLE,
	comment			TEXT,
	
	CONSTRAINT	reference_coord_uniq	UNIQUE(ID, measure_date)
) WITHOUT OIDS;


--
-- field_rotations
--
CREATE TABLE field_rotations	(
	ID				INT4			NOT	NULL REFERENCES	object(ID),
	rotation		FLOAT8  		NOT	NULL,
	CONSTRAINT	field_rotations_uniq	UNIQUE(ID)
) WITHOUT OIDS;


--
-- normal_vector
--
CREATE TABLE normal_vectors	(
	ID				INT4			NOT	NULL REFERENCES	object(ID),
	vector			FLOAT8[3]		NOT	NULL,
	CONSTRAINT	normal_vectors_uniq	UNIQUE(ID)
) WITHOUT OIDS;


--
-- rotation_matrices
--
CREATE TABLE rotation_matrices (
	ID				INT4				NOT	NULL REFERENCES	object(ID),
	matrix			FLOAT8[3][3]		NOT	NULL,
	CONSTRAINT	rotation_matrices_uniq	UNIQUE(ID)
) WITHOUT OIDS;


--
-- hba_deltas
--
CREATE TABLE generated_hba_deltas (
	ID				INT4				NOT	NULL REFERENCES	object(ID),
	deltas			FLOAT8[16][3]		NOT	NULL,
	CONSTRAINT	generated_deltas_uniq	UNIQUE(ID)
) WITHOUT OIDS;


--
-- generated_coord
--
CREATE TABLE generated_coord (
	ID				INT4			NOT	NULL REFERENCES	object(ID),
	X				FLOAT8			NOT	NULL,
	Y				FLOAT8			NOT	NULL,
	Z				FLOAT8			NOT	NULL,
	target_date		FLOAT4			NOT	NULL,
	ref_frame		VARCHAR(10)		NOT	NULL REFERENCES	reference_frame(name),

	CONSTRAINT	generated_coord_uniq	UNIQUE(ID, target_date)
) WITHOUT OIDS;


