//  Lorrays.h: Define Array type to use (Blitz or AIPS++)
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#ifndef COMMON_LORRAYS_H
#define COMMON_LORRAYS_H

#include <config.h>
    
#ifndef COMMON_LONUMERICS_H
  #include <Common/Lonumerics.h>
#endif
    
<<<<<<< Lorrays.h
//#if defined(HAVE_BLITZ) && !defined(USE_AIPS_ARRAYS)
  #include <Common/Lorrays-Blitz.h>
//#else
//  #include <Common/Lorrays-Aips.h>
//#endif
=======
#if !defined(HAVE_BLITZ) 
  #error Blitz not configured
#endif

#include <Common/Lorrays-Blitz.h>
>>>>>>> 1.5
    
#endif
