#include "DummySolver.h"

#include <unistd.h>
    
using namespace AppState;
using namespace SolverVocabulary;
    
//##ModelId=3E6F603A01D6
void DummySolver::addTileToDomain (VisTile::Ref &tileref)
{
  const VisTile &tile = *tileref;
  // first tile of new domain
  if( domain_start == 0 )
  {
    control().startDomain();
    domain_start = tile.time(0);
    domain_end = domain_start + domain_size;
    cdebug(2)<<"starting new domain with Tstart="<<domain_start<<endl;
    in_domain = True;
    ntiles = 1;
  }
  // does tile belong to next domain?
  else
  {
    if( tile.time(0) >= domain_end )
    {
      cdebug(2)<<"tile time past end of domain, closing domain"<<endl;
      endDomain();
      return;
    }
  }
  // assume the domain size is a multiple of the tile size (in time),
  // so at this point the tile fully belongs to our domain
  // Detach the tileref; in real life we would actually xfer it to some
  // internal container 
  tileref.detach();
  ntiles++;
 cdebug(2)<<"adding tile to domain"<<endl;
}

//##ModelId=3E6F603A0256
void DummySolver::checkInputState (int instat)
{
  // if input stream is ERROR or CLOSED, say endOfData() to the 
  // control agent, and end the current domain
  // error on the input stream? terminate the transaction
  if( instat == AppEvent::ERROR )
  {
    cdebug(2)<<"error on input: "<<input().stateString()<<endl;
    control().endData(InputErrorEvent);
    endDomain();
  }
  // closed the input stream? terminate the transaction
  else if( instat == AppEvent::CLOSED )
  {
    cdebug(2)<<"input closed: "<<input().stateString()<<endl;
    control().endData(InputClosedEvent);
    endDomain();
  }
}

//##ModelId=3E6F603A02D1
void DummySolver::endDomain ()
{
  if( in_domain )
  {
    control().endDomain();
    domain_start = 0;
    in_domain = False;
  }
}

//##ModelId=3E6F603A030B
void DummySolver::endSolution (const DataRecord &endrec)
{
  cdebug(2)<< "end solution: "<<endrec.sdebug(3)<<endl;
}


//##ModelId=3E00ACED00BB
void DummySolver::run ()
{
  using namespace SolverControl;
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
        // default is 1 min domains
    domain_size = (*initrec)[FDomainSize].as<double>(60);
    cdebug(1)<< "starting run()\n";
    // run main loop
    DataRecord::Ref header;
    VisTile::Ref tile;
    while( control().checkState() > 0 )  // while in a running state
    {
      int instat;
      // -------------- receive data set header
      // check for a cached header (once a new header is received, it stays 
      // in cache until all solutions on the previous data set have been 
      // finished) and/or wait for a header to arrive
      while( !header.valid() && control().checkState() > 0 )
      {
        HIID id;
        ObjRef ref;
        instat = input().getNext(id,ref,0,AppEvent::WAIT);
        if( instat > 0 )
        {
          if( instat == HEADER )
          {
            // got header? break out
            header = ref.ref_cast<DataRecord>();
            // setup whatever's needed from the header
            cdebug(1)<<"got header: "<<header->sdebug(2)<<endl;
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
      // we have a valid header now
      // ought to post the header to the output later, because the solver
      // may want to update something in it.
      // DummySolver doesn't post anything to the output.
      
      // --------- read full domain from input ---------------------------
      in_domain = True;
      domain_start = 0;
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
              endDomain();
            }
            else if( instat == HEADER )
            {
              header = ref.ref_cast<DataRecord>();
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
          addTileToDomain(tile);
      }
      if( control().checkState() <= 0 )
        continue;
      // ought to check that we've actually got a domain of data (we could
      // have received two headers in a row, for example). If no data,
      // do a continue to top of loop. But we won't bother for now.
      
      // We have a full domain of data now. Start the solution loop
      // control state ought to be IDLE at start of every solution. (Otherwise,
      // it's either NEXT_DOMAIN, or a terminal state)
      while( control().state() == IDLE )
      {
        DataRecord::Ref params,endrec,solution(DMI::ANONWR);
        // get solution parameters, break out if none
        if( control().startSolution(params) != RUNNING )
          break;
        cdebug(2)<< "startSolution: "<<params->sdebug(3)<<endl;
        int niter = 0;
        double converge = 1000;
        // iterate the solution until stopped
        do
        {
          solution()[FConvergence] = converge /= 10;
          sleep(1); // Jesus is coming... Look busy!
          niter++;
        }
        while( control().endIteration(solution) == AppState::RUNNING );
        
        // if state is ENDSOLVE, end the solution properly
        if( control().state() == ENDSOLVE )
        {
          cdebug(2)<<"ENDSOLVE after "<<niter<<" iterations, converge="<<converge<<endl;
          int res = control().endSolution(solution,endrec);
          endSolution(*endrec);
        }
        // else we were probably interrupted
        else
        {
          cdebug(2)<<"stopped with state "<<control().stateString()<<" after "<<niter<<" iterations, converge="<<converge<<endl;
        }
      }
      // end of solving in this domain, for whatever reason.
      // output data and/or whatever
      
      // go back to top of loop and check for state
    }
    // broke out of main loop -- close i/o agents
    input().close();
    output().close();
  }
  cdebug(1)<<"exiting with control state "<<control().stateString()<<endl;
  control().close();
}

//##ModelId=3E00B22801E4
//string DummySolver::sdebug(int detail, const string &prefix, const char *name) const
//{
//}

