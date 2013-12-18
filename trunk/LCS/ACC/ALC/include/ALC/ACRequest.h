//# ACRequest.h: small structure used for comm. with ACDaemon
//#
//# Copyright (C) 2002-2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#ifndef LOFAR_ACC_ACREQUEST_H
#define LOFAR_ACC_ACREQUEST_H

// \file
// Small structure used for communication with the ACDaemon

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes

namespace LOFAR {
  namespace ACC {
    namespace ALC {
// \addtogroup ALC
// @{

#define	ACREQUESTNAMESIZE 80
#define ACREQUEST_VERSION 0x0100
enum	ACRstate { ACRloaded = 0, ACRnew, ACRok, ACRlosing };

// The ACRequest structure is exchanged with the ACDaemon to request an
// Application Controller.
struct ACRequest
{
	// \name Request
	// The following fields are send by the requester.
	// @{

	// Versionnumber of this structure, is always ACREQUEST_VERSION
	uint16	itsVersion;

	// Uniq request information sent by the client
	char	itsRequester [ACREQUESTNAMESIZE];

	// Number of processes the user will start
	uint16	itsNrProcs;

	// Expected lifetime of application in minutes
	uint32	itsLifetime;

	// Activity of AC (1/2/3: low/medium/high)
	uint16	itsActivityLevel;

	// Architecture code (0 = Intel, 1 = Blue Gene)
	uint16	itsArchitecture;
	// @}

	// \name Answer
	// Based on the information provided by the requester the ACDaemon
	// (re)assigns an AC to the requester.
	// @{

	// Address of the machine the AP is started on (htonl format)
	uint32	itsAddr;			// in_addr_t

	// portnr the AP will be listening on (htons format)
	uint16	itsPort;			// in_port_t
	// @}

	// \name Internal information
	// @{
	
	// Time last ping message was received.
	//#time_t	itsPingtime;	 == signed long
	int32	itsPingtime;

	// Internal state administration
	uint16	itsState;		// loaded, new, ok, losing

	// @}
} __attribute__ ((packed));

// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
