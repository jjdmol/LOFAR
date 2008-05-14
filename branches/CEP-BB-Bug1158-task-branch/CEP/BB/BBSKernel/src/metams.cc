//# metams.cc:
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iomanip.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_fstream.h>

#include <Blob/BlobOBufStream.h>
#include <Blob/BlobOStream.h>
#include <Blob/BlobIBufStream.h>
#include <Blob/BlobIStream.h>

#include <BBSKernel/MetaMeasurement.h>
#include <BBSKernel/MeasurementAIPS.h>

//#include <Common/lofar_sstream.h>
//#include <casa/BasicMath/Math.h>
//#include <casa/Utilities/GenSort.h>

using namespace casa;
using namespace LOFAR;
using namespace LOFAR::BBS;

void extractMeta(MetaMeasurement &meta, const string &host, const string &path,
    Measurement::Pointer ms)
{
    meta.setPhaseCenter(ms->getPhaseCenter());
    
    const Instrument &instrument = ms->getInstrument();
    meta.setInstrument(instrument);
    
    const VisDimensions &dims = ms->getDimensions();
    meta.setTimeAxis(dims.getTimeAxis());
    
    meta.addPart(host, path, dims.getFreqAxis());
}


bool mergeMeta(MetaMeasurement &lhs, MetaMeasurement &rhs)
{
/*
    if(rhs.getPartCount() != 1)
    {
        LOG_ERROR_STR("" << rhs.getName() << ": has more than 1 part.");
        return false;
    }
    
    if(lhs.getPhaseCenter() != rhs.getPhaseCenter())
    {
        LOG_ERROR_STR("" << rhs.getName() << ": inconsistent phase center.");
        return false;
    }

    if(lhs.getArrayPosition() != rhs.getArrayPosition())
    {
        LOG_ERROR_STR("" << rhs.getName() << ": inconsistent array position.");
        return false;
    }

    if(lhs.getTimes() != rhs.getTimes()
        || lhs.getIntervals() != rhs.getIntervals())
    {
        LOG_ERROR_STR("" << rhs.getName() << ": inconsistent time axis.");
        return false;
    }
    
    const set<string> &polLhs = lhs.getPolarizations();
    const set<string> &polRhs = rhs.getPolarizations();
    set<string> polUnion;

    set_union(polLhs.begin(), polLhs.end(), polRhs.begin(), polRhs.end(),
        inserter(polUnion, polUnion.begin()));
    lhs.setPolarizations(polUnion);
    
    pair<double, double> range = rhs.getFreqRange(0);
    lhs.addPart(rhs.getHostName(0), rhs.getPath(0), range.first, range.second,
        rhs.getChannelCount(0));
*/

    return true;
}


int main(int argc, char **argv)
{
    INIT_LOGGER("test");
    LOG_INFO_STR("starting up ...");

    ASSERT(argc >= 2);
    
    string command(argv[1]);
    if(command == "--extract")
    {
        ASSERT(argc >= 3);

        for(size_t i = 2; i < static_cast<size_t>(argc); ++i)
        {
            string host;
            casa::Path path;
            string file(argv[i]);
            
            // Split filename.
            size_t colon = file.find(':');
            if(colon == file.npos)
            {
                path = casa::Path(file);
            }
            else
            {
                string tmp;
                host.insert(0, file, 0, colon);
                
                if(file.size() > colon)
                {
                    tmp.insert(0, file, colon + 1, file.size() - colon - 1);
                }
                path = casa::Path(tmp);
            }

            LOG_DEBUG_STR("Host: " << host);
            LOG_DEBUG_STR("Path: " << path.originalName());
            
            // Strip extension.
            string name;
            casa::String base = path.baseName();
            size_t dot = base.find('.');
            if(dot == base.npos)
            {
                name = string(base.begin(), base.end());
            }
            else
            {
                name.insert(0, base, 0, dot);
            }

            LOG_DEBUG_STR("Meta name: " << name);
            
            MetaMeasurement meta(name);
            MeasurementAIPS::Pointer measurement
                (new MeasurementAIPS(path.absoluteName()));
            
            extractMeta(meta, host, path.absoluteName(), measurement);

            ofstream fout((meta.getName() + ".meta").c_str());
            BlobOBufStream bufs(fout);
            BlobOStream bos(bufs);
            bos << meta;
        }
    }
    else if(command == "--merge")
    {
        ASSERT(argc >= 4);

        MetaMeasurement meta;
        ifstream fin(argv[3]);
        BlobIBufStream ibufs(fin);
        BlobIStream bis(ibufs);
        bis >> meta;
        
        meta.setName(argv[2]);
        
        for(size_t i = 4; i < static_cast<size_t>(argc); ++i)
        {
            MetaMeasurement rhs;

            ifstream fin(argv[i]);
            BlobIBufStream ibufs(fin);
            BlobIStream bis(ibufs);
            bis >> rhs;
            
            mergeMeta(meta, rhs);
        }
            
        ofstream fout((meta.getName() + ".meta").c_str());
        BlobOBufStream obufs(fout);
        BlobOStream bos(obufs);
        bos << meta;
        
        LOG_INFO_STR("Measurement info:" << endl << meta);
    }        
    else if(command == "--show")
    {
        ASSERT(argc >= 3);

        MetaMeasurement meta;
        ifstream fin(argv[2]);
        BlobIBufStream ibufs(fin);
        BlobIStream bis(ibufs);
        bis >> meta;
    
        LOG_INFO_STR("Measurement info:" << endl << meta);
    }
    else
    {
//        usage();
          return 1;
    }

    return 0;
}

