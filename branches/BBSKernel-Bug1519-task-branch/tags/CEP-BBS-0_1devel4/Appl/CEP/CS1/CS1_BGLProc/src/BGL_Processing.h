//#  BGL_Processing.h: polyphase filter and correlator
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

#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_BGL_PROCESSING_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_BGL_PROCESSING_H

#if 0 || !defined HAVE_BGL
#define C_IMPLEMENTATION
#endif

#include <Transport/TransportHolder.h>
#include <CS1_Interface/CS1_Config.h>
#if 0
#include <CS1_Interface/CS1_Parset.h>
#else
#include <CS1_Interface/BGL_Configuration.h>
#endif

#include <Allocator.h>
#include <InputData.h>
#include <FilteredData.h>
#include <TransposedData.h>
#include <CorrelatedData.h>

#include <Transpose.h>
#include <PPF.h>
#include <Correlator.h>

#if defined HAVE_BGL
#include <mpi.h>
#endif

#if defined HAVE_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif



namespace LOFAR {
namespace CS1 {


class BGL_Processing {
  public:
			BGL_Processing(TransportHolder *th);
			~BGL_Processing();

#if 0
    void		preprocess(CS1_Parset *parset);
#else
    void		preprocess(BGL_Configuration &);
#endif
    void		process();
    void		postprocess();

    static char		**original_argv;

  //private:
    // TODO: make test program friend of itsTransposedData
#if 0
    void		checkConsistency(CS1_Parset *) const;
#endif

#if defined HAVE_BGL
    void		getPersonality();
#endif

#if defined HAVE_ZOID && defined HAVE_BGL
    void		initIONode() const;
#endif

#if defined HAVE_MPI
    void		printSubbandList() const;
#endif

    TransportHolder	*itsTransportHolder;
    std::vector<double> itsCenterFrequencies;
    unsigned    	itsFirstSubband, itsCurrentSubband, itsLastSubband, itsSubbandIncrement;
    bool		itsIsTransposeInput, itsIsTransposeOutput;

    Heap		*itsHeaps[2];
    InputData		*itsInputData;
    TransposedData	*itsTransposedData;
    FilteredData	*itsFilteredData;
    CorrelatedData	*itsCorrelatedData;

#if defined HAVE_MPI
    Transpose		*itsTranspose;
#endif
    PPF			*itsPPF;
    Correlator		*itsCorrelator;

#if defined HAVE_BGL
    BGLPersonality	itsPersonality;
    unsigned		itsRankInPset; // core number, not node number!
#endif
};

} // namespace CS1
} // namespace LOFAR

#endif
