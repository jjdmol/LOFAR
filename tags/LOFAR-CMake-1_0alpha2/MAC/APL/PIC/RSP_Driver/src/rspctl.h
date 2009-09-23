//#  -*- mode: c++ -*-
//#
//#  rspctl.h: command line interface to the RSPDriver
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

#ifndef RSPCTL_H_
#define RSPCTL_H_

#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/RCUSettings.h>
#include <APL/RSP_Protocol/HBASettings.h>
#include <GCF/TM/GCF_Control.h>

#include <APL/RTCCommon/Timestamp.h>

#include <complex>
#include <blitz/array.h>

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPort;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFPortInterface;
  namespace rspctl {
		
//
// class Command :base class for control commands towards the RSPDriver.
//
class Command {
public:
	virtual ~Command() {}

	// Send the command to the RSPDriver
	virtual void send() = 0;

	// rspctl is stopped. perform cleanup code here
	virtual void stop() {}

	// Check the acknowledgement sent by the RSPDriver.
	virtual GCFEvent::TResult ack(GCFEvent& e) = 0;

	// Set selection.
	void setSelect(std::list<int> select) {
		m_select = select;
	}

	// Set beamlets.
	void setBeamlets(std::list<int> beamlets) {
		m_beamlets = beamlets;
	}

	// Get the mask (MAX_N_RCUS bits).
	bitset<MEPHeader::MAX_N_RCUS> getRCUMask() const {
		bitset<MEPHeader::MAX_N_RCUS> mask;

		mask.reset();
		std::list<int>::const_iterator it;
		int count = 0; // limit to ndevices
		for (it = m_select.begin(); it != m_select.end(); ++it, ++count) {
			if (count >= get_ndevices())
				break;
			if (*it < MEPHeader::MAX_N_RCUS)
				mask.set(*it);
		}
		return mask;
	}

	// Get the mask (MAX_N_RSPBOARDS bits).
	bitset<MAX_N_RSPBOARDS> getRSPMask() const {
		bitset<MAX_N_RSPBOARDS> mask;

		mask.reset();
		std::list<int>::const_iterator it;
		int count = 0; // limit to ndevices
		for (it = m_select.begin(); it != m_select.end(); ++it, ++count) {
			if (count >= get_ndevices())
				break;
			if (*it < MAX_N_RSPBOARDS)
				mask.set(*it);
		}
		return mask;
	}

	// Get the mask (N_BEAMLETS bits).
	bitset<MEPHeader::N_BEAMLETS> getBEAMLETSMask() const {
		bitset<MEPHeader::N_BEAMLETS> mask;

		mask.reset();
		std::list<int>::const_iterator it;
		for (it = m_beamlets.begin(); it != m_beamlets.end(); ++it) {
			if (*it < MEPHeader::N_BEAMLETS)
				mask.set(*it);
		}
		return mask;
	}

	// Distill two rectdomains from the selection list
	bool getRSPRange2(blitz::Range& r1, blitz::Range& r2, int n_blps = MEPHeader::N_BLPS) const {
		blitz::TinyVector<int, 2> lowerbounds(0,0), upperbounds(0,0);
		std::list<int> select = m_select;

		if (select.size() != 4) 
			return false;

		int lb = select.front() * n_blps;
		select.pop_front();
		int ub = select.front() * n_blps;
		select.pop_front();
		r1 = blitz::Range(lb, ub - 1);

		lb = select.front() * n_blps;
		select.pop_front();
		ub = select.front() * n_blps;
		select.pop_front();
		r2 = blitz::Range(lb, ub - 1);

		return true;
	}

	// Set mode (true == get, false = set)
	void setMode(bool get) {
		m_get = get;
	}

	// Get mode
	bool getMode() const {
		return m_get;
	}

	// Set ndevices.
	void set_ndevices(int ndevices) {
		m_ndevices = ndevices;
	}

	// Get ndevices.
	int get_ndevices() const {
		return m_ndevices;
	}

	virtual void logMessage(ostream& stream, const string& message) {
		stream << message << endl;
	}

protected:
	explicit Command(GCFPortInterface& port) : 
		m_rspport(port),
		m_select(),
		m_get(true), 
		m_ndevices(0)
		{}
	Command(); // no default construction allowed

	GCFPortInterface& 	m_rspport;

private:
	std::list<int> 		m_select;
	std::list<int>		m_beamlets;	
	bool           		m_get; // get or set
	int            		m_ndevices;
};

//
// class WeightsCommand
//
class WeightsCommand : public Command
{
public:
	enum {
		COMPLEX = 1,
		ANGLE,
	};
	WeightsCommand(GCFPortInterface& port);
	virtual ~WeightsCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setValue(std::complex<double> value) {
		m_value = value;
	}
	void setType(int type) { m_type = type; }
private:
	std::complex<double> 					m_value;
	int                  					m_type;
	int									 	itsStage;
	blitz::Array<std::complex<int16>, 3>	itsWeights;
};

//
// class SubbandsCommand
//
class SubbandsCommand : public Command
{
public:
	SubbandsCommand(GCFPortInterface& port);
	virtual ~SubbandsCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setSubbandList(std::list<int> subbandlist) {
		m_subbandlist = subbandlist;
	}
	void setType(int type) {
		m_type = type;
	}
private:
	std::list<int> 		m_subbandlist;
	int 				m_type;
};

//
// class RCUCommand
//
class RCUCommand : public Command
{
public:
	RCUCommand(GCFPortInterface& port);
	virtual ~RCUCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);

	RCUSettings::Control& control() {
		// return reference so we can modify it
		// using the methods of RCUSettings::Control
		return m_control;
	}

private:
	RCUSettings::Control 	m_control;
};

//
// class HBACommand
//
class HBACommand : public Command
{
public:
	HBACommand(GCFPortInterface& port);
	virtual ~HBACommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);

	void setDelayList(std::list<int> delaylist) {
		m_delaylist = delaylist;
	}

private:
	std::list<int> 		m_delaylist;
};

//
// class RSUCommand
//
class RSUCommand : public Command
{
public:
	RSUCommand(GCFPortInterface& port);
	virtual ~RSUCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);

	RSUSettings::ResetControl& control() {
		// return reference so we can modify it
		// using the methods of RCUSettings::Control
		return m_control;
	}

private:
	RSUSettings::ResetControl 	m_control;
};

//
// class WGCommand
//
class WGCommand : public Command
{
public:
	static double AMPLITUDE_SCALE;

	WGCommand(GCFPortInterface& port);
	virtual ~WGCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);

	// set frequency in range 0 <= frequency < sample_frequency / 2.0
//	void setFrequency(double frequency, double samplefreq) {
//		m_frequency = (uint32)round(frequency * ((uint64)1 << 32) / samplefreq);
	void setFrequency(double frequency) {
		itsFrequency = frequency;
	}

	void setWaveMode(int mode) {
		m_mode = mode;
	}

	// set phase in range 0 <= phase < 360
	void setPhase(uint8 phase) {
#if 0
		while (phase < 0.0)    phase += 360.0;
		while (phase >= 360.0) phase -= 360.0;

		m_phase = (uint8)round((1 << 8) * (phase / 360.0));
#else
		m_phase = phase;
#endif
	}

	// set amplitude in range 0.0 <= amplitude < 2.0
	void setAmplitude(double amplitude) {
		if (amplitude >=  2.0) 
			amplitude = 2.0;
		if (amplitude < 0.0) 
			amplitude = 0.0;
		m_amplitude = (uint32)(amplitude * AMPLITUDE_SCALE);
	}

private:
	uint8  		m_mode;
	uint8  		m_phase;
//	uint32 		m_frequency;
	double 		itsFrequency;
	uint32 		m_amplitude;
};

//
// class StatisticsBaseCommand
//
class StatisticsBaseCommand	: public Command
{
public:
	StatisticsBaseCommand(GCFPortInterface& port);
	virtual ~StatisticsBaseCommand() {
		if(m_file) {
			delete[] m_file;
			m_file=0;
		}
	}
	void setDuration(uint32 duration) {
		m_duration=duration;
		m_endTime.setNow((double)m_duration);
	}
	void setIntegration(int32 integration) {
		m_integration=integration;
	}
	void setDirectory(const char* dir) {
		m_directory = dir;
		if(dir[strlen(dir)-1] != '/') {
			m_directory += "/";
		}
	}
	FILE* getFile(int rcu, char* fileName) {
		if(!m_file) {
			m_file = new FILE*[get_ndevices()];
			if(!m_file) {
				logMessage(cerr,"Error: failed to allocate memory for file handles.");
				exit(EXIT_FAILURE);
			}
			memset(m_file,0,sizeof(FILE*)*get_ndevices());
		}
		if(!m_file[rcu]) {
			m_file[rcu] = fopen(fileName, "w+");
		}
		if(!m_file[rcu]) {
			logMessage(cerr,formatString("Error: Failed to open file: %s",fileName));
			exit(EXIT_FAILURE);
		}
		return m_file[rcu];
	}
protected:
	memptr_t		m_subscriptionhandle;
	uint32 			m_duration;
	RTC::Timestamp 	m_endTime;
	int32  			m_integration;
	uint32 			m_nseconds;
	string 			m_directory;
	FILE** 			m_file; // array of file descriptors, one for each rcu
};

//
// class Statistics
//
class StatisticsCommand : public StatisticsBaseCommand
{
public:
	StatisticsCommand(GCFPortInterface& port);
	virtual ~StatisticsCommand() {}
	virtual void send();
	virtual void stop();
	void capture_statistics(blitz::Array<double, 2>& stats, const RTC::Timestamp& timestamp);
	void plot_statistics(blitz::Array<double, 2>& stats, const RTC::Timestamp& timestamp);
	void dump_statistics(blitz::Array<double, 2>& stats, const RTC::Timestamp& timestamp);
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setType(uint8 type) {
		m_type = type;
	}

private:
	uint8 					m_type;
	blitz::Array<double, 2> m_stats;
};

//
// class XCStatisticsCommand
//
class XCStatisticsCommand : public StatisticsBaseCommand
{
public:
	XCStatisticsCommand(GCFPortInterface& port);
	virtual ~XCStatisticsCommand() {}
	virtual void send();
	virtual void stop();
	void capture_xcstatistics(blitz::Array<std::complex<double>, 4>& stats, const RTC::Timestamp& timestamp);
	void plot_xcstatistics(blitz::Array<std::complex<double>, 4>& stats, const RTC::Timestamp& timestamp);
	void dump_xcstatistics(blitz::Array<std::complex<double>, 4>& stats, const RTC::Timestamp& timestamp);
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setAngle(bool value) { m_xcangle = value; }

private:
	blitz::Array<std::complex<double>, 4> 	m_stats;
	bool 									m_xcangle;
};

//
// class StatusCommand
//
class StatusCommand : public Command
{
public:
	StatusCommand(GCFPortInterface& port);
	virtual ~StatusCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	blitz::Array<SystemStatus, 1>& board() {	// just pass reference to user
		return m_board;
	}
private:
	blitz::Array<RSP_Protocol::SystemStatus, 1> 	m_board;
};

//
// class ClockCommand
//
class ClockCommand : public Command
{
public:
	ClockCommand(GCFPortInterface& port);
	virtual ~ClockCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);

	void setClock(uint32 clock) {
		m_clock = clock;
	}
private:
	uint32 		m_clock;
};

//
// class SubClockCommand
//
class SubClockCommand : public Command
{
public:
	SubClockCommand(GCFPortInterface& port);
	virtual ~SubClockCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);

	uint32 getClock() const {
		return m_clock;
	}
private:
	uint32 		m_clock;
};

//
// class TDStatusCommand
//
class TDStatusCommand : public Command
{
public:
	TDStatusCommand(GCFPortInterface& port);
	virtual ~TDStatusCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	blitz::Array<TDStatus, 1>& board() {	// just pass reference to user
		return m_board;
	}
private:
	blitz::Array<RSP_Protocol::TDStatus, 1> 	m_board;
};


//
// class TBBCommand
//
class TBBCommand : public Command
{
public:
	TBBCommand(GCFPortInterface& port);
	virtual ~TBBCommand() {}
	enum {
		TRANSIENT = 1,
		SUBBANDS,
	};
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setSubbandSet(std::list<int> subbandlist) {
		m_subbandlist = subbandlist;
	}
	void setType(int type) {
		m_type = type;
	}
private:
	std::list<int> 		m_subbandlist;
	int 				m_type;
};

//
// class SICommand
//
class SICommand : public Command
{
public:
	SICommand(GCFPortInterface& port);
	virtual ~SICommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void setSI(bool	siOn) {
		m_siOn = siOn;
	}
private:
	bool 	m_siOn;
};

//
// class RegisterStateCommand
//
class RegisterStateCommand : public Command
{
public:
	RegisterStateCommand(GCFPortInterface& port);
	virtual ~RegisterStateCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void stop();
protected:
	memptr_t	 m_subscriptionhandle;
};

//
// class SPUStatusCommand
//
class SPUStatusCommand : public Command
{
public:
	SPUStatusCommand(GCFPortInterface& port);
	virtual ~SPUStatusCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	blitz::Array<SPUStatus, 1>& subrack() {	// just pass reference to user
		return 	itsSPUs;
	}
private:
	blitz::Array<RSP_Protocol::SPUStatus, 1> 	itsSPUs;
};

//
// class RawBlockCommand
//
class RawBlockCommand : public Command
{
public:
	RawBlockCommand(GCFPortInterface& port);
	virtual ~RawBlockCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void	setAddressInfo(uint16 RSPboard, uint32 address, uint16 offset);
	void	setDataInfo   (const string& filename, uint16 dataLen, uint8*	dataPtr);
	void	getAddressInfo(uint16* RSPboard, uint32* address, uint16* offset);
	void	getDataInfo   (const string& filename, uint16* dataLen, uint8** dataHandle);
private:
	uint16	itsRSPboard;
	uint32	itsAddress;
	uint16	itsOffset;
	uint16	itsDataLen;
	string	itsFileName;
	uint8	itsData[ETH_DATA_LEN];
};

//
// class SplitterCommand
//
class SplitterCommand : public Command
{
public:
	SplitterCommand(GCFPortInterface& port);
	virtual ~SplitterCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
	void state(bool switch_on) { itsState=switch_on; }
	bool state() const { return(itsState); }
private:
	bool		itsState;
};

//
// class VersionCommand
//
class VersionCommand : public Command
{
public:
	VersionCommand(GCFPortInterface& port);
	virtual ~VersionCommand() {}
	virtual void send();
	virtual GCFEvent::TResult ack(GCFEvent& e);
};

// Controller class for rspctl
//
// class RSPCtl
//
class RSPCtl : public GCFTask
{
public:
	// The constructor of the RSPCtl task.
	// @param name The name of the task. The name is used for looking
	// up connection establishment information using the GTMNameService and
	// GTMTopologyService classes.
	RSPCtl(string name, int argc, char** argv);
	virtual ~RSPCtl();

	// state methods

	// The initial state. In this state a connection with the RSP driver is attempted. 
	GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

	// Get the clockvalue.
	GCFEvent::TResult getClock(GCFEvent& e, GCFPortInterface &p);

	// Get a subscription on the clockvalue.
	GCFEvent::TResult sub2Clock(GCFEvent& e, GCFPortInterface &p);

	// Get a subscription on the splitter state.
	GCFEvent::TResult sub2Splitter(GCFEvent& e, GCFPortInterface &p);

	// In this state the command is sent and the acknowledge handled. Any relevant output is printed.
	GCFEvent::TResult doCommand(GCFEvent& e, GCFPortInterface &p);

	// Start the controller main loop.
	void mainloop();

private:
	// private methods
	Command* parse_options(int argc, char** argv);
	std::list<int> strtolist(const char* str, int max);
	void logMessage(ostream& stream, const string& message);

	// ports
	GCFTCPPort*		itsRSPDriver;

	// the command to execute
	Command* 		itsCommand;

	// dimensions of the connected hardware
	int 			m_nrcus;
	int 			m_nrspboards;
	int 			m_maxrspboards;

	// commandline parameters
	int    			m_argc;
	char** 			m_argv;

	int32	 		m_instancenr;

	// subscribtion admin
	bool			itsNeedClockOnce;		// getClock
	bool			itsNeedClock;			// subClock
	bool			itsNeedSplitter;		// subSplitter

	SubClockCommand m_subclock; // always subscribe to clock updates
};

  }; // namespace rspctl
}; // namespace LOFAR

#endif /* RSPCTL_H_ */
