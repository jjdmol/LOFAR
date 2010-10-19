//# TPOParm.h: Persistency object for MeqParmHolder
//#
//# Copyright (C) 2003
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

#ifndef MNS_TPOPARM_H
#define MNS_TPOPARM_H

#include <lofar_config.h>
#ifdef HAVE_DTL

#include <PL/TPersistentObject.h>
#include <PL/DBRep.h>
#include <MNS/MeqParmHolder.h>

namespace LOFAR {
  namespace PL {

    // The DBRep<MeqParmHolder> structure is a compilation of the fields
    // of the MeqParmHolder class and the persistency layer. It contains
    // all fields that should be stored to the database
    class DBRep<MeqParmDefHolder>
    {
    public:
      void bindCols (dtl::BoundIOs& cols);
      void toDBRep (const MeqParmDefHolder&);
      void fromDBRep (MeqParmDefHolder&) const;

    private:
      std::string itsName;
      int         itsSrcNr;
      int         itsStatNr;
      double      itsCoeff00;
      dtl::blob   itsCoeff;
      dtl::blob   itsSimCoeff;
      dtl::blob   itsPertSimCoeff;
      double      itsTime0;
      double      itsFreq0;
      bool        itsNormalized;
      dtl::blob   itsMask;
      double      itsDiff;
      bool        itsDiffRel;
    };


    class DBRep<MeqParmHolder> : public DBRep<MeqParmDefHolder>
    {
    public:
      void bindCols (dtl::BoundIOs& cols);
      void toDBRep (const MeqParmHolder&);
      void fromDBRep (MeqParmHolder&) const;

    private:
      double           itsStartTime;
      double           itsEndTime;
      double           itsStartFreq;
      double           itsEndFreq;
    };



    // Copy the fields from the A object to the DBRep<A> structure.
    template<>
    inline void TPersistentObject<MeqParmDefHolder>::toDBRep
                                  (DBRep<MeqParmDefHolder>& dest) const
      { dest.toDBRep (data()); }

    // Copy the fields from the DBRep<A> structure to the A object.
    template<>
    inline void TPersistentObject<MeqParmDefHolder>::fromDBRep
                                  (const DBRep<MeqParmDefHolder>& src)
      { src.fromDBRep (data()); }

    // Initialize the internals of TPersistentObject<MeqParmDefHolder>
    template<>
    void TPersistentObject<MeqParmDefHolder>::init();

    // Initialize the attribute map of TPersistentObject<MeqParmDefHolder>
    template<>
    void TPersistentObject<MeqParmDefHolder>::initAttribMap();


    // Copy the fields from the A object to the DBRep<A> structure.
    template<>
    inline void TPersistentObject<MeqParmHolder>::toDBRep
                                  (DBRep<MeqParmHolder>& dest) const
      { dest.toDBRep (data()); }

    // Copy the fields from the DBRep<A> structure to the A object.
    template<>
    inline void TPersistentObject<MeqParmHolder>::fromDBRep
                                  (const DBRep<MeqParmHolder>& src)
      { src.fromDBRep (data()); }

    // Initialize the internals of TPersistentObject<MeqParmHolder>
    template<>
    void TPersistentObject<MeqParmHolder>::init();

    // Initialize the attribute map of TPersistentObject<MeqParmHolder>
    template<>
    void TPersistentObject<MeqParmHolder>::initAttribMap();

  } // end namespace PL
}   // end namespace LOFAR


#endif

#endif
