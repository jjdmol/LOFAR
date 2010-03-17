#ifndef OBSERVATORIUM_H
#define OBSERVATORIUM_H

#include <vector>

#include <AOFlagger/msio/antennainfo.h>

class Observatorium
{
	public:
		void AddAntenna(AntennaInfo &antenna)
		{
			_antennae.push_back(antenna);
		}
		size_t AntennaCount() const { return _antennae.size(); }
		const AntennaInfo &GetAntenna(size_t index) const { return _antennae[index]; }
	private:
		std::vector<AntennaInfo> _antennae;
};

struct WSRTObservatorium : public Observatorium
{
	WSRTObservatorium()
	{
		AntennaInfo antennas[14];
		for(unsigned i=0;i<14;++i)
			WSRTCommon(antennas[i]);
		WSRT0(antennas[0]);
		WSRT1(antennas[1]);
		WSRT2(antennas[2]);
		WSRT3(antennas[3]);
		WSRT4(antennas[4]);
		WSRT5(antennas[5]);
		WSRT6(antennas[6]);
		WSRT7(antennas[7]);
		WSRT8(antennas[8]);
		WSRT9(antennas[9]);
		WSRTA(antennas[10]);
		WSRTB(antennas[11]);
		WSRTC(antennas[12]);
		WSRTD(antennas[13]);
		for(unsigned i=0;i<4;++i)  // ONLY USE FIRST 4 FOR NOW
			AddAntenna(antennas[i]);
	}

	private:
		void WSRTCommon(AntennaInfo &antenna)
		{
			antenna.diameter = 25;
			antenna.mount = "equatorial";
			antenna.station = "WSRT";
		}
		void WSRT0(AntennaInfo &antenna)
		{
			antenna.id = 0;
			antenna.name = "RT0";
			antenna.position.x = 3.82876e+06;
			antenna.position.y = 442449;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT1(AntennaInfo &antenna)
		{
			antenna.id = 1;
			antenna.name = "RT1";
			antenna.position.x = 3.82875e+06;
			antenna.position.y = 442592;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT2(AntennaInfo &antenna)
		{
			antenna.id = 2;
			antenna.name = "RT2";
			antenna.position.x = 3.82873e+06;
			antenna.position.y = 442735;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT3(AntennaInfo &antenna)
		{
			antenna.id = 3;
			antenna.name = "RT3";
			antenna.position.x = 3.82871e+06;
			antenna.position.y = 442878;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT4(AntennaInfo &antenna)
		{
			antenna.id = 4;
			antenna.name = "RT4";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT5(AntennaInfo &antenna)
		{
			antenna.id = 5;
			antenna.name = "RT5";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT6(AntennaInfo &antenna)
		{
			antenna.id = 6;
			antenna.name = "RT6";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT7(AntennaInfo &antenna)
		{
			antenna.id = 7;
			antenna.name = "RT7";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT8(AntennaInfo &antenna)
		{
			antenna.id = 8;
			antenna.name = "RT8";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRT9(AntennaInfo &antenna)
		{
			antenna.id = 9;
			antenna.name = "RT9";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRTA(AntennaInfo &antenna)
		{
			antenna.id = 10;
			antenna.name = "RTA";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRTB(AntennaInfo &antenna)
		{
			antenna.id = 11;
			antenna.name = "RTB";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRTC(AntennaInfo &antenna)
		{
			antenna.id = 12;
			antenna.name = "RTC";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
		void WSRTD(AntennaInfo &antenna)
		{
			antenna.id = 13;
			antenna.name = "RTD";
			antenna.position.x = 0;
			antenna.position.y = 0;
			antenna.position.z = 5.06492e+06;
		}
};

#endif
