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

#include <PL/TPersistentObject.h>
#include <PL/DTLBase.h>
#include <MNS/MeqParmHolder.h>

namespace LOFAR {
  namespace PL {

    class MetaDataRep
    {
    public:
      MetaDataRep()
        {}
      void metaBindCols (dtl::BoundIOs& cols);
      void metaToRep (const PersistentObject&);
      void metaFromRep (PersistentObject&) const;

      LOFAR::PL::ObjectId::oid_t  itsOid;
      LOFAR::PL::ObjectId::oid_t  itsOwnerOid;
      unsigned int     itsVersionNr;
    };

    // The DBRep<MeqParmHolder> structure is a compilation of the fields
    // of the MeqParmHolder class and the persistency layer. It contains
    // all fields that should be stored to the database
    class DBRep<MeqParmDefHolder>
    {
    public:
      struct Rep : public MetaDataRep
      {
	Rep()
	  {}
	void bindCols (dtl::BoundIOs& cols);
	void toRep (const MeqParmDefHolder&);
	void fromRep (MeqParmDefHolder&) const;

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

      DBRep()
        {}

      Rep& rep()
        { return itsRep; }

      const Rep& rep() const
        { return itsRep; }

      LOFAR::PL::ObjectId::oid_t getOid() const
        { return itsRep.itsOid; }

    private:
      Rep itsRep;
    };


    class DBRep<MeqParmHolder>
    {
    public:
      struct Rep : public DBRep<MeqParmDefHolder>::Rep
      {
	Rep()
	  {}
	void bindCols (dtl::BoundIOs& cols);
	void toRep (const MeqParmHolder&);
	void fromRep (MeqParmHolder&) const;

	double           itsStartTime;
	double           itsEndTime;
	double           itsStartFreq;
	double           itsEndFreq;
      };

      DBRep()
        {}

      Rep& rep()
        { return itsRep; }

      const Rep& rep() const
        { return itsRep; }

      LOFAR::PL::ObjectId::oid_t getOid() const
        { return itsRep.itsOid; }

    private:
      Rep itsRep;
    };


    // The BCA<MeqParmDefHolder> structure 'binds' the database columns
    // to the members of the DBRep<MeqParmDefHolder> class.
    template<>
    inline void BCA<MeqParmDefHolder>::operator() (dtl::BoundIOs& cols,
						   DataObj& rowbuf)
      { rowbuf.rep().metaBindCols(cols); rowbuf.rep().bindCols(cols); }

    // toDatabaseRep copies the fields of the persistency layer
    // and of the MeqParmDefHolder class to the given DBRep<MeqParmDefHolder>
    // structure
    template<>
    inline void TPersistentObject<MeqParmDefHolder>::toDatabaseRep
                                         (DBRep<MeqParmDefHolder>& dest) const
      { dest.rep().metaToRep(*this); dest.rep().toRep(*itsObjectPtr); }

    // fromDatabaseRep copies the fields of the DBRep<MeqParmDefHolder> struct
    // to the persistency layer and the MeqParmDefHolder class.
    template<>
    inline void TPersistentObject<MeqParmDefHolder>::fromDatabaseRep
                                         (const DBRep<MeqParmDefHolder>& org)
      { org.rep().metaFromRep(*this); org.rep().fromRep(*itsObjectPtr); }


    // Initialize the internals of TPersistentObject<MeqParmDefHolder>
    template<>
    void TPersistentObject<MeqParmDefHolder>::init();

    // Initialize the attribute map of TPersistentObject<MeqParmDefHolder>
    template<>
    void TPersistentObject<MeqParmDefHolder>::initAttribMap();

    // The BCA<MeqParmHolder> structure 'binds' the database columns
    // to the members of the DBRep<MeqParmHolder> class.
    template<>
    inline void BCA<MeqParmHolder>::operator() (dtl::BoundIOs& cols,
						DataObj& rowbuf)
      { rowbuf.rep().metaBindCols(cols); rowbuf.rep().bindCols(cols); }

    // toDatabaseRep copies the fields of the persistency layer
    // and of the MeqParmHolder class to the given DBRep<MeqParmHolder>
    // structure
    template<>
    inline void TPersistentObject<MeqParmHolder>::toDatabaseRep
                                         (DBRep<MeqParmHolder>& dest) const
      { dest.rep().metaToRep(*this); dest.rep().toRep(*itsObjectPtr); }

    // fromDatabaseRep copies the fields of the DBRep<MeqParmHolder> struct
    // to the persistency layer and the MeqParmHolder class.
    template<>
    inline void TPersistentObject<MeqParmHolder>::fromDatabaseRep
                                         (const DBRep<MeqParmHolder>& org)
      { org.rep().metaFromRep(*this); org.rep().fromRep(*itsObjectPtr); }


    // Initialize the internals of TPersistentObject<MeqParmHolder>
    template<>
    void TPersistentObject<MeqParmHolder>::init();

    // Initialize the attribute map of TPersistentObject<MeqParmHolder>
    template<>
    void TPersistentObject<MeqParmHolder>::initAttribMap();

  } // end namespace PL
}   // end namespace LOFAR


#endif
