//#  FIR.h: header files for BGL assembly
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FFT_ASM_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FFT_ASM_H

#if defined HAVE_BGL || defined HAVE_BGP
#include <Common/lofar_complex.h>

namespace LOFAR {
namespace CS1 {

extern "C" {
  void _fft256(const fcomplex in[256], fcomplex out[256]);
};

} // end namespace CS1
} // end namespace LOFAR

#endif
#endif
