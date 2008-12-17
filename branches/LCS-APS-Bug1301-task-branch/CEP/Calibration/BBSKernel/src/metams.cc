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
#include <Common/lofar_set.h>
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
    
    const vector<baseline_t> &baselines = dims.getBaselines();
    meta.setBaselines(set<baseline_t>(baselines.begin(), baselines.end()));
    
    const vector<string> &polarizations = dims.getPolarizations();
    meta.setPolarizations(set<string>(polarizations.begin(),
        polarizations.end()));

    meta.addPart(host, path, dims.getFreqAxis());
}


bool mergeMeta(MetaMeasurement &lhs, MetaMeasurement &rhs)
{
    if(rhs.getPartCount() != 1)
    {
        cout << "error: " << rhs.getName() << ": has more than 1 part." << endl;
        return false;
    }
    
    // Check phase center.
    /*
    if(lhs.getPhaseCenter() != rhs.getPhaseCenter())
    {
        cout << "error: " << rhs.getName() << ": inconsistent phase center."
            << endl;
        return false;
    }
    */
    
    // Check instrument.
    const Instrument &instrumentLhs = lhs.getInstrument();
    const Instrument &instrumentRhs = rhs.getInstrument();
    
    if(instrumentLhs.name != instrumentRhs.name
//        || instrumentLhs.position != instrumentRhs.position
        || instrumentLhs.stations.size() != instrumentRhs.stations.size())
    {
        cout << "error: " << rhs.getName() << ": inconsistent instrument."
            << endl;
        return false;
    }
    else
    {
        size_t i = 0;
        bool ok = true;
        while(ok && i < instrumentLhs.stations.size())
        {
            ok = ok 
                && (instrumentLhs.stations[i].name
                    == instrumentRhs.stations[i].name);
//                && (instrumentLhs.stations[i].position
//                    == instrumentRhs.stations[i].position);
            ++i;                    
        }
        
        if(!ok)
        {
            cout << "error: " << rhs.getName() << ": inconsistent instrument."
                << endl;
            return false;
        }
    }

    // Check time axis.
    Axis::ShPtr timeLhs = lhs.getTimeAxis();
    Axis::ShPtr timeRhs = rhs.getTimeAxis();

    if(timeLhs->size() != timeRhs->size())
    {
        cout << "error: " << rhs.getName() << ": inconsistent time axis."
            << endl;
        return false;
    }
            
    bool ok = true;
    for(size_t i = 0; i < timeLhs->size(); ++i)
    {
        if(timeLhs->center(i) != timeRhs->center(i)
            || timeLhs->width(i) != timeRhs->width(i))
        {
            ok = false;
            break;
        }
    }
    
    if(!ok)
    {
        cout << "error: " << rhs.getName() << ": inconsistent time axis."
            << endl;
        return false;
    }

    // Merge baselines.
    const set<baseline_t> &blLhs = lhs.getBaselines();
    const set<baseline_t> &blRhs = rhs.getBaselines();

    set<baseline_t> blUnion;
    set_union(blLhs.begin(), blLhs.end(), blRhs.begin(), blRhs.end(),
        inserter(blUnion, blUnion.begin()));
    lhs.setBaselines(blUnion);

    // Merge polarizations.
    const set<string> &polLhs = lhs.getPolarizations();
    const set<string> &polRhs = rhs.getPolarizations();

    set<string> polUnion;
    set_union(polLhs.begin(), polLhs.end(), polRhs.begin(), polRhs.end(),
        inserter(polUnion, polUnion.begin()));
    lhs.setPolarizations(polUnion);
    
    // Add part.
    lhs.addPart(rhs.getHostName(0), rhs.getPath(0), rhs.getFreqAxis(0));

    return true;
}


void usage()
{
    cout << "Usage: metams <command> <arguments> <file>..." << endl;
    cout << endl;
    cout << "Available commands:" << endl;
    cout << "    -x, --extract <measurement-file>...         Extract meta data."
        << endl;
    cout << "    -m, --merge <meta-name> <meta-file>...      Merge meta"
        " file(s)." << endl;
    cout << "    -s, --show <meta-file>                      Show contents of"
        " meta file." << endl;
    cout << endl;
    cout << "Examples:" << endl;
    cout << "    metams --show test.meta" << endl;
    cout << "    metams --extract SB0.MS SB1.MS lioff002:/data/SB2.MS" << endl;
    cout << "       NOTE: if a hostname is specified, the measurement should be"
        " readable" << endl;
    cout << "       at the specified path from the host on which metams is run."
        << endl;
    cout << "    metams --merge L2007_01221.meta SB0.meta SB1.meta SB2.meta"
        << endl;
    cout << endl;
}


int main(int argc, char **argv)
{
//    INIT_LOGGER("metams");

    if(argc < 2)
    {
        usage();
        return 1;
    }
    
    string command(argv[1]);
    if(command == "-x" || command == "--extract")
    {
        if(argc < 3)
        {
            usage();
            return 1;
        }

        cout << "extracting meta data..." << endl;
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

            MetaMeasurement meta(name);
            MeasurementAIPS::Pointer measurement
                (new MeasurementAIPS(path.absoluteName()));
            
            extractMeta(meta, host, path.absoluteName(), measurement);

            ofstream fout((meta.getName() + ".meta").c_str());
            BlobOBufStream bufs(fout);
            BlobOStream bos(bufs);
            bos << meta;
            
            cout << base << endl;
        }
        
        cout << endl;
    }
    else if(command == "-m" || command == "--merge")
    {
        if(argc < 4)
        {
            usage();
            return 1;
        }

        // Strip extension.
        string name;
        casa::Path path(argv[2]);
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

        cout << "merging meta data..." << endl;

        MetaMeasurement meta;
        ifstream fin(argv[3]);
        BlobIBufStream ibufs(fin);
        BlobIStream bis(ibufs);
        bis >> meta;

        meta.setName(name);
        cout << casa::Path(argv[3]).baseName() << endl;

        bool ok = true;
        size_t i = 4;
        while(ok && i < static_cast<size_t>(argc))
        {
            MetaMeasurement rhs;

            ifstream fin(argv[i]);
            BlobIBufStream ibufs(fin);
            BlobIStream bis(ibufs);
            bis >> rhs;
            
            ok = mergeMeta(meta, rhs);
            cout << casa::Path(argv[i]).baseName() << endl;

            ++i;
        }
            
        if(!ok)
        {
            cout << endl << "exit due to error in " << argv[i - 1] << endl;
            return 1;
        }
        
        ofstream fout(argv[2]);
        BlobOBufStream obufs(fout);
        BlobOStream bos(obufs);
        bos << meta;
        
        cout << endl << meta << endl;
    }        
    else if(command == "-s" || command == "--show")
    {
        if(argc != 3)
        {
            usage();
            return 1;
        }

        MetaMeasurement meta;
        ifstream fin(argv[2]);
        BlobIBufStream ibufs(fin);
        BlobIStream bis(ibufs);
        bis >> meta;
    
        cout << meta << endl;
    }
    else
    {
        usage();
        return 1;
    }

    return 0;
}

