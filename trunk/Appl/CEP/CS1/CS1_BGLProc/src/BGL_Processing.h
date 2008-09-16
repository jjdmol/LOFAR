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

#if 0 || !(defined HAVE_BGL || defined HAVE_BGP)
#define C_IMPLEMENTATION
#endif

#include <Stream/Stream.h>
#include <CS1_Interface/CS1_Config.h>
#if 0
#include <CS1_Interface/CS1_Parset.h>
#else
#include <CS1_Interface/BGL_Configuration.h>
#endif

#include <CS1_Interface/Allocator.h>

#include <InputData.h>
#include <FilteredData.h>
#include <TransposedData.h>
#include <CS1_Interface/CorrelatedData.h>

#include <Transpose.h>
#include <AsyncTranspose.h>
#include <PPF.h>
#include <Correlator.h>

#include <LocationInfo.h>

#if defined HAVE_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif



namespace LOFAR {
namespace CS1 {


class BGL_Processing_Base // untemplated helper class
{
  public:
    virtual		~BGL_Processing_Base();    

    virtual void	preprocess(BGL_Configuration &) = 0;
    virtual void	process() = 0;
    virtual void	postprocess() = 0;
};


template <typename SAMPLE_TYPE> class BGL_Processing : public BGL_Processing_Base
{
  public:
			BGL_Processing(Stream *, const LocationInfo &);
			~BGL_Processing();

    virtual void	preprocess(BGL_Configuration &);
    virtual void	process();
    virtual void	postprocess();

  private:
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

    unsigned            itsNrStations;
    unsigned            itsOutputPsetSize;
    Stream		*itsStream;
    const LocationInfo	&itsLocationInfo;
    std::vector<double> itsCenterFrequencies;
    unsigned    	itsFirstSubband, itsCurrentSubband, itsLastSubband, itsSubbandIncrement;
    bool		itsIsTransposeInput, itsIsTransposeOutput;
    
    Arena		*itsArenas[3];
    InputData<SAMPLE_TYPE>	*itsInputData;
    TransposedData<SAMPLE_TYPE>	*itsTransposedData;
    FilteredData	*itsFilteredData;
    CorrelatedData	*itsCorrelatedData;

#if defined HAVE_MPI
    bool                itsDoAsyncCommunication;
    Transpose<SAMPLE_TYPE> *itsTranspose;
    AsyncTranspose<SAMPLE_TYPE> *itsAsyncTranspose;
#endif
    PPF<SAMPLE_TYPE>	*itsPPF;
    Correlator		*itsCorrelator;

#if defined HAVE_BGL
    BGLPersonality	itsPersonality;
    unsigned		itsRankInPset; // core number, not node number!
#endif
};

} // namespace CS1
} // namespace LOFAR

#endif
