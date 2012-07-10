/***************************************************************************
 *   Copyright (C) 2011 by A.R. Offringa   *
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

#ifndef MS_ROW_DATAEXT_H
#define MS_ROW_DATAEXT_H

#include "msrowdata.h"

#include <AOFlagger/util/serializable.h>

class MSRowDataExt : public Serializable
{
	public:
		MSRowDataExt()
		: _data()
		{
		}
		
		MSRowDataExt(unsigned polarizationCount, unsigned channelCount)
		: _data(polarizationCount, channelCount)
		{
		}
		
		const MSRowData &Data() const { return _data; }
		MSRowData &Data() { return _data; }
		unsigned Antenna1() const { return _antenna1; }
		unsigned Antenna2() const { return _antenna2; }
		double U() const { return _u; }
		double V() const { return _v; }
		double W() const { return _w; }
		double Timestep() const { return _timestep; }
		
		virtual void Serialize(std::ostream &stream) const
		{
			_data.Serialize(stream);
			SerializeToUInt32(stream, _antenna1);
			SerializeToUInt32(stream, _antenna2);
			SerializeToDouble(stream, _u);
			SerializeToDouble(stream, _v);
			SerializeToDouble(stream, _w);
			SerializeToDouble(stream, _timestep);
		}
		
		virtual void Unserialize(std::istream &stream)
		{
			_data.Unserialize(stream);
			_antenna1 = UnserializeUInt32(stream);
			_antenna2 = UnserializeUInt32(stream);
			_u = UnserializeDouble(stream);
			_v = UnserializeDouble(stream);
			_w = UnserializeDouble(stream);
			_timestep = UnserializeDouble(stream);
		}
	private:
		MSRowData _data;
		unsigned _antenna1, _antenna2;
		double _u, _v, _w;
		double _timestep;
};

#endif
