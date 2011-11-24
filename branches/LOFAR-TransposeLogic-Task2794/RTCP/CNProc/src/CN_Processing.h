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

#if 0 || !defined HAVE_BGP
#define C_IMPLEMENTATION
#endif

#include <Interface/Allocator.h>
#include <Interface/BeamFormedData.h>
#include <Interface/Config.h>
#include <Interface/CorrelatedData.h>
#include <Interface/FilteredData.h>
#include <Interface/InputData.h>
#include <Interface/Parset.h>
#include <Interface/SmartPtr.h>
#include <Interface/SubbandMetaData.h>
#include <Interface/TransposedData.h>
#include <Interface/TriggerData.h>

#include <Stream/Stream.h>

#include <AsyncTranspose.h>
#include <AsyncTransposeBeams.h>
#include <BeamFormer.h>
#include <Correlator.h>
#include <Dedispersion.h>
#include <LocationInfo.h>
#include <PPF.h>
#include <PreCorrelationFlagger.h>
#include <PostCorrelationFlagger.h>
#include <Ring.h>
#include <Stokes.h>
#include <Trigger.h>

#include <string>


namespace LOFAR {
namespace RTCP {


class CN_Processing_Base // untemplated helper class
{
  public:
    virtual		~CN_Processing_Base();    

    virtual void	process(unsigned) = 0;
};


template <typename SAMPLE_TYPE> class CN_Processing : public CN_Processing_Base
{
  public:
#if defined CLUSTER_SCHEDULING
			CN_Processing(const Parset &, const std::vector<SmartPtr<Stream> > &inputStream, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &, Allocator & = heapAllocator);
#else
			CN_Processing(const Parset &, Stream *inputStream, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &, Allocator & = heapAllocator);
#endif
			~CN_Processing();

    virtual void	process(unsigned);

  private:
    double		blockAge(); // age of the current block, in seconds since it was observed by the stations
#if defined CLUSTER_SCHEDULING
    void		receiveInput();
#else
    void		transposeInput();
#endif
    int			transposeBeams(unsigned block);
    void		filter();
    void		dedisperseBeforeBeamForming();
    void		dedisperseAfterBeamForming(unsigned beam, double dm);
    void		preCorrelationFlagging();
    void		mergeStations();
    void		formBeams(unsigned sap, unsigned firstBeam, unsigned nrBeams);
    void		receiveBeam(unsigned stream);
    void		preTransposeBeams(unsigned inbeam, unsigned outbeam);
    void		postTransposeBeams(unsigned subband);
    void		postTransposeStokes(unsigned subband);
    void		calculateCoherentStokes(unsigned inbeam, unsigned outbeam);
    void		calculateIncoherentStokes();
    void		correlate();
    void		postCorrelationFlagging();

    void		sendOutput(StreamableData *, Stream *);
    void		finishSendingInput();
    void		finishSendingBeams();

    std::string		itsLogPrefix;
    Allocator           &itsBigAllocator;

    double		itsStartTime, itsIntegrationTime;
    unsigned		itsBlock;
    unsigned		itsNrStations;
    unsigned		itsNrSubbands;
    std::vector<unsigned> itsSubbandToSAPmapping;
    std::vector<unsigned> itsNrPencilBeams;
    unsigned		itsMaxNrPencilBeams, itsTotalNrPencilBeams;
    unsigned		itsNrSubbandsPerPset;
    unsigned		itsNrSubbandsPerPart;
    unsigned		itsNrChannels;
    unsigned		itsNrSamplesPerIntegration;
    unsigned		itsPhaseTwoPsetSize, itsPhaseThreePsetSize;
    unsigned		itsPhaseTwoPsetIndex, itsPhaseThreePsetIndex;
    bool		itsPhaseThreeExists, itsPhaseThreeDisjunct;

    const Parset        &itsParset;

    SmartPtr<Stream>	itsFilteredDataStream;
#if defined CLUSTER_SCHEDULING
    const std::vector<SmartPtr<Stream> > &itsInputStreams;
#else
    Stream		*itsInputStream;
#endif
    SmartPtr<Stream>	itsCorrelatedDataStream;
    SmartPtr<Stream>	itsIncoherentStokesStream;
    SmartPtr<Stream>	itsFinalBeamFormedDataStream;
    SmartPtr<Stream>	itsFinalCoherentStokesDataStream;
    SmartPtr<Stream>	itsTriggerDataStream;

    const LocationInfo	&itsLocationInfo;
    const CN_Transpose2 &itsTranspose2Logic;
    std::vector<double> itsCenterFrequencies;
    SmartPtr<Ring>	itsFirstInputSubband, itsCurrentSubband;
    std::vector<double> itsDMs;
    bool		itsFakeInputData;
    bool		itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree;


#if defined HAVE_MPI
    SmartPtr<AsyncTranspose<SAMPLE_TYPE> >	itsAsyncTransposeInput;
    SmartPtr<AsyncTransposeBeams>		itsAsyncTransposeBeams;
#endif

    SmartPtr<InputData<SAMPLE_TYPE> >		itsInputData;
    SmartPtr<SubbandMetaData>			itsInputSubbandMetaData;
    SmartPtr<SubbandMetaData>			itsTransposedSubbandMetaData;
    SmartPtr<TransposedData<SAMPLE_TYPE> > 	itsTransposedInputData;
    SmartPtr<FilteredData>			itsFilteredData;
    SmartPtr<CorrelatedData>			itsCorrelatedData;
    SmartPtr<BeamFormedData>			itsBeamFormedData;
    SmartPtr<PreTransposeBeamFormedData>	itsPreTransposeBeamFormedData;
    SmartPtr<StokesData>			itsIncoherentStokesData;
    SmartPtr<StokesData>			itsCoherentStokesData;
    SmartPtr<TransposedStokesData>		itsTransposedCoherentStokesData;
    SmartPtr<TransposedBeamFormedData>		itsTransposedBeamFormedData;
    SmartPtr<FinalStokesData>			itsFinalCoherentStokesData;
    SmartPtr<FinalBeamFormedData>		itsFinalBeamFormedData;
    SmartPtr<TriggerData>			itsTriggerData;

    SmartPtr<PPF<SAMPLE_TYPE> >			itsPPF;
    SmartPtr<BeamFormer>			itsBeamFormer;
    SmartPtr<Stokes>				itsCoherentStokes;
    SmartPtr<Stokes>				itsIncoherentStokes;
    SmartPtr<Correlator>			itsCorrelator;
    SmartPtr<DedispersionBeforeBeamForming>	itsDedispersionBeforeBeamForming;
    SmartPtr<DedispersionAfterBeamForming>	itsDedispersionAfterBeamForming;
    SmartPtr<PreCorrelationFlagger>		itsPreCorrelationFlagger;
    SmartPtr<PostCorrelationFlagger>		itsPostCorrelationFlagger;
    SmartPtr<Trigger>				itsTrigger;
};

} // namespace RTCP
} // namespace LOFAR

#endif
