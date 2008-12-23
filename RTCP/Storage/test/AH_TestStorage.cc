//#  tAH_TestStorage.cc:
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include "AH_TestStorage.h"
#include <Common/LofarLogger.h>
#include <Storage/SubbandWriter.h>
#include <Interface/CorrelatedData.h>

namespace LOFAR
{
  namespace RTCP
  {

    AH_TestStorage::AH_TestStorage()
    {
    }

    AH_TestStorage::~AH_TestStorage() {
      undefine();
    }

    void AH_TestStorage::define(const KeyValueMap&) {

      LOG_TRACE_FLOW_STR("Start of tAH_Storage::define()");
      
      ParameterSet parameterSet("RTCP.parset");
      Parset itsPS(&parameterSet);
       
      
      int nrSubbands = itsPS.nrSubbands();
      
      
      uint nrSubbandsPerPset = itsPS.nrSubbandsPerPset();
      uint nrPsetsPerStorage = itsPS.getUint32("OLAP.psetsPerStorage");
      uint nrWriters = nrSubbands / nrSubbandsPerPset / nrPsetsPerStorage;
      ASSERT(nrSubbands % nrSubbandsPerPset == 0);
      ASSERT(nrSubbands / nrSubbandsPerPset % nrPsetsPerStorage == 0);
      LOG_TRACE_VAR_STR("Creating " << nrWriters << " subband writers ...");
      
      for (unsigned nw = 0; nw < nrWriters; ++nw)
      {
        vector<uint> sbIDs(nrSubbandsPerPset * nrPsetsPerStorage);
	for (uint i = 0; i < nrSubbandsPerPset * nrPsetsPerStorage; ++i) {
          sbIDs[i] = nrSubbandsPerPset * nrPsetsPerStorage * nw + i;     
	  LOG_TRACE_LOOP_STR("Writer " << nw << ": sbIDs[" << i << "] = " 
                             << sbIDs[i]);
        }

        SubbandWriter wh(&itsPS, 0);
      }
    
      LOG_TRACE_FLOW_STR("Finished define()");

    }
  
    void AH_TestStorage::setTestPattern(DH_Visibilities &dh, int factor) {
      unsigned nrPolarizations = itsPS.getUint32("Observation.nrPolarisations");
      unsigned nrChannels      = itsPS.nrChannelsPerSubband();
      unsigned nrStations      = itsPS.nrStations();
      unsigned nrBaselines     = (nrStations + 1) * nrStations / 2;

      for (unsigned bl = 0; bl < nrBaselines; bl++) {
        for (unsigned ch = 0; ch < nrChannels; ch++) {
          // Set number of valid samples
          //dh.getNrValidSamples(bl, ch) = bl * ch;

          // Set visibilities
          for (unsigned pol1 = 0; pol1 < nrPolarizations; pol1 ++) {
            for (unsigned pol2 = 0; pol2 < nrPolarizations; pol2 ++) {
              //dh.getVisibility(bl, ch, pol1, pol2) = makefcomplex(bl + ch, factor * (pol1 + pol2));
            }
          }
        }
      }
    }

    void AH_TestStorage::prerun() {
      // Fill inDHs here
      for (uint i = 0; i < itsInDHs.size(); i++)
      {
        //itsInDHs[i]->init();
        setTestPattern(*itsInDHs[i], i);
      }
    }

    void AH_TestStorage::run(int nsteps) {

      for (int i = 0; i < nsteps; i++) {
        for (uint nrInp = 0; nrInp < itsInConns.size(); nrInp++)
        {
          //itsInConns[nrInp]->write();
        }

        LOG_TRACE_LOOP_STR("processing run " << i );
        cout<<"run "<<i+1<<" of "<<nsteps<<endl;
        //getComposite().process();
      
      }    
    }

    void AH_TestStorage::postrun() {
      // check outresult here
      // do an assert or exit(1) if results are not correct

    }

    void AH_TestStorage::undefine() {
      for (uint i = 0; i < itsInDHs.size(); i++)
      {
        //delete itsInDHs[i];
        //delete itsInConns[i];
      }
      itsInDHs.clear();
      itsInConns.clear();
    }  

    void AH_TestStorage::quit() {
    }

  } // namespace RTCP

} // namespace LOFAR


