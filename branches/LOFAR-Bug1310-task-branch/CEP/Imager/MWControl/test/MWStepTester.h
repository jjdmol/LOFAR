//# MWStepTester.h: Test class for the MWStep framework
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_MWCONTROL_MWSTEPTESTER_H
#define LOFAR_MWCONTROL_MWSTEPTESTER_H

#include <MWCommon/MWStepVisitor.h>
#include <MWCommon/MWStep.h>
#include <Blob/BlobOStream.h>


namespace LOFAR { namespace CEP {

  class MWStepTester: public MWStepVisitor
  {
  public:
    MWStepTester (int streamId, LOFAR::BlobOStream* out);

    virtual ~MWStepTester();

    // Get the resulting operation.
    int getResultOperation() const
      { return itsOperation; }

  private:
    // Process the various MWStep types.
    //@{
    virtual void visitGlobal (const MWGlobalStep&);
    virtual void visitLocal  (const MWLocalStep&);
    //@}

    // Write the boolean result in the output stream buffer.
    void writeResult (bool result);

    //# Data members.
    int                 itsStreamId;
    int                 itsOperation;
    LOFAR::BlobOStream* itsOut;
  };

}} // end namespaces

#endif
