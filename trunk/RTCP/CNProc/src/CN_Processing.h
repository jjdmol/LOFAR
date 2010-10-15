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

#include <Stream/Stream.h>
#include <Interface/Config.h>
#if 0
#include <Interface/Parset.h>
#else
#include <Interface/CN_Configuration.h>
#endif

#include <Interface/CN_ProcessingPlan.h>
#include <ArenaMapping.h>
#include <Ring.h>

#include <AsyncTranspose.h>
#include <AsyncTransposeBeams.h>
#include <BeamFormer.h>
#include <PPF.h>
#include <Correlator.h>
#include <Stokes.h>

#include <LocationInfo.h>

#include <string>
#include <boost/noncopyable.hpp>

namespace LOFAR {
namespace RTCP {


class CN_Processing_Base // untemplated helper class
{
  public:
    virtual		~CN_Processing_Base();    

    virtual void	preprocess(CN_Configuration &) = 0;
    virtual void	process(unsigned) = 0;
    virtual void	postprocess() = 0;
};


template <typename SAMPLE_TYPE> class CN_Processing : public CN_Processing_Base, boost::noncopyable
{
  public:
			CN_Processing(Stream *, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &);
			~CN_Processing();

    virtual void	preprocess(CN_Configuration &);
    virtual void	process(unsigned);
    virtual void	postprocess();

  private:
    void                transposeInput();
    bool                transposeBeams(unsigned block);
    void                filter();
    void                mergeStations();
    void                formBeams();
    void                preTransposeBeams();
    void                postTransposeBeams();
    void                postTransposeStokes();
    void                calculateCoherentStokes();
    void                calculateIncoherentStokes();
    void                correlate();

    void                sendOutput( StreamableData *outputData );
    void                finishSendingInput();
    void                finishSendingBeams();

#if 0
    void		checkConsistency(Parset *) const;
#endif

    std::string         itsLogPrefix;

    unsigned            itsNrStations;
    unsigned            itsNrBeamFormedStations;
    bool                itsFlysEye;
    unsigned            itsNrPencilBeams;
    unsigned            itsNrSubbands;
    unsigned            itsNrSubbandsPerPset;
    unsigned            itsNrBeams;
    unsigned            itsNrSubbeams; // the number of polarizations/stokes that will be split off per beam during the transpose
    unsigned            itsMyNrBeams;
    unsigned            itsNrBeamsPerPset;
    unsigned            itsComputeGroupRank;
    unsigned            itsPhaseTwoPsetSize, itsPhaseThreePsetSize;
    unsigned            itsPhaseTwoPsetIndex, itsPhaseThreePsetIndex;
    bool                itsPhaseThreeExists, itsPhaseThreeDisjunct;
    unsigned            itsUsedCoresPerPset, itsMyCoreIndex, itsNrPhaseOneTwoCores, itsNrPhaseThreeCores;
    Stream	        *itsStream;
    Stream              *(*itsCreateStream)(unsigned, const LocationInfo &);
    std::vector<Stream*> itsOutputStreams;
    const LocationInfo	&itsLocationInfo;
    std::vector<double> itsCenterFrequencies;
    Ring                *itsCurrentSubband, *itsCurrentBeam;
    bool		itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree;
    bool		itsStokesIntegrateChannels;
    bool                itsNrStokes;

    CN_ProcessingPlan<SAMPLE_TYPE> *itsPlan;
    ArenaMapping        itsMapping; // needs to be a member to ensure that its lifetime extends beyond that of its data sets

#if defined HAVE_MPI
    AsyncTranspose<SAMPLE_TYPE> *itsAsyncTransposeInput;
    AsyncTransposeBeams         *itsAsyncTransposeBeams;
#endif

    PPF<SAMPLE_TYPE>	*itsPPF;
    BeamFormer          *itsBeamFormer;
    Stokes              *itsCoherentStokes, *itsIncoherentStokes;
    Correlator		*itsCorrelator;
};

} // namespace RTCP
} // namespace LOFAR

#endif
