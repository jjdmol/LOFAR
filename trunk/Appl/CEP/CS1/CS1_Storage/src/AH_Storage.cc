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
#include <CS1_Interface/Stub_BGL_Visibilities.h>
#include <CS1_Storage/WH_SubbandWriter.h>
#include <CEPFrame/Step.h>

namespace LOFAR
{
  namespace CS1
  {

    AH_Storage::AH_Storage() 
      : itsStub (0)
    {
    }


    AH_Storage::~AH_Storage()
    {
    }


    void AH_Storage::define(const LOFAR::KeyValueMap&)
    {
      LOG_TRACE_LIFETIME(TRACE_LEVEL_FLOW,"");

      LOG_TRACE_FLOW_STR("Create the top-level composite");
      Composite comp(0, 0, "topComposite");

      // tell the ApplicationHolder this is the top-level composite
      setComposite(comp);

      itsStub = new Stub_BGL_Visibilities(false, itsParamSet);

      uint nrSubbands = itsParamSet.getUint32("Observation.NSubbands");
      ASSERT(nrSubbands > 0);
      uint nrSubbandsPerCell = itsParamSet.getUint32("General.SubbandsPerCell");
      ASSERT(nrSubbandsPerCell > 0);

      // We must derive how many WH_SubbandWriter objects we have to
      // create. Each WH_SubbandWriter will write up to \a nrSubbandsPerCell
      // to an AIPS++ Measurement Set.
      uint nrWriters = (nrSubbands - 1) / nrSubbandsPerCell + 1;
      LOG_TRACE_VAR_STR("Creating " << nrWriters << " subband writers ...");
      
      for (uint nw = 0; nw < nrWriters; ++nw)
      {
        // For now, we'll assume that the subbands can be sorted and grouped
        // by ID. Hence, the first WH_SubbandWriter will write the first \a
        // nrSubbandsPerCell subbands, the second will write the second \a
        // nrSubbandsPerCell, etc.
        vector<uint> sbIDs(nrSubbandsPerCell);
        for (uint i = 0; i < nrSubbandsPerCell; ++i) {
          sbIDs[i] = nrSubbandsPerCell * nw + i;
          LOG_TRACE_LOOP_STR("Writer " << nw << ": sbIDs[" << i << "] = " 
                             << sbIDs[i]);
        }

        char whName[32];
        snprintf(whName, 32, "WH_Storage_%03d", nw);
        LOG_TRACE_STAT_STR("Creating " << whName);
        WH_SubbandWriter wh(whName, sbIDs, itsParamSet);

        Step step(wh);
        comp.addBlock(step);

        // Each writer will run on a separate node.
        step.runOnNode(nw);

        // Connect to BG output
        itsStub->connect(nw, nw, step.getInDataManager(nw), nw);
      }

#ifdef HAVE_MPI
      ASSERTSTR (TH_MPI::getNumberOfNodes() ==  nrWriters, 
                 TH_MPI::getNumberOfNodes() << " == " << nrWriters);
#endif
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
      delete itsStub;
      itsStub = 0;
    }


  } // namespace CS1

} // namespace LOFAR
