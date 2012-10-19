//#e   ReadCmd.cc: implementation of the ReadCmd class
//#
//#  Copyright (C) 2002-2004
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

#include <math.h>
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>

#include "ReadCmd.h"
#include "UdpIpTools.h"


using namespace LOFAR;
using namespace GCF::TM;
using namespace TBB_Protocol;
using namespace TP_Protocol;
using namespace TBB;


//--Constructors for a ReadCmd object.----------------------------------------
ReadCmd::ReadCmd():
	itsSecondstime(0), itsSampleNr(0), itsPrepages(0), itsPostpages(0), itsStage(0),
	itsLastSavedSecond(0), itsLastSavedSampleNr(0), 
	itsLastSavedNrOfSamples(0), itsLastSavedSampleFreq(0),
	itsTimestamp(0), itsTimeBefore(0), itsTimeAfter(0)
{
	TS = TbbSettings::instance();
	setWaitAck(true);
}

//--Destructor for ReadCmd.---------------------------------------------------
ReadCmd::~ReadCmd()
{
	//if (itsTimestamp) { delete itsTimestamp; }
}

// ----------------------------------------------------------------------------
bool ReadCmd::isValid(GCFEvent& event)
{
	if ((event.signal == TBB_READ)
		||(event.signal == TP_UDP_ACK)
		||(event.signal == TP_READ_ACK)
		||(event.signal == TP_READR_ACK)
		||(event.signal == TP_ARP_ACK)) {
		return(true);
	}
	return(false);
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTbbEvent(GCFEvent& event)
{
	TBBReadEvent tbb_event(event);

	setChannel(tbb_event.rcu);

//	itsSecondstime = tbb_event.secondstime;
//	itsSampleNr    = tbb_event.samplenr;
//	itsPrepages    = tbb_event.prepages;
//	itsPostpages   = tbb_event.postpages;
	itsTimestamp   = tbb_event.nstimestamp;
	itsTimeBefore  = tbb_event.nstimebefore;
	itsTimeAfter   = tbb_event.nstimeafter;
	nextChannelNr();

	if (!isDone()) {
		// check if channel is stopped
		if (TS->getChState(getChannelNr()) != 'S') {
            LOG_WARN_STR(formatString("selected rcu(%d) NOT stopped", tbb_event.rcu));
			setStatus(0, TBB_CH_NOT_STOPPED);
			setDone(true);
		}
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTpEvent()
{
	switch (itsStage) {
		
		// stage 0, haal de sampletijd op van het laatst geschreven frame
		// stage 1, haal de sample frequentie op van het laatst gescreven frame
		//          en kijk of gevraagde data aanwezig is.
		// stage 2, zet udp poort gegevens
		// stage 3, verstuur 1 arp bericht
		// stage 4, stuur het read commando
		
		// get last saved sampletime and samplenr
		case 0: {
			TPReadrEvent tp_event;
			tp_event.opcode = oc_READR;
			tp_event.status = 0;
			tp_event.mp     = (uint32)TS->getChMpNr(getChannelNr());
			tp_event.pid    = (uint32)TS->getChMemWriter(getChannelNr());
			tp_event.regid  = 4;
				
			TS->boardPort(getBoardNr()).send(tp_event);
			TS->setBoardUsed(getBoardNr());
			TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		} break;
		
		// get last saved samplefreq
		case 1: {
			TPReadrEvent tp_event;
			tp_event.opcode = oc_READR;
			tp_event.status = 0;
			tp_event.mp     = TS->getChMpNr(getChannelNr());
			tp_event.pid    = TS->getChMemWriter(getChannelNr());
			tp_event.regid  = 6;
				
			TS->boardPort(getBoardNr()).send(tp_event);
			TS->setBoardUsed(getBoardNr());
			TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		} break;
		
		// send UDP header settings for CEP port
		case 2: {
			TPUdpEvent tp_event;
			tp_event.opcode = oc_UDP;
			tp_event.status = 0;
			uint32 mode = TS->getChOperatingMode(getChannelNr());
			LOG_DEBUG_STR(formatString("selected mode = %u", mode));
			// fill in destination mac address
			string2mac(TS->getSrcMacCep(getBoardNr()).c_str(), tp_event.srcmac);
			string2mac(TS->getDstMacCep(getChannelNr()).c_str(), tp_event.dstmac);
			// fill in udp-ip header
			setup_udpip_header(  getBoardNr(),
								      mode,
								      TS->getSrcIpCep(getBoardNr()).c_str(),
								      TS->getDstIpCep(getChannelNr()).c_str(),
								      tp_event.ip,
								      tp_event.udp );

			TS->boardPort(getBoardNr()).send(tp_event);
			TS->setBoardUsed(getBoardNr());
			TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		} break;
		
		// send 1 arp message
		case 3: {
			TPArpEvent tp_event;
			
			tp_event.opcode = oc_ARP;
			tp_event.status = 0;
			
			TS->boardPort(getBoardNr()).send(tp_event);
			TS->setBoardUsed(getBoardNr());
			TS->boardPort(getBoardNr()).setTimer(TS->timeout());
		} break;
		
		// send requested data to CEP
		case 4: {
			TPReadEvent tp_event;

			tp_event.opcode      = oc_READ;
			tp_event.status      = 0;
			tp_event.channel     = TS->getChInputNr(getChannelNr());
			tp_event.secondstime = itsSecondstime;
			tp_event.sampletime  = itsSampleNr;
			tp_event.prepages    = itsPrepages;
			tp_event.postpages   = itsPostpages;
			TS->boardPort(getBoardNr()).send(tp_event);
			TS->setBoardUsed(getBoardNr());
			TS->boardPort(getBoardNr()).setTimer(TS->timeout());
			LOG_INFO_STR(formatString("DATA->>CEP rcu %d : time=%lf  timebefore=%lf  timeafter=%lf",
				                       TS->convertChanToRcu(getChannelNr()),
				                       (double)itsTimestamp,
				                       (double)itsTimeBefore,
				                       (double)itsTimeAfter));
		} break;
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::saveTpAckEvent(GCFEvent& event)
{

	// in case of a time-out, set error mask
	if (event.signal == F_TIMER) {
		setStatus(getBoardNr(), TBB_TIME_OUT);
	}
	else if (event.signal == TP_READR_ACK) {
		TPReadrAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received ReadrAck from boardnr[%d]", getBoardNr()));
		if (tp_ack.status != 0) {
			setStatus(0, (tp_ack.status << 24));
		} else {
			if (itsStage == 0) {
				// get time information of last saved frame
				itsLastSavedSecond = tp_ack.data[0];
				itsLastSavedSampleNr = tp_ack.data[1];
				itsStage = 1;
			}
			else {
				// get number of samples and sample frequency of last saved frame
				itsLastSavedNrOfSamples = tp_ack.data[0];
				itsLastSavedSampleFreq = tp_ack.data[1];
				//LOG_DEBUG_STR(formatString("Last frame info: time=%lu  samplenr=%lu  nSamples=%lu  freq=%lu",
				//                           itsLastSavedSecond, itsLastSavedSampleNr, itsLastSavedNrOfSamples, itsLastSavedSampleFreq ));
								
				double sampletime = 1. / (itsLastSavedSampleFreq * 1E6); // in sec
				 
				// calculate time of last sample in memory
				RTC::NsTimestamp 
				lastSampleTime(itsLastSavedSecond + ((itsLastSavedSampleNr + itsLastSavedNrOfSamples) * sampletime));
				
				// calculate time of first sample in memory
				RTC::NsTimestamp 
				firstSampleTime((double)lastSampleTime - (TS->getChPageSize(getChannelNr()) * itsLastSavedNrOfSamples * sampletime));
				
				// calculate start and stop time
				RTC::NsTimestamp startTimestamp = itsTimestamp - itsTimeBefore;
				RTC::NsTimestamp stopTimestamp = itsTimestamp + itsTimeAfter;
				
				#if 1
			//LOG_DEBUG_STR(formatString("Timestamp      =  %lu seconds  %lu nseconds", itsTimestamp.sec(), itsTimestamp.nsec()));
			LOG_DEBUG_STR(formatString("firstSampleTime=  %lu seconds  %lu nseconds", firstSampleTime.sec(), firstSampleTime.nsec()));
			LOG_DEBUG_STR(formatString("lastSampleTime =  %lu seconds  %lu nseconds", lastSampleTime.sec(), lastSampleTime.nsec()));
			LOG_DEBUG_STR(formatString("startTimestamp =  %lu seconds  %lu nseconds", startTimestamp.sec(), startTimestamp.nsec()));
			LOG_DEBUG_STR(formatString("stopTimestamp  =  %lu seconds  %lu nseconds", stopTimestamp.sec(), stopTimestamp.nsec()));
				#endif
                
                // to get last part of recording				
				if (itsTimestamp == 0.0) {
				    itsTimestamp = lastSampleTime;
				}
				
				// check if center time in memory
				if ((itsTimestamp >= firstSampleTime) && (itsTimestamp <= lastSampleTime)) {
					// check if start time in memory, if not correct it
					if (startTimestamp < firstSampleTime) {
						LOG_WARN_STR(formatString("Not all requested prepages are available for rcu[%d]", TS->convertChanToRcu(getChannelNr())));
                        LOG_WARN_STR(formatString("requested starttime = %lf, first time in memory = %lf", (double)startTimestamp, (double)firstSampleTime)); 
                        
						startTimestamp = firstSampleTime;
					}
					// check if stop time in memory, if not correct it
					if (stopTimestamp > lastSampleTime) {
						LOG_WARN_STR(formatString("Not all requested postpages are available for rcu[%d]", TS->convertChanToRcu(getChannelNr())));
                        LOG_WARN_STR(formatString("requested stoptime = %lf, last time in memory = %lf", (double)stopTimestamp, (double)lastSampleTime)); 
						stopTimestamp = lastSampleTime;
					}
				}
				else {
					// requested time not in memory
					LOG_WARN_STR(formatString("Requested time(data) not in memory for rcu[%d]", TS->convertChanToRcu(getChannelNr())));
                    LOG_WARN_STR(formatString("requested time = %lf, in memory [%lf .. %lf]", (double)itsTimestamp, (double)firstSampleTime, (double)lastSampleTime)); 
					setDone(true);
				} 
                
                itsTimeBefore = itsTimestamp - startTimestamp;
                itsTimeAfter =  stopTimestamp - itsTimestamp;
                
				// convert it to board units
				
				itsSecondstime = (uint32)itsTimestamp.sec();
				itsSampleNr    = (uint32)(itsTimestamp.nsec() / (sampletime * 1E9));
				itsPrepages    = (uint32)ceil(((double)(itsTimestamp - startTimestamp) / sampletime) / itsLastSavedNrOfSamples);
				itsPostpages   = (uint32)ceil(((double)(stopTimestamp - itsTimestamp) / sampletime) / itsLastSavedNrOfSamples);
				//if (itsPostpages == 0) { itsPostpages = 1; }
				LOG_DEBUG_STR(formatString("Read request time=%lu samplenr=%lu prepages=%lu postpages=%lu",
				                           itsSecondstime, itsSampleNr, itsPrepages, itsPostpages));
				itsStage = 2;
			}
		}
	}
	else if (event.signal == TP_UDP_ACK) {
		TPUdpAckEvent tp_ack(event);

		if (tp_ack.status != 0) {
			setStatus(0, (tp_ack.status << 24));
		}
		LOG_DEBUG_STR(formatString("Received UdpAck from boardnr[%d]", getBoardNr()));
		itsStage = 3;
	}
	else if (event.signal == TP_ARP_ACK) {
		TPArpAckEvent tp_ack(event);
		
		if (tp_ack.status != 0) {
			setStatus(0, (tp_ack.status << 24));
		}
		LOG_DEBUG_STR(formatString("Received ArpAck from boardnr[%d]", getBoardNr()));
		itsStage = 4;
	}
	else if (event.signal == TP_READ_ACK) {
		TPReadAckEvent tp_ack(event);
		LOG_DEBUG_STR(formatString("Received ReadAck from boardnr[%d]", getBoardNr()));
		LOG_DEBUG_STR(formatString("ReadAck.status=%d", tp_ack.status));
		/*
		LOG_INFO_STR("ReadCmd Info: page-index     :" << tp_ack.page_index);
		LOG_INFO_STR("              pages-left     :" << tp_ack.pages_left);
		LOG_INFO_STR("              period-samples :" << tp_ack.period_samples);
		LOG_INFO_STR("              period seconds :" << tp_ack.period_seconds);
		LOG_INFO_STR("              page-offset    :" << tp_ack.page_offset);
		*/
		if (tp_ack.status == 0xfd) {
			LOG_INFO_STR(formatString("TBB busy, %d pages left", tp_ack.pages_left));
			usleep(1000); // wait for some time and try again
		}
		else { 
		    if (tp_ack.status != 0) {
			    setStatus(0, (tp_ack.status << 24));
			}
			setDone(true);
		}
	}
}

// ----------------------------------------------------------------------------
void ReadCmd::sendTbbAckEvent(GCFPortInterface* clientport)
{
	TBBReadAckEvent tbb_ack;

	tbb_ack.status_mask = getStatus(0);

	if (clientport->isConnected()) {clientport->send(tbb_ack); }
}
