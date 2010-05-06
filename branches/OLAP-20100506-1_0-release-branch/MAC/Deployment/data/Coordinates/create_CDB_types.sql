--
--	create_CDB_types.sql : Create type that	are	exchanged with the users.
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
-- genCoord
--
CREATE TYPE	genCoord AS	(
	station			VARCHAR(10),		-- object.stationname%TYPE
	objtype			VARCHAR(10),		-- object.type%TYPE
	number			INT4,				-- object.number%TYPE
	ref_frame		VARCHAR(10),		-- generated_coord.ref_frame%TYPE
	X				FLOAT8,				-- generated_coord.X%TYPE
	Y				FLOAT8,				-- generated_coord.Y%TYPE
	Z				FLOAT8				-- generated_coord.Z%TYPE
) ;

--
-- genDeltas
--
CREATE TYPE	genDeltas AS (
	station			VARCHAR(10),		-- object.stationname%TYPE
	objtype			VARCHAR(10),		-- object.type%TYPE
	deltas			FLOAT8[16][3]		-- generated_deltas.detas%TYPE
) ;

--
-- transInfo
--
CREATE TYPE	transInfo AS (
	name			VARCHAR(10),		-- reference_frame.name%TYPE
	Tx				FLOAT8,				-- reference_frame.Tx%TYPE
	Ty				FLOAT8,				-- reference_frame.Ty%TYPE
	Tz				FLOAT8,				-- reference_frame.Tz%TYPE
	sf				FLOAT8,				-- reference_frame.sf%TYPE
	Rx				FLOAT8,				-- reference_frame.Rx%TYPE
	Ry				FLOAT8,				-- reference_frame.Ry%TYPE
	Rz				FLOAT8				-- reference_frame.Rz%TYPE
) ;

--
-- fieldRotation
--
CREATE TYPE	fieldRotation AS	(
	station			VARCHAR(10),		-- object.stationname%TYPE
	objtype			VARCHAR(10),		-- object.type%TYPE
	rotation		FLOAT8  			-- field_rotations.rotation%TYPE
) ;

--
-- normalVector
--
CREATE TYPE	normalVector AS	(
	station			VARCHAR(10),		-- object.stationname%TYPE
	objtype			VARCHAR(10),		-- object.type%TYPE
	vector			FLOAT8[3]			-- normal_vectors.vectors%TYPE
) ;

--
-- rotationMatrix
--
CREATE TYPE	rotationMatrix AS (
	station			VARCHAR(10),		-- object.stationname%TYPE
	objtype			VARCHAR(10),		-- object.type%TYPE
	matrix			FLOAT8[3][3]		-- rotation_matrices.matrix%TYPE
) ;