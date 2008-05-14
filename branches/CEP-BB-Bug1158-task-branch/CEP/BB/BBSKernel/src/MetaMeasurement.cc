//# MetaMeasurement.cc:
//#
//# Copyright (C) 2008
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

#include <lofar_config.h>

#include <BBSKernel/MetaMeasurement.h>
#include <BBSKernel/Types.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobAipsIO.h>
#include <Blob/BlobSTL.h>

#include <measures/Measures/MeasConvert.h>
#include <measures/Measures/MCDirection.h>
#include <measures/Measures/MCPosition.h>
#include <casa/Quanta/MVDirection.h>
#include <casa/Quanta/MVPosition.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Quanta/MVTime.h>
#include <casa/Quanta/Quantum.h>

#include <Common/StreamUtil.h>
#include <Common/lofar_algorithm.h>
#include <Common/lofar_iomanip.h>

namespace LOFAR
{
namespace BBS
{

pair<double, double> MetaMeasurement::getFreqRange()
{
    ASSERT(itsParts.size() > 0);
    return make_pair(getFreqRange(0).first,
        getFreqRange(getPartCount() - 1).second);
}

MetaMeasurement::Part &MetaMeasurement::getPart(size_t part)
{
    if(!itsIsSorted)
    {
        stable_sort(itsParts.begin(), itsParts.end());
        itsIsSorted = true; 
    }
    
    return itsParts[part];
}

void MetaMeasurement::addPart(const string &host, const string &path,
    Axis::Pointer axis)
{
    itsParts.push_back(MetaMeasurement::Part(host, path, axis));
    itsIsSorted = false;        
}    

// -----------------------------------------------------------------------------
// BlobStream I/O
// -----------------------------------------------------------------------------

BlobOStream &operator<<(BlobOStream &out, const Station &obj)
{
    return out << obj.name << obj.position;
}

BlobIStream &operator>>(BlobIStream &in, Station &obj)
{
    return in >> obj.name >> obj.position;
}

BlobOStream &operator<<(BlobOStream &out, const Instrument &obj)
{
    return out << obj.name << obj.position << obj.stations;
}

BlobIStream &operator>>(BlobIStream &in, Instrument &obj)
{
    return in >> obj.name >> obj.position >> obj.stations;
}

BlobOStream &operator<<(BlobOStream &out, const casa::MDirection &obj)
{
    // Convert to J2000.
    casa::MDirection j2k = casa::MDirection::Convert(obj,
        casa::MDirection::Ref(casa::MDirection::J2000))();
    casa::Quantum<casa::Vector<double> > angles = j2k.getAngle("rad");

    return out << angles.getValue()(0) << angles.getValue()(1);
}

BlobIStream &operator>>(BlobIStream &in, casa::MDirection &obj)
{
    casa::Double angle1, angle2;
    in >> angle1 >> angle2;
    
    obj = casa::MDirection(casa::Quantity(angle1, "rad"),
        casa::Quantity(angle2, "rad"),
        casa::MDirection::Ref(casa::MDirection::J2000));

    return in;        
}

BlobOStream &operator<<(BlobOStream &out, const casa::MPosition &obj)
{
    // Convert to ITRF.
    casa::MPosition itrf = casa::MPosition::Convert(obj,
        casa::MPosition::Ref(casa::MPosition::ITRF))();
    casa::Quantum<casa::Vector<casa::Double> > coordinates = itrf.get("m");

    return out << coordinates.getValue()(0)
        << coordinates.getValue()(1)
        << coordinates.getValue()(2);
}

BlobIStream &operator>>(BlobIStream &in, casa::MPosition &obj)
{
    casa::Double x, y, z;

    in >> x >> y >> z;
    
    casa::MVPosition itrf(x, y, z);
    obj = casa::MPosition(itrf, casa::MPosition::Ref(casa::MPosition::ITRF));
    
    return in;
}

BlobOStream &operator<<(BlobOStream &out, const MetaMeasurement::Part &obj)
{
    out << obj.itsHostName
        << obj.itsPath;

    obj.itsFreqAxis->serialize(out);
    return out;
}

BlobIStream &operator>>(BlobIStream &in, MetaMeasurement::Part &obj)
{
    in >> obj.itsHostName
        >> obj.itsPath;
        
    obj.itsFreqAxis =
        Axis::Pointer(dynamic_cast<Axis*>(BlobStreamable::deserialize(in)));

    return in;
}

BlobOStream &operator<<(BlobOStream &out, const MetaMeasurement &obj)
{
//    vector<string> tmp(obj.itsPolarizations.size());
//    copy(obj.itsPolarizations.begin(), obj.itsPolarizations.end(), tmp.begin());

    out.putStart("MetaMeasurement", 1);
    out << obj.itsName
        << obj.itsPhaseCenter
        << obj.itsInstrument
//        << obj.itsBaselines
        << obj.itsPolarizations
        << obj.itsParts;

    obj.itsTimeAxis->serialize(out);

    out.putEnd();
    return out;
}

BlobIStream &operator>>(BlobIStream &in, MetaMeasurement &obj)
{
//    vector<string> tmp;

    in.getStart("MetaMeasurement");
    in >> obj.itsName
        >> obj.itsPhaseCenter
        >> obj.itsInstrument
//        >> obj.itsBaselines
        >> obj.itsPolarizations
        >> obj.itsParts;

    obj.itsTimeAxis =
        Axis::Pointer(dynamic_cast<Axis*>(BlobStreamable::deserialize(in)));
    
    in.getEnd();

//    obj.itsPolarizations = set<string>(tmp.begin(), tmp.end());

    return in;
}

ostream &operator<<(ostream &out, MetaMeasurement &obj)
{
    double timeStart = obj.itsTimeAxis->range().first;
    double timeEnd = obj.itsTimeAxis->range().second;

    casa::Quantum<casa::Vector<casa::Double> > angles =
        obj.itsPhaseCenter.getAngle("rad");

    out << "Name          : " << obj.itsName << endl
        << "Array position: " << obj.itsInstrument.position << endl
        << "Phase center  : "
        << casa::MVAngle::Format(casa::MVAngle::TIME, 6)
        << casa::MVAngle(angles.getValue()(0))
        << " "
        << casa::MVAngle::Format(casa::MVAngle::ANGLE, 6)
        << casa::MVAngle(angles.getValue()(1)) << endl
        << "Time          : "
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeStart, "s"))
        << " - "        
        << casa::MVTime::Format(casa::MVTime::YMD, 6)
        << casa::MVTime(casa::Quantum<casa::Double>(timeEnd, "s")) << endl
        << "Duration      : "
        << setprecision(3) << (timeEnd - timeStart) / 3600.0
        << " hours (" << obj.getTimeslotCount() << " samples of "
        << setprecision(3)
        << (timeEnd - timeStart) / obj.getTimeslotCount()
        << " s on average)" << endl
        << endl;

    out << "Stations: " << endl;
    out << setfill('0');
    const Instrument &instrument = obj.itsInstrument;
    for(size_t i = 0; i < instrument.getStationCount(); ++i)
    {
        out << "[" << setw(3) << i << "]: " << instrument.stations[i].name
            << endl;
    }
    out << endl;
    
    out << "Parts:" << endl;
    for(size_t i = 0; i < obj.itsParts.size(); ++i)
    {
        const MetaMeasurement::Part &part = obj.getPart(i);
        pair<double, double> range = part.getFreqRange();
        uint32 count = part.getChannelCount();

        string host(part.getHostName());
        if(host.empty())
        {
            host = "*";
        }

        out << "[" << setw(3) << i << "]: " << host << ":" << part.getPath()
            << " :: " << range.first / 1e6 << " - " << range.second / 1e6
            << " MHz (" << count << " channel(s) of "
            << (range.second - range.first) / count << " kHz on average)"
            << endl;
    }
    out << setfill(' ');
                
    return out;
}


} // namespace BBS
} // namespace LOFAR

