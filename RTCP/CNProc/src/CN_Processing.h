//#  CN_Processing.h: polyphase filter and correlator
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

#ifndef LOFAR_CNPROC_CN_PROCESSING_H
#define LOFAR_CNPROC_CN_PROCESSING_H

#if 0 || !(defined HAVE_BGL || defined HAVE_BGP)
#define C_IMPLEMENTATION
#endif

#include <Stream/Stream.h>
#include <Interface/Config.h>
#if 0
#include <Interface/Parset.h>
#else
#include <Interface/CN_Configuration.h>
#endif

#include <Interface/CN_Mode.h>

#include <ArenaMapping.h>

#include <InputData.h>
#include <Interface/FilteredData.h>
#include <TransposedData.h>
#include <Interface/CorrelatedData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/StokesData.h>
#include <Interface/StreamableData.h>

#include <AsyncTranspose.h>
#include <PPF.h>
#include <Correlator.h>
#include <BeamFormer.h>
#include <PencilBeams.h>
#include <Stokes.h>

#include <LocationInfo.h>

#if defined HAVE_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif

#include <string>
#include <boost/noncopyable.hpp>

namespace LOFAR {
namespace RTCP {


class CN_Processing_Base // untemplated helper class
{
  public:
    virtual		~CN_Processing_Base();    

    virtual void	preprocess(CN_Configuration &) = 0;
    virtual void	process() = 0;
    virtual void	postprocess() = 0;
};


template <typename SAMPLE_TYPE> class CN_Processing : public CN_Processing_Base, boost::noncopyable
{
  public:
			CN_Processing(Stream *, const LocationInfo &);
			~CN_Processing();

    virtual void	preprocess(CN_Configuration &);
    virtual void	process();
    virtual void	postprocess();

  private:
    void                transpose();
    void                filter();
    void                formBeams();
    void                formPencilBeams();
    void                calculateIncoherentStokesI();
    void                calculateCoherentStokes();
    void                calculateIncoherentStokes();
    void                correlate();

    void                sendOutput( StreamableData *outputData );
    void                finishSendingInput();

#if 0
    void		checkConsistency(Parset *) const;
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
    unsigned            itsNrPencilBeams;
    unsigned            itsNrSubbands;
    unsigned            itsNrSubbandsPerPset;
    unsigned            itsComputeGroupRank;
    unsigned            itsOutputPsetSize;
    Stream	        *itsStream;
    const LocationInfo	&itsLocationInfo;
    std::vector<double> itsCenterFrequencies;
    unsigned    	itsFirstSubband, itsCurrentSubband, itsLastSubband, itsSubbandIncrement;
    bool		itsIsTransposeInput, itsIsTransposeOutput;
    bool		itsStokesIntegrateChannels;
    
    ArenaMapping        itsMapping;

    InputData<SAMPLE_TYPE>	*itsInputData;
    TransposedData<SAMPLE_TYPE>	*itsTransposedData;
    FilteredData		*itsFilteredData;
    CorrelatedData		*itsCorrelatedData;
    PencilBeamData              *itsPencilBeamData;
    StokesData                  *itsStokesData;
    StokesData                  *itsIncoherentStokesIData;
    StokesDataIntegratedChannels *itsStokesDataIntegratedChannels;
    CN_Mode                     itsMode;
    bool			itsOutputIncoherentStokesI;

#if defined HAVE_MPI
    AsyncTranspose<SAMPLE_TYPE> *itsAsyncTranspose;
#endif

    PPF<SAMPLE_TYPE>	*itsPPF;
    BeamFormer          *itsBeamFormer;
    PencilBeams         *itsPencilBeamFormer;
    Stokes              *itsStokes, *itsIncoherentStokesI;
    Correlator		*itsCorrelator;

#if defined HAVE_BGL
    CNPersonality	itsPersonality;
    unsigned		itsRankInPset; // core number, not node number!
#endif
};

} // namespace RTCP
} // namespace LOFAR

#endif
