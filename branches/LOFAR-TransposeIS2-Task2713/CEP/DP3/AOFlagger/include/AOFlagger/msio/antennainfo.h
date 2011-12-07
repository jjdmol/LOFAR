/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef ANTENNAINFO_H
#define ANTENNAINFO_H

#include <string>
#include <sstream>
#include <cmath>
#include <cstdlib>
#include <vector>

#include "types.h"

#include <AOFlagger/util/serializable.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
struct EarthPosition : public Serializable {
	double x, y, z;
	std::string ToString() {
		std::stringstream s;
		s.setf(std::ios::fixed,std::ios::floatfield);
		s.width(16);
		s.precision(16);
		s << x << "," << y << "," << z << " (alt " << sqrtl(x*x+y*y+z*z) << "), or "
		<< "N" << Lattitude()*180/M_PI << " E" << Longitude()*180/M_PI;
		return s.str();
	}
	EarthPosition FromITRS(long double x, long double y, long double z);
	
	double Longitude() const
	{
		return atan2l(y, x);
	}

	double Lattitude() const
	{
		return atan2l(z, sqrtl((long double) x*x + y*y));
	}
	
	double Altitude() const
	{
		return sqrtl((long double) x*x+y*y+z*z);
	}

	virtual void Serialize(std::ostream &stream) const
	{
		SerializeToDouble(stream, x);
		SerializeToDouble(stream, y);
		SerializeToDouble(stream, z);
	}
	
	virtual void Unserialize(std::istream &stream)
	{
		x = UnserializeDouble(stream);
		y = UnserializeDouble(stream);
		z = UnserializeDouble(stream);
	}
};

struct UVW {
	UVW() : u(0.0), v(0.0), w(0.0) { }
	UVW(num_t _u, num_t _v, num_t _w) : u(_u), v(_v), w(_w) { }
	num_t u, v, w;
};

struct AntennaInfo : public Serializable {
	AntennaInfo() { }
	AntennaInfo(const AntennaInfo &source)
		: id(source.id), position(source.position), name(source.name), diameter(source.diameter), mount(source.mount), station(source.station)
	{
	}
	void operator=(const AntennaInfo &source)
	{
		id = source.id;
		position = source.position;
		name = source.name;
		diameter = source.diameter;
		mount = source.mount;
		station = source.station;
	}
	unsigned id;
	EarthPosition position;
	std::string name;
	double diameter;
	std::string mount;
	std::string station;
	
	virtual void Serialize(std::ostream &stream) const
	{
		SerializeToUInt32(stream, id);
		position.Serialize(stream);
		SerializeToString(stream, name);
		SerializeToDouble(stream, diameter);
		SerializeToString(stream, mount);
		SerializeToString(stream, station);
	}
	
	virtual void Unserialize(std::istream &stream)
	{
		id = UnserializeUInt32(stream);
		position.Unserialize(stream);
		UnserializeString(stream, name);
		diameter = UnserializeDouble(stream);
		UnserializeString(stream, mount);
		UnserializeString(stream, station);
	}
};

struct ChannelInfo {
	unsigned frequencyIndex;
	double frequencyHz;
	double channelWidthHz;
	double effectiveBandWidthHz;
	double resolutionHz;
	
	double MetersToLambda(double meters) const
	{
		return meters * frequencyHz / 299792458.0L;
	}
};

struct BandInfo {
	unsigned windowIndex;
	unsigned channelCount;
	std::vector<ChannelInfo> channels;

	BandInfo() { }
	BandInfo(const BandInfo &source) :
		windowIndex(source.windowIndex),
		channelCount(source.channelCount),
		channels(source.channels)
	{
	}
	void operator=(const BandInfo &source)
	{
		windowIndex = source.windowIndex;
		channelCount = source.channelCount;
		channels = source.channels;
	}
	num_t CenterFrequencyHz() const
	{
		num_t total = 0.0;
		for(std::vector<ChannelInfo>::const_iterator i=channels.begin();i!=channels.end();++i)
			total += i->frequencyHz;
		return total / channels.size();
	}
};

struct FieldInfo {
	unsigned fieldId;
	num_t delayDirectionRA;
	num_t delayDirectionDec;
	num_t delayDirectionDecNegSin;
	num_t delayDirectionDecNegCos;
};

struct Baseline {
	EarthPosition antenna1, antenna2;
	Baseline(const AntennaInfo &_antenna1, const AntennaInfo &_antenna2)
		: antenna1(_antenna1.position), antenna2(_antenna2.position) { }
	Baseline(EarthPosition _antenna1, EarthPosition _antenna2)
		: antenna1(_antenna1), antenna2(_antenna2) { }

	num_t Distance() const {
		num_t dx = antenna1.x-antenna2.x;
		num_t dy = antenna1.y-antenna2.y;
		num_t dz = antenna1.z-antenna2.z;
		return sqrtn(dx*dx+dy*dy+dz*dz);
	}
	num_t Angle() const {
		num_t dz = antenna1.z-antenna2.z;
 		// baseline is either orthogonal to the earths axis, or
		// the length of the baseline is zero. 
		if(dz == 0.0) return 0.0;
		num_t transf = 1.0/(antenna1.z-antenna2.z);
		num_t dx = (antenna1.x-antenna2.x)*transf;
		num_t dy = (antenna1.y-antenna2.y)*transf;
		num_t length = sqrtn(dx*dx + dy*dy + 1.0);
		return acosn(1.0/length);
	}
	num_t DeltaX() const { return antenna2.x-antenna1.x; }
	num_t DeltaY() const { return antenna2.y-antenna1.y; }
	num_t DeltaZ() const { return antenna2.z-antenna1.z; }
};

struct Frequency {
	static std::string ToString(num_t value)
	{
		std::stringstream s;
		if(value >= 1000000.0L)
			s << round(value/10000.0L)/100.0L << " MHz";
		else if(value >= 1000.0L)
			s << round(value/10.0L)/100.0L << " KHz";
		else
			s << value << " Hz";
		return s.str();
	}
};

struct RightAscension {
	static std::string ToString(numl_t value)
	{
		value = fmod(value, 2.0*M_PInl);
		if(value < 0.0) value += 2.0*M_PInl;
		std::stringstream s;
		s << (int) floorn(value*12.0/M_PInl) << ':';
		int d2 = (int) floornl(fmodnl(value*12.0*60.0/M_PInl, 60.0));
		if(d2 < 10) s << '0';
		s << d2 << ':';
		numl_t d3 = fmodnl(value*12.0*60.0*60.0/M_PInl, 60.0);
		if(d3 < 10.0) s << '0';
		s << d3;
		return s.str();
	}
};

struct Declination {
	static std::string ToString(numl_t value)
	{
		value = fmod(value, 2.0*M_PInl);
		if(value < 0.0) value += 2.0*M_PInl;
		if(value > M_PInl*0.5) value = M_PInl - value;
		std::stringstream s;
		if(value > 0.0)
			s << '+';
		else
			s << '-';
		value = fabsnl(value);
		s << (int) floornl(value*180.0/M_PIn) << '.';
		int d2 = (int) fmodnl(value*180.0*60.0/M_PIn, 60.0);
		if(d2 < 10) s << '0';
		s << d2 << '.';
		numl_t d3 = fmodnl(value*180.0*60.0*60.0/M_PIn, 60.0);
		if(d3 < 10.0) s << '0';
		s << d3;
		return s.str();
	}
};

struct Angle {
	static std::string ToString(numl_t valueRad)
	{
		std::stringstream s;
		numl_t deg = valueRad * 180.0/M_PI;
		if(std::abs(deg) > 3)
			s << deg << " deg";
		else if(std::abs(deg) > 3.0/60.0)
			s << (deg / 60.0) << " arcmin";
		else
			s << (deg / 3600.0) << " arcsec";
		return s.str();
	}
};

#endif
