//#  AH_Storage.cc: 
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$
//
/////////////////////////////////////////////////////////////////////

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Common/lofar_iostream.h>

#include <APS/ParameterSet.h>
#include <CS1_Storage/AH_Storage.h>
#include <Transport/TH_MPI.h>
#include <CS1_Storage/WH_SubbandWriter.h>
#include <CEPFrame/Step.h>

#include <Common/LofarLocators.h>

namespace LOFAR
{
  namespace CS1
  {

    AH_Storage::AH_Storage() : 
      itsCS1PS(0),
      itsStub (0)
    {
    }


    AH_Storage::~AH_Storage()
    {
      undefine();
    }


    void AH_Storage::define(const LOFAR::KeyValueMap&)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"");
      
      ConfigLocator aCL;

      LOG_TRACE_FLOW_STR("Create the top-level composite");
      Composite comp(0, 0, "topComposite");

      // tell the ApplicationHolder this is the top-level composite
      setComposite(comp);

      itsCS1PS = new CS1_Parset(&itsParamSet);
      
      string configFile(aCL.locate("OLAP.parset"));
      
      if (!configFile.empty()) {
		LOG_DEBUG_STR ("Using OLAP.parset file: " << configFile);
		itsCS1PS->adoptFile(configFile);
      }
      else {
	LOG_DEBUG_STR ("NO DEFAULT OLAP.PARSET FOUND");
      }
	
      //itsCS1PS->adoptFile(configFile);
      itsStub = new Stub_BGL(false, true, "BGLProc_Storage", itsCS1PS);
      uint nrSubbands = itsCS1PS->nrSubbands(TH_MPI::getCurrentRank()/itsCS1PS->nrStorageNodes());
      uint nrSubbandsPerPset = itsCS1PS->nrSubbandsPerPset(TH_MPI::getCurrentRank()/itsCS1PS->nrStorageNodes());
      ASSERT(nrSubbandsPerPset > 0);
      uint nrInputChannels = itsCS1PS->useGather() ? 1 : itsCS1PS->nrCoresPerPset();
      ASSERT(nrInputChannels > 0);
      uint nrPsetsPerStorage = itsCS1PS->nrPsetsPerStorage(TH_MPI::getCurrentRank()/itsCS1PS->nrStorageNodes());
      uint maxConcurrent = itsCS1PS->getInt32("OLAP.BGLProc.maxConcurrentComm");
       
      unsigned index = TH_MPI::getCurrentRank()/itsCS1PS->nrStorageNodes();
      
      uint nrWriters = 0;
      
      if (nrSubbands > 0 && itsCS1PS->nrStations(TH_MPI::getCurrentRank()/itsCS1PS->nrStorageNodes()) > 0) {
        ASSERT(nrSubbands % nrSubbandsPerPset == 0);
        ASSERT(nrSubbands / nrSubbandsPerPset % nrPsetsPerStorage == 0);
        
	// We must derive how many WH_SubbandWriter objects we have to
        // create. Each WH_SubbandWriter will write up to \a nrSubbandsPerPset
        // to an AIPS++ Measurement Set.
        nrWriters = nrSubbands / nrSubbandsPerPset / nrPsetsPerStorage;
      }
      else
      {
        time_t now = time(0);
        char buf[26];
        ctime_r(&now, buf);
	
        cout << "time = " << buf <<
#if defined HAVE_MPI
	", rank = " << TH_MPI::getCurrentRank() <<
#endif
	", nrSubbands = " << nrSubbands << endl;

        cout << "time = " << buf <<
#if defined HAVE_MPI
	", rank = " << TH_MPI::getCurrentRank() <<
#endif
	", nrStations = " << itsCS1PS->nrStations(TH_MPI::getCurrentRank()/itsCS1PS->nrStorageNodes()) << endl;
      }
	
      LOG_TRACE_VAR_STR("Creating " << nrWriters << " subband writers ...");
      
      if (nrWriters != 0)
      {
        char whName[32];
        snprintf(whName, 32, "WH_Storage_%03d", TH_MPI::getCurrentRank());
        LOG_TRACE_STAT_STR("Creating " << whName);
        WH_SubbandWriter wh(whName, itsCS1PS); 

        Step step(wh);
        comp.addBlock(step);
	
	if (nrWriters == 1) {
	  if (TH_MPI::getCurrentRank() % itsCS1PS->nrStorageNodes() == 0) {
	    step.runOnNode(TH_MPI::getCurrentRank());
	  }  
	}
	else {
	    step.runOnNode(TH_MPI::getCurrentRank());
	}
	 
        // Connect to BG output
	for (unsigned pset = 0; pset < nrPsetsPerStorage; pset ++) {
	  vector<int> channels;
	  for (unsigned core = 0; core < nrInputChannels; core++) {
	    int channel = pset * nrInputChannels + core;
	    step.getInDataManager(channel).setInBuffer(channel, false, 10);
	    itsStub->connect(TH_MPI::getCurrentRank() % itsCS1PS->nrStorageNodes(), channel, index, step.getInDataManager(channel), channel);
	    channels.push_back(channel);
	  }
	  // limit the number of concurrent incoming connections
	  // actually, we would like to set the number of concurrent
	  // communications for all psets together, but we cannot express this
	  // thus we do this for each pset
	  step.getInDataManager(0).setInRoundRobinPolicy(channels, maxConcurrent);
	}
      }
    }


    void AH_Storage::undefine()
    {
      delete itsCS1PS; itsCS1PS = 0;
      delete itsStub;  itsStub  = 0;
    }


    void AH_Storage::prerun() 
    {
      getComposite().preprocess();
    }
    

    void AH_Storage::run(int steps)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"");
      for (int i = 0; i < steps; i++) {
        LOG_TRACE_LOOP_STR("processing run " << i );
        getComposite().process();
      }
    }


    void AH_Storage::dump() const 
    {
      LOG_TRACE_FLOW_STR("AH_Storage::dump() not implemented"  );
    }


    void AH_Storage::quit()
    {
    }


  } // namespace CS1

} // namespace LOFAR
