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
			CN_Processing(const Parset &, Stream *inputStream, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &);
			~CN_Processing();

    virtual void	process(unsigned);

  private:
    double		blockAge(); // age of the current block, in seconds since it was observed by the stations
    void		transposeInput();
    int			transposeBeams(unsigned block);
    void		filter();
    void		dedisperseBeforeBeamForming();
    void		dedisperseAfterBeamForming(unsigned beam);
    void		preCorrelationFlagging();
    void		mergeStations();
    void		formBeams(unsigned firstBeam, unsigned nrBeams);
    void		receiveBeam(unsigned beam);
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

    double		itsStartTime, itsIntegrationTime;
    unsigned		itsBlock;
    unsigned		itsNrStations;
    unsigned		itsNrSubbands;
    unsigned		itsNrSubbandsPerPset;
    unsigned		itsNrSubbandsPerPart;
    unsigned		itsNrPartsPerStokes;
    unsigned		itsNrBeams;
    unsigned		itsNrStokes; // the number of polarizations/stokes that will be split off per beam during the transpose
    unsigned		itsNrBeamsPerPset;
    unsigned		itsNrChannels;
    unsigned		itsNrSamplesPerIntegration;
    unsigned		itsPhaseTwoPsetSize, itsPhaseThreePsetSize;
    unsigned		itsPhaseTwoPsetIndex, itsPhaseThreePsetIndex;
    bool		itsPhaseThreeExists, itsPhaseThreeDisjunct;
    unsigned		itsUsedCoresPerPset, itsMyCoreIndex, itsNrPhaseOneTwoCores, itsNrPhaseThreeCores;

    Stream		*itsInputStream;
    SmartPtr<Stream>	itsFilteredDataStream;
    SmartPtr<Stream>	itsCorrelatedDataStream;
    SmartPtr<Stream>	itsIncoherentStokesStream;
    SmartPtr<Stream>	itsFinalBeamFormedDataStream;
    SmartPtr<Stream>	itsFinalCoherentStokesDataStream;
    SmartPtr<Stream>	itsTriggerDataStream;

    const LocationInfo	&itsLocationInfo;
    std::vector<double> itsCenterFrequencies;
    SmartPtr<Ring>	itsCurrentSubband, itsCurrentBeam;
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
    SmartPtr<StokesData>			itsTransposedCoherentStokesData;
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
