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
#include <math.h>
#include <vector>

#include "types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
struct EarthPosition {
	num_t x, y, z;
	std::string ToString() {
		std::stringstream s;
		s.setf(std::ios::fixed,std::ios::floatfield);
		s.width(16);
		s.precision(16);
		s << x << "," << y << "," << z << " (alt " << sqrtl(x*x+y*y+z*z) << ")";
		return s.str();
	}
	EarthPosition FromITRS(long double x, long double y, long double z);
};

struct UVW {
	num_t u, v, w;
};

struct AntennaInfo {
	AntennaInfo() { }
	AntennaInfo(const AntennaInfo &source)
		: id(source.id), position(source.position), name(source.name), diameter(source.diameter), mount(source.mount), station(source.station)
	{
	}
	unsigned id;
	EarthPosition position;
	std::string name;
	double diameter;
	std::string mount;
	std::string station;
};

struct ChannelInfo {
	unsigned frequencyIndex;
	double frequencyHz;
	double channelWidthHz;
	double effectiveBandWidthHz;
	double resolutionHz;
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

	long double Distance() {
		num_t dx = antenna1.x-antenna2.x;
		num_t dy = antenna1.y-antenna2.y;
		num_t dz = antenna1.z-antenna2.z;
		return sqrtl(dx*dx+dy*dy+dz*dz);
	}
	long double Angle() {
		num_t dz = antenna1.z-antenna2.z;
 		// baseline is either orthogonal to the earths axis, or
		// the length of the baseline is zero. 
		if(dz == 0.0) return 0.0;
		num_t transf = 1.0/(antenna1.z-antenna2.z);
		num_t dx = (antenna1.x-antenna2.x)*transf;
		num_t dy = (antenna1.y-antenna2.y)*transf;
		num_t length = sqrtl(dx*dx + dy*dy + 1.0);
		return acosl(1.0/length);
	}
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

#endif
