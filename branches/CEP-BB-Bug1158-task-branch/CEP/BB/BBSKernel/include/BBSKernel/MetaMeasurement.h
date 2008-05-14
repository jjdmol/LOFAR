//# MetaMeasurement.h: 
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

#ifndef LOFAR_BBS_METAMEASUREMENT_H
#define LOFAR_BBS_METAMEASUREMENT_H

#include <BBSKernel/Axis.h>
#include <BBSKernel/Measurement.h>

#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

#include <casa/OS/Path.h>

namespace LOFAR
{
    //# Forward Declarations
    class BlobOStream;
    class BlobIStream;

namespace BBS
{
    class MetaMeasurement
    {
    public:
        MetaMeasurement(const string &name = "")
            :   itsName(name),
                itsIsSorted(false)
        {}
        
        void setName(const string &name)
        { itsName = name; }
        
        void setPhaseCenter(const casa::MDirection &center)
        { itsPhaseCenter = center; }
        
        void setInstrument(const Instrument &instrument)
        { itsInstrument = instrument; }

        void setTimeAxis(Axis::Pointer axis)
        { itsTimeAxis = axis; }
        
        void setBaselines(const vector<baseline_t> &baselines)
        { itsBaselines = baselines; }

        void setPolarizations(const vector<string> &polarizations)
        { itsPolarizations = polarizations; }
        
        void addPart(const string &host, const string &path,
            Axis::Pointer axis);

// -----------------------------------------------------------------------------
        const string &getName() const
        { return itsName; }
        
        casa::MDirection getPhaseCenter() const
        { return itsPhaseCenter; }
        
        const Instrument &getInstrument() const
        { return itsInstrument; }
        
        pair<double, double> getFreqRange();

        const Axis::Pointer getTimeAxis() const
        { return itsTimeAxis; }
        pair<double, double> getTimeRange() const
        { return itsTimeAxis->range(); }
        size_t getTimeslotCount() const
        { return itsTimeAxis->size(); }

        const vector<baseline_t> &getBaselines() const
        { return itsBaselines; }
        size_t getBaselineCount() const
        { return itsBaselines.size(); }

        const vector<string> &getPolarizations() const
        { return itsPolarizations; }
        size_t getPolarizationCount() const
        { return itsPolarizations.size(); }

        size_t getPartCount() const
        { return itsParts.size(); }
        
        string getHostName(size_t part)
        { return getPart(part).getHostName(); }

        string getPath(size_t part)
        { return getPart(part).getPath(); }

        const Axis::Pointer getFreqAxis(size_t part)
        { return getPart(part).getFreqAxis(); }
        pair<double, double> getFreqRange(size_t part)
        { return getPart(part).getFreqRange(); }
        size_t getChannelCount(size_t part)
        { return getPart(part).getChannelCount(); }


    private:
        class Part
        {
        public:
            Part()
            {}
            
            Part(const string &host, const string &path, Axis::Pointer axis)
                :   itsHostName(host),
                    itsFreqAxis(axis)
            {
                casa::String absPath = casa::Path(path).absoluteName();
                itsPath = string(absPath.begin(), absPath.end());
            }                                    
            
            string getHostName() const
            { return itsHostName; }

            string getPath() const
            { return itsPath; }

            const Axis::Pointer getFreqAxis() const
            { return itsFreqAxis; }

            size_t getChannelCount() const
            { return itsFreqAxis->size(); }

            pair<double, double> getFreqRange() const
            { return itsFreqAxis->range(); }

            bool operator<(const Part &other) const
            { return itsFreqAxis->lower(0) < other.itsFreqAxis->lower(0); }

        private:
            string                  itsHostName;
            string                  itsPath;
            Axis::Pointer           itsFreqAxis;

            friend BlobOStream &operator<<(BlobOStream &out,
                const MetaMeasurement::Part &obj);
            friend BlobIStream &operator>>(BlobIStream &in,
                MetaMeasurement::Part &obj);
        };

        Part &getPart(size_t part);

        string              itsName;
        casa::MDirection    itsPhaseCenter;
        Instrument          itsInstrument;
        
        Axis::Pointer       itsTimeAxis;
        vector<baseline_t>  itsBaselines;
        vector<string>      itsPolarizations;

        vector<Part>        itsParts;
        
        bool                itsIsSorted;

        friend BlobOStream &operator<<(BlobOStream &out,
            const MetaMeasurement &obj);
        friend BlobOStream &operator<<(BlobOStream &out,
            const MetaMeasurement::Part &obj);

        friend BlobIStream &operator>>(BlobIStream &in,
            MetaMeasurement &obj);
        friend BlobIStream &operator>>(BlobIStream &in,
            MetaMeasurement::Part &obj);
        friend ostream &operator<<(ostream &out, MetaMeasurement &obj);
    };

    BlobOStream &operator<<(BlobOStream &out, const MetaMeasurement &obj);
    BlobIStream &operator>>(BlobIStream &in, MetaMeasurement &obj);

    BlobOStream &operator<<(BlobOStream &out, const Station &obj);
    BlobIStream &operator>>(BlobIStream &in, Station &obj);
    BlobOStream &operator<<(BlobOStream &out, const Instrument &obj);
    BlobIStream &operator>>(BlobIStream &in, Instrument &obj);
    BlobOStream &operator<<(BlobOStream &out, const casa::MDirection &obj);
    BlobIStream &operator>>(BlobIStream &in, casa::MDirection &obj);
    BlobOStream &operator<<(BlobOStream &out, const casa::MPosition &obj);
    BlobIStream &operator>>(BlobIStream &in, casa::MPosition &obj);

    ostream &operator<<(ostream &out, MetaMeasurement &obj);

} // namespace BBS
} // namespace LOFAR

#endif
