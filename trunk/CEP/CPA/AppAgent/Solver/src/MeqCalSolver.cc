//# MeqCalSolver.cc: VDM implementation of the meqcalibrater
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$


//# Make AIPS++ Arrays available in Hooks.
#define AIPSPP_HOOKS 1
#include <DMI/AIPSPP-Hooks.h>

#include <Solver/MeqCalSolver.h>
#include <MSVisAgent/MSVisAgentVocabulary.h>

#include <MNS/MeqJonesNode.h>
#include <MNS/MeqStatExpr.h>
#include <MNS/MeqMatrixTmp.h>
#include <MNS/MeqStoredParmPolc.h>
#include <MNS/MeqParmSingle.h>
#include <MNS/MeqPointDFT.h>
#include <MNS/MeqPointSource.h>
#include <MNS/MeqWsrtInt.h>
#include <MNS/MeqWsrtPoint.h>
#include <MNS/MeqLofarPoint.h>
#include <MNS/MeqMatrixComplexArr.h>

#include <casa/Utilities/Regex.h>
#include <casa/Utilities/GenSort.h>
#include <casa/OS/Timer.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MEpoch.h>

#include <unistd.h>
    

using namespace AppState;
using namespace SolverControl;
using namespace VisVocabulary;
using namespace MSVisAgent;


//##ModelId=3EC9F6EC0231
MeqCalSolver::MeqCalSolver()
: itsMEP    (0),
  itsGSMMEP (0)
{}    

//##ModelId=3EC9F6EC0232
MeqCalSolver::~MeqCalSolver()
{
  clear();
}    

//##ModelId=3EC9F6EC0233
void MeqCalSolver::run ()
{
  using namespace SolverControl;
  string out = "";
  try
  {
    DataRecord::Ref initrec;
    // repeat the [re]start sequence as long as the control agent gives us
    // a non-terminal state
    while( control().start(initrec) > 0)
    {
      // [re]initialize i/o agents with record returned by control
      cdebug(1)<<"initializing I/O agents\n";
      if( !input().init(*initrec) )
      {
        control().postEvent(InputInitFailed);
        control().setState(STOPPED);
        continue;
      }
      if( !output().init(*initrec) )
      {
        control().postEvent(OutputInitFailed);
        control().setState(STOPPED);
        continue;
      }
      // set up our parameters from init record
      // default is 1 hr domains
      domain_size = (*initrec)[FDomainSize].as<double>(3600);
      cdebug(1)<< "starting run()\n";
      // run main loop
      DataRecord::Ref header,new_header;
      VisTile::Ref tile;
      control().setStatus(StDomain,DataRecord::Ref(DMI::ANONWR));
      while( control().checkState() > 0 )  // while in a running state
      {
        int instat;
        // -------------- receive data set header
        // check for a cached header (once a new header is received, it stays 
        // in cache until all solutions on the previous data set have been 
        // finished) and/or wait for a header to arrive
        input().resume();
        // no header? wait for a new one to come in
        if( !header.valid() )
        {
          while( !new_header.valid() && control().checkState() > 0 )
          {
            HIID id;
            ObjRef ref;
            instat = input().getNext(id,ref,0,AppEvent::WAIT);
            if( instat > 0 )
            {
              if( instat == HEADER )
              {
                // got header? break out
                new_header = ref.ref_cast<DataRecord>();
                // setup whatever's needed from the header
                cdebug(1)<<"got new header: "<<new_header->sdebug(2)<<endl;
                break;
              }
              else
              {
                cdebug(2)<<"ignoring "<<AtomicID(-instat)<<" while waiting for header\n";
                ref.detach();
              }
            }
            // checks for end-of-data or error on the input stream and changes
            // the control state accordingly
            else
              checkInputState(instat);
          }
          if( control().checkState() <= 0 )
            continue;
        }
        // process new data set header (else keep on using old one)
        if( new_header.valid() )
        {
          // we have a valid header now, process it
          header = new_header;
          processHeader(*initrec, *header);

          // get tile format from header and add a residuals column 
          tileformat_.attach((*header)[FTileFormat].as_p<VisTile::Format>(),DMI::READONLY);
          if( !tileformat_->defined(VisTile::RESIDUALS) ) 
          {
            tileformat_.privatize(DMI::WRITE);
            tileformat_().add(VisTile::RESIDUALS,
                                tileformat_->type(VisTile::DATA),
                                tileformat_->shape(VisTile::DATA));
            // this will be applied to each tile as necessary later on
          }
          if( !tileformat_->defined(VisTile::PREDICT) ) 
          {
            tileformat_.privatize(DMI::WRITE);
            tileformat_().add(VisTile::PREDICT,
                                tileformat_->type(VisTile::DATA),
                                tileformat_->shape(VisTile::DATA));
            // this will be applied to each tile as necessary later on
          }
        }
        // --------- read full domain from input ---------------------------
        in_domain = True;
        domain_start = 0;
        itsVisTiles.resize(ntiles_=0);
        control().setStatus(StDomain,StTileCount,0);
        while( in_domain && control().checkState() > 0 )
        {
          // a tile may be left over from the previous iteration
          if( !tile.valid() )
          {
            // if not, then go look for one from the input agent
            cdebug(4)<<"looking for tile\n";
            HIID id;
            ObjRef ref;
            instat = input().getNext(id,ref,0,AppEvent::WAIT);
            if( instat > 0 )
            { 
              if( instat == DATA )
              {
                tile = ref.ref_cast<VisTile>();
                // cdebug(3)<<"received tile "<<tile->tileId()<<endl;
              }
              else if( instat == FOOTER )
              {
                cdebug(1)<<"got footer"<<endl;
                endDomain();
              }
              else if( instat == HEADER )
              {
                cdebug(1)<<"got new header: "<<new_header->sdebug(2)<<endl;
                cdebug(1)<<"interrupting data stream and ending domain"<<endl;
                new_header = ref.ref_cast<DataRecord>();
                endDomain();
              }
              // got a tile? do nothing (addTileToDomain will be called below)
              // cdebug(3)<<"received tile "<<tile->tileId()<<endl;
            }
            // checks for end-of-data or error on the input stream and changes
            // the control state accordingly
            checkInputState(instat);
          }
          // recheck, have we received a tile? Add it to the domain then.
          // addTileToDomain will detach the ref if it accepts the tile. If
          // the tile belongs to the next domain, it will leave the ref in place
          // and clear the in_domain flag.
          if( tile.valid() )
          {
            // tiles are held as read-only
            tile.privatize(DMI::READONLY|DMI::DEEP);
            addTileToDomain(tile);
          }
        }
        if( control().checkState() <= 0 )
          continue;
        // ought to check that we've actually got a domain of data (we could
        // have received two headers in a row, for example). If no data,
        // do a continue to top of loop. But we won't bother for now.

        // generate a domain header
        DataRecord::Ref domainheader = header.copy();
        // make it writable at the top level, since that's the only place
        // where we'll change any fields
        domainheader.privatize(DMI::WRITE,0);
        // generate new dataset ID
	///        HIID &vdsid = domainheader()[FVDSID];
	  ///vdsid[1] = control().domainNum() + 1;
	  ///vdsid[2] = dataset_seq_num_ = 0;
        // put relevant info into domain header
        domainheader()[FDomainIndex] = control().domainNum();
        domainheader()[FDomainStartTime] = domain_start;
        domainheader()[FDomainEndTime] = domain_end;
        domainheader()[FTileFormat] <<= tileformat_.copy(); 

        // suspend input until we go onto next domain
        input().suspend();

        // We have a full domain of data now. Start the solution loop
        // control state ought to be IDLE at start of every solution. (Otherwise,
        // it's either NEXT_DOMAIN, or a terminal state)
        while( control().state() == IDLE )
        {
          DataRecord::Ref paramrec,endrec;
          // get solution parameters, break out if none
          if( control().startSolution(paramrec) != RUNNING ) {
            break;
          }
          double converge = 1;
          try
          {
            cdebug(2)<< "startSolution: "<<paramrec->sdebug(3)<<endl;
            solution_.unlock() <<= new DataRecord;
            solution_.lock();
            const DataRecord& params = *paramrec;
            if (params[SolvableParm].exists()) 
            {
              setSolvable(params[SolvableParm], params[SolvableFlag]);
              initParms();
            }
            if (params[PeelNrs].exists()) 
            {
              setPeel (params[PeelNrs],
                       params[PredNrs].as_vector<int>(vector<int>()));
            }
            if (params[Ant1].exists()) 
            {
              solveSelect(params[Ant1], params[Ant2], params[AntMode][0],
                          params[CorrSel]);
            }
    //        int niter = params[Niter].as<int>(0);
    //        cdebug(2) << "niter=" << niter << endl;
            control().setStatus(StSolution,solution_.copy());
            bool useSVD = params[UseSVD].as<bool>(false);
            // iterate the solution until stopped
            do 
            {
              solve (useSVD,domainheader);
              converge -= 1;
            }
            while( control().endIteration(solution_) == AppState::RUNNING );
          }
          catch( std::exception &exc )
          {
            control().failSolution(string("solution failed: ")+exc.what());
          }
          // if state is ENDSOLVE, end the solution properly
          if( control().state() == ENDSOLVE )
          {
            cdebug(2)<<"ENDSOLVE, converge="
                     <<converge<<endl;
            int res = control().endSolution(solution_,endrec);
            endSolution(*endrec, domainheader);
          }
          // else we were probably interrupted
          else
          {
            cdebug(2)<<"stopped with state "<<control().stateString()<<", converge="<<converge<<endl;
          }
        }
        // end of solving in this domain, for whatever reason.
        // go back to top of loop and check for state
      }
      // broke out of main loop -- close i/o agents
      input().close();
      output().close();
    }
  }
  catch( AipsError &err )
  {
    out = "exiting with AIPS++ exception: " + err.getMesg();
  }
  catch( std::exception &exc)
  {
    out = "exiting with exception: " + string(exc.what());
  }
  catch( ... )
  {
    out = "exiting with unknown exception";
  }
  // post an error event, if one has occurred
  if( out.length() )
  {
    control().postEvent(SolverErrorEvent,out);
    cdebug(0)<<out<<endl;
  }
  else
  {
    cdebug(1)<<"exiting with control state "<<control().stateString()<<endl;
  }
  control().close();
}

//##ModelId=3EC9F6EC0265
void MeqCalSolver::addTileToDomain (VisTile::Ref &tileref)
{
  const VisTile &tile = *tileref;
  double tiletime = tile.time(0);
  // first tile of new domain
  if( !ntiles_ )
  {
    control().startDomain();
    control().setStatus(StDomain,StDomainStart,domain_start = tiletime);
    control().setStatus(StDomain,StDomainEnd,domain_end = domain_start + domain_size);
    cdebug(2)<<"starting new domain with Tstart="<<domain_start<<endl;
    in_domain = True;
    itsVisTiles.resize(ntiles_=0);
  }
  // does tile belong to next domain?
  else
  {
    if( tiletime >= domain_end )
    {
      cdebug(2)<<"tile time past end of domain, closing domain"<<endl;
      endDomain();
      return;
    }
  }
  // assume the domain size is a multiple of the tile size (in time),
  // so at this point the tile fully belongs to our domain
  // Xfer the tile to the internal container .
  itsVisTiles.push_back (tileref);
  ntiles_++;
  if( !(ntiles_%100) )
  {
    control().setStatus(StDomain,StTileCount,ntiles_);
    control().setStatus(StDomain,StDomainCurrent,tiletime);
  }
  cdebug(3)<<"added tile #"<<ntiles_<<" to domain"<<endl;
  if (ntiles_%500 == 0) {
    cdebug(2) << "#tiles = " << ntiles_ << endl;
  }
}

//##ModelId=3EC9F6EC0267
void MeqCalSolver::checkInputState (int instat)
{
  // if input stream is ERROR or CLOSED, say endOfData() to the 
  // control agent, and end the current domain
  if( instat == AppEvent::ERROR )
  {
    cdebug(2)<<"error on input: "<<input().stateString()<<endl;
    control().endData(InputErrorEvent);
    if( ntiles_ )
      endDomain();
    else
      control().setState(STOPPED);
  }
  // closed the input stream? terminate the transaction
  else if( instat == AppEvent::CLOSED )
  {
    cdebug(2)<<"input closed: "<<input().stateString()<<endl;
    control().endData(InputClosedEvent);
    if( ntiles_ )
      endDomain();
    else
      control().setState(STOPPED);
  }
}

//##ModelId=3EC9F6EC026A
void MeqCalSolver::endDomain ()
{
  control().setStatus(StDomain,StTileCount,ntiles_);
  if( in_domain )
  {
    control().endDomain();
    domain_start = 0;
    in_domain = False;
  }
  itsCorrSel.resize(4);
  itsCorrSel.assign (itsCorrSel.size(), true);
  itsVisTilesSel.resize (itsVisTiles.size());
  itsVisTilesSel.assign (itsVisTilesSel.size(), true);
  cdebug(2) << "enddomain: " << itsVisTiles.size() << " tiles read" << endl;
  // Remove all existing expressions and make them for the new data.
  clearDomain();
  // Find the sources in the sky ParmTable.
  // Attach sourcenr and phaseref to them.
  itsSources = itsGSMMEP->getPointSources (Vector<int>(), itsExprDel);
  for (int i=0; i<itsSources.size(); i++) {
    itsSources[i].setSourceNr (i);
    itsSources[i].setPhaseRef (&itsPhaseRef);
  }
  makeExpr();
  if (!itsCalcUVW) {
    // Fill the UVW coordinates from the MS instead of calculating them.
    fillUVW();
  }
  // initialize the ComplexArr pool with the most frequently used size
  // itsNrChan is the numnber frequency channels
  // 1 is the number of time steps. this code is limited to one timestep only
  MeqMatrixComplexArr::poolActivate(itsNrChan * 1);
}

//##ModelId=3EC9F6EC026C
void MeqCalSolver::endSolution (const DataRecord& endrec,
                                DataRecord::Ref& header)
{
  cdebug(2)<< "end solution: "<<endrec.sdebug(3)<<endl;
  try
  {
    if( endrec[SaveParms].as<bool>(False) )
    {
      saveParms();
      control().postEvent(SolverNotifyEvent,"parameters saved");
    }
    else
      control().postEvent(SolverNotifyEvent,"parameters not saved");
    bool do_peel = endrec[ApplyPeel].as<bool>(False);
    if( endrec[SaveResiduals].as<bool>(False) ) 
    {
      control().postEvent(SolverNotifyEvent,
          string(do_peel ? "peeling and " : "" ) + "generating residuals");
      saveResiduals(header,do_peel);
      control().postEvent(SolverNotifyEvent,"residuals generated");
    }
    else
      control().postEvent(SolverNotifyEvent,"no residuals generated");
  }
  catch( std::exception &exc )
  {
    control().postEvent(FailSolutionEvent,string("error ending solution")+exc.what());
  }
}


//##ModelId=3EC9F6EC024C
void MeqCalSolver::setSolvable (const vector<string>& parms,
                               const vector<bool>& flags)
{
  // Add them to the current settings.
  Assert (parms.size() == flags.size());
  if (parms.size() > 0) {
    int nr = itsSolvParms.size();
    itsSolvParms.resize (nr + parms.size());
    itsSolvFlags.resize (nr + parms.size());
    for (unsigned int i=0; i<parms.size(); i++) {
      itsSolvParms[nr+i] = parms[i];
      itsSolvFlags[nr+i] = flags[i];
    }
  }

  // Only set the solvable parms if expressions are defined.
  if (itsStatUVW.size() == 0  ||  itsSolvParms.size() == 0) {
    return;
  }

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  cdebug(1) << "setSolvable" << endl;

  // Test which parameter names match the pattern.
  for (unsigned int i=0; i<itsSolvParms.size(); i++) {
    Regex parmRegex (Regex::fromPattern(String(itsSolvParms[i])));
    // Find all parms matching the parmPatterns
    for (vector<MeqParm*>::const_iterator iter = parmList.begin();
         iter != parmList.end();
         iter++) {
      if (*iter != 0) {
        if (String((*iter)->getName()).matches (parmRegex)) {
          (*iter)->setSolvable(itsSolvFlags[i]);
          cdebug(2) << "parm " << (*iter)->getName() << " set to "
                    << (itsSolvFlags[i] ? "" : "non-") << "solvable" << endl;
        }
      }
    }
  }
}

//##ModelId=3EC9F6EC0252
void MeqCalSolver::setPeel (const vector<int>& peelSourceNrs,
                           const vector<int>& extraSourceNrs)
{
  // Keep the new sources to peel.
  if (peelSourceNrs.size() > 0) {
    itsPeelSourceNrs = peelSourceNrs;
    itsExtraSourceNrs = extraSourceNrs;
  }
  // Only set them if expressions have been built.
  if (itsStatUVW.size() == 0  ||  itsPeelSourceNrs.size() == 0) {
    return;
  }
  vector<int> srcnrs(itsPeelSourceNrs.size() + itsExtraSourceNrs.size());
  unsigned int i;
  for (i=0; i<itsPeelSourceNrs.size(); i++) {
    srcnrs[i] = itsPeelSourceNrs[i];
    cdebug(2) << "Peel source " << srcnrs[i] << endl;
  }
  for (unsigned int j=0; j<itsExtraSourceNrs.size(); j++) {
    srcnrs[i++] = itsExtraSourceNrs[j];
    cdebug(2) << " also predict source " << itsExtraSourceNrs[j] << endl;
  }
  itsSources.setSelected (srcnrs);
}

//##ModelId=3EC9F6EC0255
void MeqCalSolver::solveSelect (const vector<int>& ant1,
                               const vector<int>& ant2,
                               int antMode,
                               const vector<bool>& corrSel)
{
  cdebug(1) << "solveSelect" << endl;
  if (corrSel.size() == 0) {
    itsCorrSel.resize(4);
    itsCorrSel.assign (itsCorrSel.size(), true);
  } else {
    Assert (corrSel.size() == 4);
    itsCorrSel = corrSel;
  }
  // Determine which baselines to use, thus which VisTiles.
  if (ant1.size() > 0  ||  ant2.size() > 0) {
    if (antMode == 3) {
      Assert (ant1.size() == ant2.size());
      int inx = 0;
      for (vector<VisTile::Ref>::const_iterator iter = itsVisTiles.begin();
         iter != itsVisTiles.end();
         iter++, inx++) {
        const VisTile& tile = **iter;
        int a1 = tile.antenna1();
        int a2 = tile.antenna2();
        for (unsigned int i=0; i<ant1.size(); i++) {
          if (a1 == ant1[i]  &&  a2 == ant2[i]) {
            itsVisTilesSel[inx] = true;
            break;
          }
        }
      }
    } else if (antMode == 1) {
      int inx = 0;
      for (vector<VisTile::Ref>::const_iterator iter = itsVisTiles.begin();
         iter != itsVisTiles.end();
         iter++, inx++) {
        const VisTile& tile = **iter;
        int a1 = tile.antenna1();
        int a2 = tile.antenna2();
        bool fnd = false;
        for (unsigned int i=0; i<ant1.size(); i++) {
          if (a1 == ant1[i]) {
            fnd = true;
            break;
          }
        }
        if (!fnd) {
          for (unsigned int i=0; i<ant2.size(); i++) {
            if (a2 == ant2[i]) {
              fnd = true;
              break;
            }
          }
        }
        if (fnd) itsVisTilesSel[inx] = true;
      }
    } else {
      int inx = 0;
      for (vector<VisTile::Ref>::const_iterator iter = itsVisTiles.begin();
         iter != itsVisTiles.end();
         iter++, inx++) {
        const VisTile& tile = **iter;
        int a1 = tile.antenna1();
        int a2 = tile.antenna2();
        bool fnd = false;
        for (unsigned int i=0; i<ant1.size(); i++) {
          if (a1 == ant1[i]) {
            fnd = true;
            break;
          }
        }
        if (fnd) {
          fnd = false;
          for (unsigned int i=0; i<ant2.size(); i++) {
            if (a2 == ant2[i]) {
              fnd = true;
              break;
            }
          }
          if (fnd) itsVisTilesSel[inx] = true;
        }
      }
    }
  }
}

//##ModelId=3EC9F6EC025A
void MeqCalSolver::solve (bool useSVD, DataRecord::Ref& header)
{
  cdebug(1) << "solve; useSVD=" << useSVD << endl;
  AssertMsg (itsNrScid > 0, "No parameters are set to solvable");

  // change the header to reflect an intermediate data set
  ///  HIID &vdsid = header()[FVDSID];
  ///  vdsid[2] = ++dataset_seq_num_;
  header()[FDataType] = DataType_Intermediate;
  header()[FSolveIterNum] = control().iterationNum();
  
  // write the header
  output().put(HEADER,header.copy());

  int nrpoint = 0;
  Timer timer;
  for (unsigned int i=0; i<itsVisTiles.size(); i++) 
  {
    if( !(i%100) )
      control().setStatus(StSolverControl,StSolutionTileCount,int(i));
    if (itsVisTilesSel[i]) {
      // Privatize the tile for writing (this is a no-op if we have the only
      // copy). This is good practice to minimize data copying between
      // threads: we hold a writable ref to the tile only as long as we're 
      // really writing to it.
      itsVisTiles[i].privatize(DMI::WRITE|DMI::DEEP);
      VisTile& visTile = itsVisTiles[i];
      // apply format if we don't have a residuals column in this tile
      if( !visTile.format().defined(VisTile::RESIDUALS) )
        visTile.changeFormat(tileformat_);
      
      // update the tile ID with the new dataset ID
      ///      visTile.setTileId(-1,-1,vdsid);
      
      cdebug(3) << "Tile: "<<visTile.sdebug(5)<<endl;
      
      // Do a predict for the elements in this range.
      bool showd = false;
      //bool showd = i==0;
      int ant1 = visTile.antenna1();
      int ant2 = visTile.antenna2();
      int blnr = ifrNumber(ant1,ant2);
      Assert (blnr < int(itsBLIndex.size()));
      int blindex = itsBLIndex[blnr];
      Assert (blnr >= 0);
      for (VisTile::const_iterator iter = visTile.begin();
           iter != visTile.end();
           iter++) {
        // First make a domain.
        double time = iter.time();
        double interval = iter.interval();
        MeqDomain domain(time - interval/2, time + interval/2,
                         itsStartFreq, itsEndFreq);
        MeqRequest request(domain, 1, itsNrChan, itsNrScid);
        MeqJonesExpr& expr = *(itsExpr[blindex]);
        expr.calcResult (request);
        // Form the equations for this row.
        // Complex values are separated in real and imaginary.
        // Make a default derivative vector with values 0.
        MeqMatrix defaultDeriv (DComplex(0,0), 1, itsNrChan);
        const complex<double>* defaultDerivPtr = defaultDeriv.dcomplexStorage();
        // Get the data of this row for the given channels.
        const LoMat_fcomplex data (iter.data());
        int npol = data.shape()[0];
        // Calculate the derivatives and get pointers to them.
        // Use the default if no perturbed value defined.
        vector<const complex<double>*> derivs(npol*itsNrScid);
        bool foundDeriv = false;
        if (showd) {
          cout << "xx val " << expr.getResult11().getValue() << endl;;
          cout << "xy val " << expr.getResult12().getValue() << endl;;
          cout << "yx val " << expr.getResult21().getValue() << endl;;
          cout << "yy val " << expr.getResult22().getValue() << endl;;
        }
        for (int scinx=0; scinx<itsNrScid; scinx++) {
          MeqMatrix val;
          if (expr.getResult11().isDefined(scinx)) {
            val = expr.getResult11().getPerturbedValue(scinx);
            if (showd) {
              cout << "xx" << scinx << ' ' << val;
            }
            val -= expr.getResult11().getValue();
            if (showd) {
              cout << "  diff=" << val;
            }
            ///cout << "Diff  " << val << endl;
            val /= expr.getResult11().getPerturbation(scinx);
            if (showd) {
              cout << "  der=" << val << endl;
            }
            ///cout << "Deriv " << val << endl;
            derivs[scinx] = val.dcomplexStorage();
            foundDeriv = true;
          } else {
            derivs[scinx] = defaultDerivPtr;
          }
          if (npol == 4) {
            if (expr.getResult12().isDefined(scinx)) {
              val = expr.getResult12().getPerturbedValue(scinx);
              if (showd) {
                cout << "xy" << scinx << ' ' << val;
              }
              val -= expr.getResult12().getValue();
              if (showd) {
                cout << "  diff=" << val;
              }
              val /= expr.getResult12().getPerturbation(scinx);
              if (showd) {
                cout << "  der=" << val << endl;
              }
              derivs[scinx + itsNrScid] = val.dcomplexStorage();
              foundDeriv = true;
            } else {
              derivs[scinx + itsNrScid] = defaultDerivPtr;
            }
            if (expr.getResult21().isDefined(scinx)) {
              val = expr.getResult21().getPerturbedValue(scinx);
              if (showd) {
                cout << "yx" << scinx << ' ' << val;
              }
              val -= expr.getResult21().getValue();
              if (showd) {
                cout << "  diff=" << val;
              }
              val /= expr.getResult21().getPerturbation(scinx);
              if (showd) {
                cout << "  der=" << val << endl;
              }
              derivs[scinx + 2*itsNrScid] = val.dcomplexStorage();
              foundDeriv = true;
            } else {
              derivs[scinx + 2*itsNrScid] = defaultDerivPtr;
            }
          }
          if (npol > 1) {
            if (expr.getResult22().isDefined(scinx)) {
              val = expr.getResult22().getPerturbedValue(scinx);
              if (showd) {
                cout << "yy" << scinx << ' ' << val;
              }
              val -= expr.getResult22().getValue();
              if (showd) {
                cout << "  diff=" << val;
              }
              val /= expr.getResult22().getPerturbation(scinx);
              if (showd) {
                cout << "  der=" << val << endl;
              }
              derivs[scinx + (npol-1)*itsNrScid] = val.dcomplexStorage();
              foundDeriv = true;
            } else {
              derivs[scinx + (npol-1)*itsNrScid] = defaultDerivPtr;
            }
          }
        }
        // Only add to solver if at least one derivative was found.
        // Otherwise these data are not dependent on the solvable parameters.
        if (foundDeriv) {
          // Get pointer to array storage; the data in it is contiguous.
          const fcomplex* dataPtr = data.data();
          vector<double> derivVec(2*itsNrScid);
          double* derivReal = &(derivVec[0]);
          double* derivImag = &(derivVec[itsNrScid]);
          // Fill in all equations.
          if (npol == 1) {
            {
              const MeqMatrix& xx = expr.getResult11().getValue();
              for (int i=0; i<itsNrChan; i++) {
                for (int j=0; j<itsNrScid; j++) {
                  derivReal[j] = derivs[j][i].real();
                  derivImag[j] = derivs[j][i].imag();
                  if (showd) {
                    cout << "derxx: " << j << ' '
                         << derivReal[j] << ' ' << derivImag[j] << endl;
                  }
                }
                DComplex diff (dataPtr[i].real(), dataPtr[i].imag());
                diff -= xx.getDComplex(0,i);
                if (showd) {
                  cout << "diffxx: " << i << ' '
                       << diff.real() << ' ' << diff.imag() << endl;
                }
                double val = diff.real();
                itsSolver.makeNorm (derivReal, 1., &val);
                val = diff.imag();
                itsSolver.makeNorm (derivImag, 1., &val);
                nrpoint++;
              }
            }
          } else if (npol == 2) {
            {
              const MeqMatrix& xx = expr.getResult11().getValue();
              for (int i=0; i<itsNrChan; i++) {
                for (int j=0; j<itsNrScid; j++) {
                  derivReal[j] = derivs[j][i].real();
                  derivImag[j] = derivs[j][i].imag();
                }
                DComplex diff (dataPtr[i*2].real(), dataPtr[i*2].imag());
                diff -= xx.getDComplex(0,i);
                double val = diff.real();
                itsSolver.makeNorm (derivReal, 1., &val);
                val = diff.imag();
                itsSolver.makeNorm (derivImag, 1., &val);
                nrpoint++;
              }
            }
            {
              const MeqMatrix& yy = expr.getResult22().getValue();
              for (int i=0; i<itsNrChan; i++) {
                for (int j=0; j<itsNrScid; j++) {
                  derivReal[j] = derivs[j+itsNrScid][i].real();
                  derivImag[j] = derivs[j+itsNrScid][i].imag();
                }
                DComplex diff (dataPtr[i*2+1].real(), dataPtr[i*2+1].imag());
                diff -= yy.getDComplex(0,i);
                double val = diff.real();
                itsSolver.makeNorm (derivReal, 1., &val);
                val = diff.imag();
                itsSolver.makeNorm (derivImag, 1., &val);
                nrpoint++;
              }
            }
          } else if (npol == 4) {
            {
              const MeqMatrix& xx = expr.getResult11().getValue();
              for (int i=0; i<itsNrChan; i++) {
                for (int j=0; j<itsNrScid; j++) {
                  derivReal[j] = derivs[j][i].real();
                  derivImag[j] = derivs[j][i].imag();
                  if (showd) {
                    cout << "derxx: " << j << ' '
                         << derivReal[j] << ' ' << derivImag[j] << endl;
                  }
                }
                DComplex diff (dataPtr[i*4].real(), dataPtr[i*4].imag());
                diff -= xx.getDComplex(0,i);
                if (showd) {
                  cout << "diffxx: " << i << ' '
                       << diff.real() << ' ' << diff.imag() << endl;
                }
                double val = diff.real();
                itsSolver.makeNorm (derivReal, 1., &val);
                val = diff.imag();
                itsSolver.makeNorm (derivImag, 1., &val);
                nrpoint++;
              }
            }
            {
              const MeqMatrix& xy = expr.getResult12().getValue();
              for (int i=0; i<itsNrChan; i++) {
                for (int j=0; j<itsNrScid; j++) {
                  derivReal[j] = derivs[j+itsNrScid][i].real();
                  derivImag[j] = derivs[j+itsNrScid][i].imag();
                  if (showd) {
                    cout << "derxy: " << i << ' '
                         << derivReal[j] << ' ' << derivImag[j] << endl;
                  }
                }
                DComplex diff (dataPtr[i*4+1].real(), dataPtr[i*4+1].imag());
                diff -= xy.getDComplex(0,i);
                if (showd) {
                  cout << "diffxy: " << i << ' '
                       << diff.real() << ' ' << diff.imag() << endl;
                }
                double val = diff.real();
                itsSolver.makeNorm (derivReal, 1., &val);
                val = diff.imag();
                itsSolver.makeNorm (derivImag, 1., &val);
                nrpoint++;
              }
            }
            {
              const MeqMatrix& yx = expr.getResult21().getValue();
              for (int i=0; i<itsNrChan; i++) {
                for (int j=0; j<itsNrScid; j++) {
                  derivReal[j] = derivs[j+2*itsNrScid][i].real();
                  derivImag[j] = derivs[j+2*itsNrScid][i].imag();
                  if (showd) {
                    cout << "deryx: " << i << ' '
                         << derivReal[j] << ' ' << derivImag[j] << endl;
                  }
                }
                DComplex diff (dataPtr[i*4+2].real(), dataPtr[i*4+2].imag());
                diff -= yx.getDComplex(0,i);
                if (showd) {
                  cout << "diffyx: " << i << ' '
                       << diff.real() << ' ' << diff.imag() << endl;
                }
                double val = diff.real();
                itsSolver.makeNorm (derivReal, 1., &val);
                val = diff.imag();
                itsSolver.makeNorm (derivImag, 1., &val);
                nrpoint++;
              }
            }
            {
              const MeqMatrix& yy = expr.getResult22().getValue();
              for (int i=0; i<itsNrChan; i++) {
                for (int j=0; j<itsNrScid; j++) {
                  derivReal[j] = derivs[j+3*itsNrScid][i].real();
                  derivImag[j] = derivs[j+3*itsNrScid][i].imag();
                  if (showd) {
                    cout << "deryy: " << i << ' '
                         << derivReal[j] << ' ' << derivImag[j] << endl;
                  }
                }
                DComplex diff (dataPtr[i*4+3].real(), dataPtr[i*4+3].imag());
                diff -= yy.getDComplex(0,i);
                if (showd) {
                  cout << "diffyy: " << i << ' '
                       << diff.real() << ' ' << diff.imag() << endl;
                }
                double val = diff.real();
                itsSolver.makeNorm (derivReal, 1., &val);
                val = diff.imag();
                itsSolver.makeNorm (derivImag, 1., &val);
                nrpoint++;
              }
            }
          } else {
            AssertMsg(false, "Number of polarizations should be 1, 2, or 4");
          }
        }
        // Subtract the predicted data from the measured data
        // and return it to the output stream.
        LoMat_fcomplex predata (iter.predict());
        LoMat_fcomplex resdata (iter.residuals());
        int nchan = resdata.shape()[1];
        // Make Blitz vectors of the predicted data.
        LoVec_dcomplex xx (const_cast<complex<double>*>
                           (expr.getResult11().getValue().dcomplexStorage()),
                           blitz::shape(nchan),
                           blitz::neverDeleteData);
        LoVec_dcomplex xy (const_cast<complex<double>*>
                           (expr.getResult12().getValue().dcomplexStorage()),
                           blitz::shape(nchan),
                           blitz::neverDeleteData);
        LoVec_dcomplex yx (const_cast<complex<double>*>
                           (expr.getResult21().getValue().dcomplexStorage()),
                           blitz::shape(nchan),
                           blitz::neverDeleteData);
        LoVec_dcomplex yy (const_cast<complex<double>*>
                           (expr.getResult22().getValue().dcomplexStorage()),
                           blitz::shape(nchan),
                           blitz::neverDeleteData);
	// Subtract 0 to 'cast' from dcomplex to fcomplex.
        if (1 == npol) {
          predata(0,blitz::Range::all()) = xx - 0.;
        } else if (2 == npol) {
          predata(0,blitz::Range::all()) = xx - 0.;
          predata(1,blitz::Range::all()) = yy - 0.;
        } else if (4 == npol) {
          predata(0,blitz::Range::all()) = xx - 0.;
          predata(1,blitz::Range::all()) = xy - 0.;
          predata(2,blitz::Range::all()) = yx - 0.;
          predata(3,blitz::Range::all()) = yy - 0.;
        }
        resdata = data - predata;
      } // end iteration over current tile
      // change the tile back to read-only
      itsVisTiles[i].privatize(DMI::READONLY|DMI::DEEP);
      // dump a copy to the output stream
      output().put(DATA,itsVisTiles[i].copy());
    } // end if( itsVisTileSel[i] )
  } // end loop over tiles
  control().setStatus(StSolverControl,StSolutionTileCount,int(itsVisTiles.size()));
  
  if (Debug(1)) timer.show("fill ");

  Assert (nrpoint >= itsNrScid);
  // Solve the equation.
  uInt rank;
  double fit;
  double stddev;
  double mu;
  cdebug(1) << "Solution before: " << itsSolution << endl;
  // It looks as if LSQ has a bug so that solveLoop and getCovariance
  // interact badly (maybe both doing an invert).
  // So make a copy to separate them.
  Matrix<double> covar;
  Vector<double> errors;
  FitLSQ tmpSolver = itsSolver;
  tmpSolver.getCovariance (covar);
  int nrs = itsSolution.nelements();
  Vector<double> sol(nrs);
  double* solData = itsSolution.doubleStorage();
  for (int i=0; i<itsSolution.nelements(); i++) {
    sol[i] = solData[i];
  }
  bool solFlag = itsSolver.solveLoop (fit, rank, sol, stddev, mu, useSVD);
  for (int i=0; i<itsSolution.nelements(); i++) {
    solData[i] = sol[i];
  }
  tmpSolver.getErrors (errors);
  if (Debug(1)) timer.show("solve");
  cdebug(1) << "Solution after:  " << itsSolution << endl;
  DataRecord &solrec = solution_();
  solrec[StParamValues].replace() = sol;
  solrec[StRank] = int(rank);   // glish only recognizes int, not uint
  solrec[StFit] = fit;
  //  solrec[StErrors] = errors;
  //  solrec[StCoVar ] = covar; 
  solrec[StFlag] = solFlag; 
  solrec[StMu] = mu;
  solrec[StStdDev] = stddev;
  //  solrec[StChi   ] = itsSolver.getChi());
  
  // write footer to output stream
  DataRecord::Ref footer(DMI::ANONWR);
  footer()[StSolution] <<= solution_.copy();
  ///  footer()[FVDSID] = vdsid;
  output().put(FOOTER,footer);

  // Update all parameters.
  const vector<MeqParm*>& parmList = MeqParm::getParmList();
  int i=0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
    {
      if (*iter  &&  (*iter)->isSolvable()) {
        (*iter)->update (itsSolution);
      }
      i++;
    }
        
  //        DataRecord rec;
  //        rec.add ("solflag", solFlag);
  //        rec.add ("sol", GlishArray(sol));
  //        rec.add ("rank", Int(rank));
  //        rec.add ("fit", fit);
  //        rec.add ("diag", GlishArray(errors));
  //        rec.add ("covar", GlishArray(covar));
  //        rec.add ("mu", mu);
  //        rec.add ("stddev", stddev);
  //        rec.add ("chi", itsSolver.getChi());
}

//##ModelId=3EC9F6EC0236
void MeqCalSolver::processHeader (const DataRecord& initrec,
                                 const DataRecord& header)
{
  cdebug(2) << "processHeader:" << endl;
  clear();
  // Get all antenna positions and make parameters for them..
  LoMat_double antPos = header[FAntennaPos];
  itsAntPos.reference (antPos);
  LoShape shp = itsAntPos.shape();
  int numant = shp[shp.size()-1];
  cdebug(2) << " numant:   " << numant << endl;
  // Determine nr of antennas and the highest baseline nr.
  // Allocate and initialize the baseline index.
  int lastbl = ifrNumber(numant-1, numant-1);
  itsBLIndex.resize (1+lastbl);
  itsBLIndex.assign (1+lastbl, -1);
  // Determine start, end, and incr frequency.
  // Check if all channels used have the same width.
  LoVec_double chanWidth = header[FChannelWidth];
  int lastch = chanWidth.shape()[0] - 1;
  int stinx = header[FChannelStartIndex].as<int>(0);
  if (stinx < 0) stinx = 0;
  int endinx = header[FChannelEndIndex].as<int>(lastch);
  if (endinx > lastch) endinx = lastch;
  Assert (stinx <= endinx);
  cdebug(2) << " channels: " << stinx << " - " << endinx << endl;
  itsStepFreq = chanWidth(stinx);
  for (int i=stinx; i<=endinx; i++) {
    Assert (chanWidth(i) == itsStepFreq);
  }
  itsNrChan = 1+endinx-stinx;
  LoVec_double chanFreq = header[FChannelFreq];
  double freq = chanFreq(stinx);
  itsStartFreq = freq - itsStepFreq/2;
  itsEndFreq = itsStartFreq + itsNrChan*itsStepFreq;
  cdebug(2) << " freq:     " << itsStartFreq << " - " << itsEndFreq
            << " (" << itsStepFreq << ")" << endl;
  // Get the phase reference direction.
  LoVec_double phdir = header[FPhaseRef];
  MVDirection dir(phdir(0), phdir(1));
  MDirection phaseRef (dir, MDirection::J2000);
  cdebug(2) << " phasedir: " << phdir(0) << ' ' << phdir(1) << " ("
            << phaseRef << ")" << endl;
  double tim = header[FTime];
  MEpoch ep(Quantity(tim,"s"), MEpoch::UTC);
  cdebug(2) << " time:     " << tim << " (" << ep << ")" << endl;
  itsPhaseRef = MeqPhaseRef(phaseRef, tim);
  // Find the MEP tables to be used.
  cdebug(2) << " mepname:  " << initrec[FMEPName].as<string>() << endl;
  cdebug(2) << " gsmname:  " << initrec[FGSMName].as<string>() << endl;
  cdebug(2) << " calcuvw:  " << initrec[FCalcUVW].as<bool>() << endl;
  cdebug(2) << " modeltp:  " << initrec[FModelType].as<string>() << endl;
  itsMEP = new ParmTable("aips", initrec[FMEPName].as<string>(), "", "");
  itsGSMMEP = new ParmTable("aips", initrec[FGSMName].as<string>(), "", "");
  // Initialize the solver.
  itsSolver = FitLSQ(1, LSQBase::REAL);
  itsCalcUVW = initrec[FCalcUVW];
  itsModelType = initrec[FModelType].as<string>();
}

//##ModelId=3EC9F6EC0239
void MeqCalSolver::clear()
{
  for (vector<MeqStation*>::iterator iter = itsStations.begin();
       iter != itsStations.end();
       iter++) {
    delete *iter;
  }
  itsStations.resize(0);
  itsBLIndex.resize(0);
  delete itsMEP;
  itsMEP = 0;
  delete itsGSMMEP;
  itsGSMMEP = 0;
  // clear up the matrix pool
  MeqMatrixComplexArr::poolDeactivate();
  clearDomain();
}

//##ModelId=3EC9F6EC023D
void MeqCalSolver::clearDomain()
{
  itsPeelSourceNrs.resize(0);
  itsExtraSourceNrs.resize(0);
  itsSolvParms.resize(0);
  itsSolvFlags.resize(0);
  clearExpr();
}

//##ModelId=3EC9F6EC023F
void MeqCalSolver::clearExpr()
{
  for (vector<MeqExpr*>::iterator iter = itsExprDel.begin();
       iter != itsExprDel.end();
       iter++) {
    delete *iter;
  }
  itsExprDel.resize(0);
  for (vector<MeqJonesExpr*>::iterator iter = itsJExprDel.begin();
       iter != itsJExprDel.end();
       iter++) {
    delete *iter;
  }
  itsJExprDel.resize(0);
  for (vector<MeqJonesExpr*>::iterator iter = itsExpr.begin();
       iter != itsExpr.end();
       iter++) {
    delete *iter;
  }
  itsExpr.resize(0);
  for (vector<MeqJonesExpr*>::iterator iter = itsResExpr.begin();
       iter != itsResExpr.end();
       iter++) {
    delete *iter;
  }
  itsResExpr.resize(0);
  for (vector<MeqJonesExpr*>::iterator iter = itsStatExpr.begin();
       iter != itsStatExpr.end();
       iter++) {
    delete *iter;
  }
  itsStatExpr.resize(0);
  for (vector<MeqStatSources*>::iterator iter = itsStatSrc.begin();
       iter != itsStatSrc.end();
       iter++) {
    delete *iter;
  }
  itsStatSrc.resize(0);
  for (vector<MeqLofarStatSources*>::iterator iter = itsLSSExpr.begin();
       iter != itsLSSExpr.end();
       iter++) {
    delete *iter;
  }
  itsLSSExpr.resize(0);
  for (vector<MeqStatUVW*>::iterator iter = itsStatUVW.begin();
       iter != itsStatUVW.end();
       iter++) {
    delete *iter;
  }
  itsStatUVW.resize(0);
  itsBLIndex.assign (itsBLIndex.size(), -1);
  MeqParm::clearParmList();
}

//##ModelId=3EC9F6EC0241
void MeqCalSolver::makeExpr()
{
  // Create the position parm objects fior all stations.
  LoShape shp = itsAntPos.shape();
  int numant = shp[shp.size()-1];
  itsStations.resize(numant);
  itsStations.assign (numant, 0);
  for (int i=0; i<numant; i++) {
    ostringstream str;
    str << "SR" << i+1;
    string name = str.str();
    MeqParmSingle* px = new MeqParmSingle ("AntPosX." + name,
                                           itsAntPos(0,i));
    itsExprDel.push_back (px);
    MeqParmSingle* py = new MeqParmSingle ("AntPosY." + name,
                                           itsAntPos(1,i));
    itsExprDel.push_back (py);
    MeqParmSingle* pz = new MeqParmSingle ("AntPosZ." + name,
                                           itsAntPos(2,i));
    itsExprDel.push_back (pz);
    itsStations[i] = new MeqStation(px, py, pz, name);
  }
  // Initialize the various vectors.
  itsStatUVW  = vector<MeqStatUVW*>     (itsStations.size(),
                                         (MeqStatUVW*)0);
  itsStatSrc  = vector<MeqStatSources*> (itsStations.size(),
                                         (MeqStatSources*)0);
  itsLSSExpr  = vector<MeqLofarStatSources*> (itsStations.size(),
                                              (MeqLofarStatSources*)0);
  itsStatExpr = vector<MeqJonesExpr*>   (itsStations.size(),
                                         (MeqJonesExpr*)0);
  // Make one pass through the data to find the baselines actually used.
  // Also find start and end time.
  itsStartTime = 1e30;
  itsEndTime = 0;
  Bool wsrtModel = (itsModelType == "WSRT");
  Bool asAP = (itsModelType != "LOFAR.RI");
  int nrbl = 0;
  for (unsigned int i=0; i<itsVisTiles.size(); i++) {
    const VisTile& visTile = *(itsVisTiles[i]);
    int ant1 = visTile.antenna1();
    int ant2 = visTile.antenna2();
    int blnr = ifrNumber(ant1,ant2);
    Assert (ant1 < 16384  &&  ant2 < 16384);
    Assert (blnr < int(itsBLIndex.size()));
    if (itsBLIndex[blnr] < 0) {
      nrbl++;
      itsBLIndex[blnr] = ant1*16384 + ant2;  // indicate baseline will be used.
      if (itsStatUVW[ant1] == 0) makeStatExpr (ant1, wsrtModel, asAP);
      if (itsStatUVW[ant2] == 0) makeStatExpr (ant2, wsrtModel, asAP);
    }
    for (VisTile::const_iterator iter = visTile.begin();
         iter != visTile.end();
         iter++) {
      double stime = iter.time() - iter.interval()/2;
      double etime = iter.time() + iter.interval()/2;
      if (stime < itsStartTime) itsStartTime = stime;
      if (etime > itsEndTime)   itsEndTime = etime;
    }
  }
  itsExpr.resize (nrbl);
  itsResExpr.resize (nrbl);
  // Create the histogram object for showing used #cells in time and freq
  itsCelltHist.resize (nrbl);
  itsCellfHist.resize (nrbl);
  makeBLExpr (wsrtModel);
  // Apply possible solvparms and peelnrs.
  setSolvable (vector<string>(), vector<bool>());
  setPeel (vector<int>(), vector<int>());
  initParms();
}

//##ModelId=3EC9F6EC0243
void MeqCalSolver::makeStatExpr (int ant, bool wsrtModel, bool asAP)
{
  // Expression to calculate UVW per station
  if (itsCalcUVW) {
    itsStatUVW[ant] = new MeqStatUVW (itsStations[ant], &itsPhaseRef);
  } else {
    itsStatUVW[ant] = new MeqStatUVW;
  }
  // Expression to calculate contribution per station per source.
  itsStatSrc[ant] = new MeqStatSources (itsStatUVW[ant], &itsSources);
  // Expression representing station parameters.
  MeqExpr* frot = new MeqStoredParmPolc ("frot." +
                                         itsStations[ant]->getName(),
                                         -1, ant+1,
                                         itsMEP);
  itsExprDel.push_back (frot);
  MeqExpr* drot = new MeqStoredParmPolc ("drot." +
                                         itsStations[ant]->getName(),
                                         -1, ant+1,
                                         itsMEP);
  itsExprDel.push_back (drot);
  MeqExpr* dell = new MeqStoredParmPolc ("dell." +
                                         itsStations[ant]->getName(),
                                         -1, ant+1,
                                         itsMEP);
  itsExprDel.push_back (dell);
  MeqExpr* gain11 = new MeqStoredParmPolc ("gain.11." +
                                           itsStations[ant]->getName(),
                                           -1, ant+1,
                                           itsMEP);
  itsExprDel.push_back (gain11);
  MeqExpr* gain22 = new MeqStoredParmPolc ("gain.22." +
                                           itsStations[ant]->getName(),
                                           -1, ant+1,
                                           itsMEP);
  itsExprDel.push_back (gain22);
  itsStatExpr[ant] = new MeqStatExpr (frot, drot, dell, gain11, gain22);
  if (!wsrtModel) {
    // Make a LOFAR expression for all source parameters for this station.
    vector<MeqJonesExpr*> vec;
    string ejname1 = "real.";
    string ejname2 = "imag.";
    if (asAP) {
      ejname1 = "ampl.";
      ejname2 = "phase.";
    }
    for (int j=0; j<itsSources.size(); j++) {
      string nm = itsStations[ant]->getName() + '.' +  itsSources[j].getName();
      MeqExpr* ej11r = new MeqStoredParmPolc ("EJ11." + ejname1 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej11r);
      MeqExpr* ej11i = new MeqStoredParmPolc ("EJ11." + ejname2 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej11i);
      MeqExpr* ej12r = new MeqStoredParmPolc ("EJ12." + ejname1 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej12r);
      MeqExpr* ej12i = new MeqStoredParmPolc ("EJ12." + ejname2 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej12i);
      MeqExpr* ej21r = new MeqStoredParmPolc ("EJ21." + ejname1 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej21r);
      MeqExpr* ej21i = new MeqStoredParmPolc ("EJ21." + ejname2 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej21i);
      MeqExpr* ej22r = new MeqStoredParmPolc ("EJ22." + ejname1 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej22r);
      MeqExpr* ej22i = new MeqStoredParmPolc ("EJ22." + ejname2 + nm,
                                              j+1, ant+1,
                                              itsMEP);
      itsExprDel.push_back (ej22i);
      MeqExpr *ej11, *ej12, *ej21, *ej22;
      if (asAP) {
	ej11 = new MeqExprAPToComplex (ej11r, ej11i);
	ej12 = new MeqExprAPToComplex (ej12r, ej12i);
	ej21 = new MeqExprAPToComplex (ej21r, ej21i);
	ej22 = new MeqExprAPToComplex (ej22r, ej22i);
      } else {
	ej11 = new MeqExprToComplex (ej11r, ej11i);
	ej12 = new MeqExprToComplex (ej12r, ej12i);
	ej21 = new MeqExprToComplex (ej21r, ej21i);
	ej22 = new MeqExprToComplex (ej22r, ej22i);
      }
      itsExprDel.push_back (ej11);
      itsExprDel.push_back (ej12);
      itsExprDel.push_back (ej21);
      itsExprDel.push_back (ej22);
      MeqJonesExpr* mjn = new MeqJonesNode (ej11, ej12, ej21, ej22);
      itsJExprDel.push_back (mjn);
      vec.push_back (mjn);
    }
    itsLSSExpr[ant] = new MeqLofarStatSources (vec, itsStatSrc[ant]);
  }
}

//##ModelId=3EC9F6EC0248
void MeqCalSolver::makeBLExpr (bool wsrtModel)
{
  int blinx = 0;
  for (unsigned int i=0; i<itsBLIndex.size(); i++) {
    int ant2 = itsBLIndex[i];
    if (ant2 >= 0) {
      int ant1 = ant2/16384;
      ant2 -= ant1*16384;
      if (wsrtModel) {
        // Create the DFT kernel.
        MeqPointDFT* dft = new MeqPointDFT (itsStatSrc[ant1],
                                            itsStatSrc[ant2]);
        itsExprDel.push_back (dft);
        itsResExpr[blinx] = new MeqWsrtPoint (&itsSources, dft,
                                              &itsCelltHist[blinx],
                                              &itsCellfHist[blinx]);
      } else {
        itsResExpr[blinx] = new MeqLofarPoint (&itsSources,
                                               itsLSSExpr[ant1],
                                               itsLSSExpr[ant2],
                                               &itsCelltHist[blinx],
                                               &itsCellfHist[blinx]);
      }
      itsExpr[blinx] = new MeqWsrtInt (itsResExpr[blinx],
                                       itsStatExpr[ant1],
                                       itsStatExpr[ant2]);
      itsBLIndex[i] = blinx++;
    }
  }
}

//##ModelId=3EC9F6EC024A
void MeqCalSolver::initParms()
{
  MeqDomain domain(itsStartTime, itsEndTime, itsStartFreq, itsEndFreq);

  const vector<MeqParm*>& parmList = MeqParm::getParmList();

  itsNrScid = 0;
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter) {
      itsNrScid += (*iter)->initDomain (domain, itsNrScid);
    }
  }

  if (itsNrScid > 0) {
    // Get the initial values of all solvable parms.
    // Resize the solution vector if needed.
    if (itsSolution.isNull()  ||  itsSolution.nx() != itsNrScid) {
      itsSolution = MeqMatrix (double(), itsNrScid, 1);
    }
    vector<string> names;
    for (vector<MeqParm*>::const_iterator iter = parmList.begin();
         iter != parmList.end();
         iter++)
    {
      if (*iter  &&  (*iter)->isSolvable()) {
        (*iter)->getInitial (itsSolution);
        names.push_back ((*iter)->getName());
      }
    }
    // Initialize the solver.
    itsSolver.set (itsNrScid, 1, 0);
    // Put names of all solvable parms in status record.
    solution_()[StParamNames] = names;
  }
}

//##ModelId=3EC9F6EC024F
void MeqCalSolver::fillUVW()
{
  cdebug(1) << "get UVW coordinates from data" << endl;
  // Get all times from the vistiles.
  vector<int> tilenr;
  vector<int> tileinx;
  Vector<uInt> inx;
  {
    vector<double> times;
    int seqnr = 0;
    for (vector<VisTile::Ref>::const_iterator iter = itsVisTiles.begin();
         iter != itsVisTiles.end();
         iter++, seqnr++) {
      const VisTile& tile = **iter;
      int inxnr = 0;
      for (VisTile::const_iterator viter = tile.begin();
           viter != tile.end();
           viter++, inxnr++) {
        // First make a domain.
        times.push_back (viter.time());
        tilenr.push_back(seqnr);
        tileinx.push_back(inxnr);
      }
    }
    // Sort them in time order.
    GenSortIndirect<double>::sort (inx, &(times[0]), times.size());
  }
  int nant = itsStatUVW.size();
  vector<bool> statDone (nant);
  vector<double> statuvw(3*nant);
  // Step through the uvw's in time order.
  double lastTime = -1;
  for (uInt i=0; i<tilenr.size(); i++) {
    const VisTile& tile = *(itsVisTiles[tilenr[i]]);
    int inxnr = tileinx[i];
    int a1 = tile.antenna1();
    int a2 = tile.antenna2();
    double time = tile.time()(inxnr);
    const LoMat_double& uvw = tile.uvw();
    if (time != lastTime) {
      lastTime = time;
      // Set UVW of first station to 0 (UVW coordinates are relative!).
      statDone.assign (statDone.size(), false);
      statuvw[3*a1]   = 0;
      statuvw[3*a1+1] = 0;
      statuvw[3*a1+2] = 0;
      statDone[a1] = true;
      itsStatUVW[a1]->set (time, 0, 0, 0);
    }
    if (!statDone[a2]) {
      AssertMsg (statDone[a1], "fillUVW: ordering problems");
      if (statDone[a1]) {
        statuvw[3*a2]   = uvw(0, inxnr) - statuvw[3*a1];
        statuvw[3*a2+1] = uvw(1, inxnr) - statuvw[3*a1+1];
        statuvw[3*a2+2] = uvw(2, inxnr) - statuvw[3*a1+2];
        statDone[a2] = true;
        itsStatUVW[a2]->set (time, statuvw[3*a2], statuvw[3*a2+1],
                             statuvw[3*a2+2]);
      }
    } else if (!statDone[a1]) {
      AssertMsg (statDone[a2], "fillUVW: ordering problems");
      if (statDone[a2]) {
        statuvw[3*a1]   = statuvw[3*a2]   - uvw(0, inxnr);
        statuvw[3*a1+1] = statuvw[3*a2+1] - uvw(1, inxnr);
        statuvw[3*a1+2] = statuvw[3*a2+2] - uvw(2, inxnr);
        statDone[a1] = true;
        itsStatUVW[a1]->set (time, statuvw[3*a1], statuvw[3*a1+1],
                             statuvw[3*a1+2]);
      }
    }
  }
}

//##ModelId=3EC9F6EC0260
void MeqCalSolver::saveParms()
{
  const vector<MeqParm*>& parmList = MeqParm::getParmList();
  cdebug(1) << "saveParms" << endl;
  Assert (!itsSolution.isNull()  &&  itsSolution.nx() == itsNrScid);
  // Save the values of all solvable paramaters.
  for (vector<MeqParm*>::const_iterator iter = parmList.begin();
       iter != parmList.end();
       iter++)
  {
    if (*iter  &&  (*iter)->isSolvable()) {
      (*iter)->save();
    }
  }
}

//##ModelId=3EC9F6EC0262
void MeqCalSolver::saveResiduals (DataRecord::Ref& header,bool apply_peel)
{
  cdebug(1) << "saveResidualData" << endl;

  // generate new vdsid for the data set
  ///  HIID &vdsid = header()[FVDSID];
    ///vdsid[2] = ++dataset_seq_num_;
  // put relevant info into header
  header()[FDataType] = DataType_Final;
  header()[FSolveIterNum] = control().iterationNum();
  
  // write the header
  output().put(HEADER,header.copy());
  
  cdebug(2) << "Using peel sources ";
  for (unsigned int i=0; i<itsPeelSourceNrs.size(); i++) {
    cdebug(2) << itsPeelSourceNrs[i] << ',';
  }
  cdebug(2) << endl;
  itsSources.setSelected (itsPeelSourceNrs);

  Timer timer;
  for (unsigned int i=0; i<itsVisTiles.size(); i++) 
  {
    // Privatize the tile for writing (this is a no-op if we have the only
    // copy). This is good practice to minimize data copying between
    // threads: we hold a writable ref to the tile only as long as we're 
    // really writing to it.
    itsVisTiles[i].privatize(DMI::WRITE|DMI::DEEP);
    VisTile& visTile = itsVisTiles[i];
    // apply format if we don't have a residuals column in this tile
    if( !visTile.format().defined(VisTile::RESIDUALS) )
      visTile.changeFormat(tileformat_);
      
    cdebug(3) << "Tile: "<<visTile.sdebug(5)<<endl;
    // Do a predict for the elements in this range.
    int ant1 = visTile.antenna1();
    int ant2 = visTile.antenna2();
    int blnr = ifrNumber(ant1,ant2);
    Assert (blnr < int(itsBLIndex.size()));
    int blindex = itsBLIndex[blnr];
    Assert (blnr >= 0);
    for (VisTile::iterator iter = visTile.begin();
         iter != visTile.end();
         iter++) 
    {
      // First make a domain.
      double time = iter.time();
      double interval = iter.interval();
      MeqDomain domain(time - interval/2, time + interval/2,
                       itsStartFreq, itsEndFreq);
      MeqRequest request(domain, 1, itsNrChan);
      MeqJonesExpr& expr = *(itsExpr[blindex]);
      expr.calcResult (request);
      // Get the data and resdata of this row for the given channels.
      LoMat_fcomplex data (iter.data());
      LoMat_fcomplex resdata (iter.residuals());
      int npol = data.shape()[0];
      int nchan = data.shape()[1];
      // Make Blitz vectors of the predicted data.
      LoVec_dcomplex xx (const_cast<complex<double>*>
                         (expr.getResult11().getValue().dcomplexStorage()),
                         blitz::shape(nchan),
                         blitz::neverDeleteData);
      LoVec_dcomplex xy (const_cast<complex<double>*>
                         (expr.getResult12().getValue().dcomplexStorage()),
                         blitz::shape(nchan),
                         blitz::neverDeleteData);
      LoVec_dcomplex yx (const_cast<complex<double>*>
                         (expr.getResult21().getValue().dcomplexStorage()),
                         blitz::shape(nchan),
                         blitz::neverDeleteData);
      LoVec_dcomplex yy (const_cast<complex<double>*>
                         (expr.getResult22().getValue().dcomplexStorage()),
                         blitz::shape(nchan),
                         blitz::neverDeleteData);
      // Subtract the predicted data from the measured data.
      resdata = data;
      if (1 == npol) {
        resdata(0,blitz::Range::all()) -= xx;
      } else if (2 == npol) {
        resdata(0,blitz::Range::all()) -= xx;
        resdata(1,blitz::Range::all()) -= yy;
      } else if (4 == npol) {
        resdata(0,blitz::Range::all()) -= xx;
        resdata(1,blitz::Range::all()) -= xy;
        resdata(2,blitz::Range::all()) -= yx;
        resdata(3,blitz::Range::all()) -= yy;
      } else {
        AssertMsg(false, "Number of polarizations should be 1, 2, or 4");
      }
      if( apply_peel )
        data = resdata;
    }
    // change the tile back to read-only
    itsVisTiles[i].privatize(DMI::READONLY|DMI::DEEP);
    // dump a copy to the output stream
    output().put(DATA,itsVisTiles[i].copy());
  }
  // write footer to output stream
  DataRecord::Ref footer(DMI::ANONWR);
  ///  footer()[FVDSID] = vdsid;
  footer()[StSolution] <<= solution_.copy();
  output().put(FOOTER,footer);
}
