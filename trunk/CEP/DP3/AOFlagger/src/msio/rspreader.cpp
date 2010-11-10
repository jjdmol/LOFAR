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

#include <stdexcept>
#include <set>

#include <AOFlagger/msio/rspreader.h>

const unsigned char RSPReader::BitReverseTable256[256] = 
{
#   define R2(n)     n,     n + 2*64,     n + 1*64,     n + 3*64
#   define R4(n) R2(n), R2(n + 2*16), R2(n + 1*16), R2(n + 3*16)
#   define R6(n) R4(n), R4(n + 2*4 ), R4(n + 1*4 ), R4(n + 3*4 )
		R6(0), R6(2), R6(1), R6(3)
};

void RSPReader::Read()
{
	std::ifstream file(_rawFile.c_str(), std::ios_base::binary);
	RCPApplicationHeader header;
	size_t frame = 0;
	std::set<short> stations;
	std::set<unsigned int> timestamps;
	unsigned int lastTimestamp = 0;
	unsigned row = 0, col = 0;
	do {
		header.Read(file);
		if(header.versionId != 2)
		{
			std::stringstream s;
			s << "Corrupted header found in frame " << frame << "!";
			throw std::runtime_error(s.str());
		}
		if(stations.count(header.stationId)==0)
		{
			std::cout << "Found station: " << header.stationId << "\n";
			stations.insert(header.stationId);
		}
		if(timestamps.count(header.timestamp)==0)
		{
			std::cout << "(new timestamp)\n";
			timestamps.insert(header.timestamp);
		}
		if(header.timestamp != lastTimestamp)
		{
			std::cout << "Timestep " << header.timestamp << ", row=" << row << "\n";
			lastTimestamp = header.timestamp;
		}
		//std::cout << "BlockSeq " << header.blockSequenceNumber << "\n";
		for(unsigned i=0;i<80;++i)
		{
			RCPBeamletData data;
			data.Read(file);
		}
		++frame;
		++col;
		if(col >= header.nofBlocks)
		{
			col = 0;
			++row;
		}
	} while(file.good());
	std::cout << "End of file, read " << frame << " frames." << std::endl;
}
