//# DH_PL_PO.h: Persistency object for DH_PL
//#
//# Copyright (C) 2004
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

#ifndef LIBTRANSPORT_PO_DH_PL_H
#define LIBTRANSPORT_PO_DH_PL_H

#include <lofar_config.h>

#include <PL/TPersistentObject.h>
#include <PL/DBRep.h>
#include <libTransport/DH_PL.h>

namespace LOFAR {
  namespace PL {

    class DBRep<DH_PL>
    {
    public:
      void bindCols (dtl::BoundIOs& cols);
      void toDBRep (const DH_PL&);
      void fromDBRep (DH_PL&) const;

    private:
      long      itsSeqNr;
      int       itsId;
      int       itsTag;
      dtl::blob itsData;
    };



    // Copy the fields from the DH_PL object to the DBRep<DH_PL> structure.
    template<typename DH_T>
    inline void TPersistentObject<DH_T>::toDBRep
                                  (DBRep<DH_T>& dest) const
      { dest.toDBRep (data()); }

    // Copy the fields from the DBRep<DH_PL> structure to the DH_PL object.
    template<typename DH_T>
    inline void TPersistentObject<DH_T>::fromDBRep
                                  (const DBRep<DH_T>& src)
      { src.fromDBRep (data()); }

    // Initialize the internals of TPersistentObject<DH_PL>
    template<typename DH_T>
    void TPersistentObject<DH_T>::init()
    {}

    // Initialize the attribute map of TPersistentObject<DH_PL>
    template<typename DH_T>
    void TPersistentObject<DH_T>::initAttribMap()
    {}

  } // end namespace PL
}   // end namespace LOFAR


#endif
