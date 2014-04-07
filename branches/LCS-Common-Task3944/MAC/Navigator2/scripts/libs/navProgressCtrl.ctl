// navTabCtrl.ctl
//
//  Copyright (C) 2002-2015
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
///////////////////////////////////////////////////////////////////
// functions to control the Progressbar of the navigator
///////////////////////////////////////////////////////////////////
//
// Functions and procedures
//
// navProgressCtrl_handleObservation         : determine the length of an observation and how long still is needed, set progressbar value based on this calculation


///////////////////////////////////////////////////////////////////////////
//
// Function navProgressCtrl_handleObservation
//
// determine the length of an observation and how long still is needed, 
// set progressbar value based on this calculation
//
///////////////////////////////////////////////////////////////////////////

#uses "claimManager.ctl"
#uses "navFunct.ctl"

void navProgressCtrl_handleObservation(string selection){
  LOG_TRACE("navProgressCtrl.ctl:navProgressCtrl_handleObservation|entered with: "+selection);

  string toolText=selection;
  
  int startpos =strpos(selection,"Observation");

  if ( startpos < 0) return;
  string obsName = "LOFAR_ObsSW_"+substr(selection,startpos);
  LOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation|found obsName: "+obsName);
  
  string obsDP=MainDBName+claimManager_nameToRealName(obsName);
  LOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation|found obsDP: "+obsDP);
  
  // find  starttime and endtime and current time and calculate %done
  
  time start,stop;
  dpGet(obsDP+".startTime",start);
  dpGet(obsDP+".stopTime",stop);
  LOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation| start: "+start);  
  LOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation| stop:" +stop);  
  
  int duration=period(stop) - period(start);

  LOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation| Duration: "+duration);
  
  float percent=duration/100;
  LOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation| percent: "+percent);  

  int finished=period(getCurrentTime())-period(start);
  LOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation| finished: "+finished);  
  
  float percentDone=0;
  if (percent > 0) {
    percentDone = finished/percent;
  }
  DLOG_DEBUG("navProgressCtrl.ctl:navProgressCtrl_handleObservation| PercentDone: "+percentDone);
  // change progressBar
  dpSet(PROGRESSBARACTIONDP,"Update|"+percentDone+"|"+toolText);
  
}
