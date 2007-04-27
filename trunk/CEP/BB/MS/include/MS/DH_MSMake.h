//#  DH_MSMake.h: DataHolder for the MS info
//#
//#  Copyright (C) 2005
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

#ifndef BB_MS_DH_MSINFO_H
#define BB_MS_DH_MSINFO_H

#include <Transport/DataHolder.h>
#include <Transport/TH_MPI.h>
#include <Transport/Connection.h>
#include <vector>

//# Forward Declarations.
namespace casa {
  class String;
  template<class T> class Array;
}


namespace LOFAR {

  class DH_MSMake: public DataHolder
  {
  public:
    DH_MSMake (const string& dhName);
    DH_MSMake (const DH_MSMake&);
    virtual ~DH_MSMake();
    virtual DH_MSMake* clone() const;
    virtual void init();
    virtual void fillDataPointers();

    void setFreq (double start, double end, int nband, int nfreq)
      { itsFreqs[0]=start; itsFreqs[1]=end; *itsNBand=nband; *itsNFreq=nfreq; }
    void setTime (double start, double end, int n)
      { itsTimes[0]=start; itsTimes[1]=end; *itsNTime=n; }
    void setTileSize (int tileSizeFreq, int tileSizeRest)
      { *itsTileSizeFreq = tileSizeFreq; *itsTileSizeRest = tileSizeRest; }
    void fillExtra (const string& msName,
		    const casa::Array<double>& ra,
		    const casa::Array<double>& dec,
		    const casa::Array<double>& antPos,
		    const casa::Array<casa::String>& antNames,
		    bool writeAutoCorr);

    void getFreq (double& start, double& end, int& nband, int& nfreq) const
      { start=itsFreqs[0]; end=itsFreqs[1]; nband=*itsNBand; nfreq=*itsNFreq; }
    void getTime (double& start, double& end, int& n) const
      { start=itsTimes[0]; end=itsTimes[1]; n=*itsNTime; }
    int getTileSizeFreq() const
      { return *itsTileSizeFreq; }
    int getTileSizeRest() const
      { return *itsTileSizeRest; }
    void getExtra (string& msName,
		   casa::Array<double>& ra,
		   casa::Array<double>& dec,
		   casa::Array<double>& antPos,
		   casa::Array<casa::String>& antNames,
		   bool& writeAutoCorr);

  private:
    // Forbid assignment.
    DH_MSMake& operator= (const DH_MSMake&);

    int*    itsNBand;
    int*    itsNFreq;
    int*    itsNTime;
    int*    itsTileSizeFreq;
    int*    itsTileSizeRest;
    double* itsFreqs;
    double* itsTimes;
  };


  struct MSMakeConn
  {
    MSMakeConn (int nslave);
    ~MSMakeConn();
    DH_MSMake sender;
    DH_MSMake receiver;
#ifdef HAVE_MPI
    std::vector<TH_MPI*> ths;
#endif
    std::vector<Connection*> conns;
  };
}

#endif
