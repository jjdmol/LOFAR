//# This file was generated by genDBcode v2.0 on Wed Jan 14 16:15:23 CET 2004
//# with the command: genDBcode DH_PL_MessageRecord.map DH_PL_MessageRecord.fun P 
//# from the directory: /home/tanaka/prj/co-0351-1621/LOFAR/CEP/CEPFrame/src/genDH_PL_MessageRecord
//#
//# EDITING THIS FILE MANUALLY IS AT YOUR OWN RISK
//# IT MIGHT BE OVERWRITTEN BY THE NEXT MAKE OF YOUR PROJECT
//#
#include "PO_DH_PL_MessageRecord.h"
#include "LCS_base.h"
#include <PL/Collection.h>
#include <PL/DTLHelperClasses.h>
#include <PL/Query.h>
#include <PL/TPersistentObject.h>

using namespace dtl;

namespace LOFAR {
	namespace PL {

// The BCA<DH_PL_MessageRecord> structure 'binds' the database columns
// to the members of the DBRep<DH_PL_MessageRecord> class.
template<>
void BCA<DH_PL_MessageRecord>::operator()(BoundIOs& cols, DataObj& rowbuf) {
	cols["ObjID"]		== rowbuf.Oid;
	cols["Owner"]	    == rowbuf.Owner;
	cols["VersionNr"]	== rowbuf.VersionNr;
	cols["APPID"]	== rowbuf.AppId;
	cols["TAG"]	== rowbuf.Tag;
	cols["SEQNO"]	== rowbuf.SeqNo;
	cols["STATUS"]	== rowbuf.Status;
	cols["SIZE"]	== rowbuf.Size;
	cols["TIMESTAMP"]	== rowbuf.TimeStamp;
	cols["TYPE"]	== rowbuf.Type;
	cols["NAME"]	== rowbuf.Name;
	cols["BLOB"]	== rowbuf.Blob;
}


// toDatabaseRep copies the fields of the persistency layer
// and of the DH_PL_MessageRecord class to the given DBRep<DH_PL_MessageRecord> structure
template<>
void TPersistentObject<DH_PL_MessageRecord>::toDatabaseRep(DBRep<DH_PL_MessageRecord>& dest) const
{
	// copy info of the DH_PL_MessageRecord to the DBRep<DH_PL_MessageRecord> class
	// First copy the PO part
	dest.Oid		= metaData().oid()->get();
	dest.Owner		= metaData().ownerOid()->get();
	dest.VersionNr	= metaData().versionNr();

	// Finally copy the info from DH_PL_MessageRecord
	dest.AppId	= itsObjectPtr->AppId;
	dest.Tag	= itsObjectPtr->Tag;
	dest.SeqNo	= itsObjectPtr->SeqNo;
	dest.Status	= itsObjectPtr->Status;
	dest.Size	= itsObjectPtr->Size;
	dest.TimeStamp	= itsObjectPtr->TimeStamp;
	dest.Type	= itsObjectPtr->Type;
	dest.Name	= itsObjectPtr->Name;
	dest.Blob	= itsObjectPtr->Blob;
}


// fromDatabaseRep copies the fields of the DBRep<DH_PL_MessageRecord> structure
// to the persistency layer and the DH_PL_MessageRecord class.
template<>
void TPersistentObject<DH_PL_MessageRecord>::fromDatabaseRep(const DBRep<DH_PL_MessageRecord>& org)
{
	// copy info of the DH_PL_MessageRecord to the DBRep<DH_PL_MessageRecord> class
	// First copy the metadata of the PO
	metaData().oid()->set(org.Oid);
	metaData().ownerOid()->set(org.Owner);
	metaData().versionNr() = org.VersionNr;

	// Finally copy the info from DH_PL_MessageRecord
	itsObjectPtr->AppId	= org.AppId;
	itsObjectPtr->Tag	= org.Tag;
	itsObjectPtr->SeqNo	= org.SeqNo;
	itsObjectPtr->Status	= org.Status;
	itsObjectPtr->Size	= org.Size;
	itsObjectPtr->TimeStamp	= org.TimeStamp;
	itsObjectPtr->Type	= org.Type;
	itsObjectPtr->Name	= org.Name;
	itsObjectPtr->Blob	= org.Blob;
}


// Initialize the internals of TPersistentObject<DH_PL_MessageRecord>
template<>
void TPersistentObject<DH_PL_MessageRecord>::init()
{
}

//
// Routine for insert this DH_PL_MessageRecord object in the database.
//
template<>
void TPersistentObject<DH_PL_MessageRecord>::doInsert() const
{
	typedef	DBView<DBRep<DH_PL_MessageRecord> >	DBViewType;
	DBViewType	insView("DH_PL_MessageRecord", BCA<DH_PL_MessageRecord>());
	DBViewType::insert_iterator	insIter = insView;

	// copy info of the DH_PL_MessageRecord to the DBRep<DH_PL_MessageRecord> class
	DBRep<DH_PL_MessageRecord>		rec;
	toDatabaseRep 	(rec);

	// save this record
	*insIter = rec;

	metaData().versionNr()++;
}

//
// Routine for updating this DH_PL_MessageRecord object in the database.
//
template<>
void TPersistentObject<DH_PL_MessageRecord>::doUpdate() const
{
	typedef DBView<DBRep<DH_PL_MessageRecord>, DBRep<ObjectId> > DBViewType;
	DBViewType	updView("DH_PL_MessageRecord", BCA<DH_PL_MessageRecord>(),
			"WHERE ObjId=(?)", BPA<ObjectId>());
	DBViewType::update_iterator 	updIter = updView;

	// copy info of the DH_PL_MessageRecord to the DBRep<DH_PL_MessageRecord> class
	DBRep<DH_PL_MessageRecord>		rec;
	toDatabaseRep 	(rec);

	// setup the selection parameters
	updIter.Params().itsOid = rec.Oid;

	// save this record
	*updIter = rec;

	metaData().versionNr()++;
}

//
// Routine for deleting this DH_PL_MessageRecord object in the database.
//
template<>
void TPersistentObject<DH_PL_MessageRecord>::doErase() const
{
	typedef DBView<DBRep<ObjectId> > DBViewType;
	DBViewType	delView("DH_PL_MessageRecord", BCA<ObjectId>());
	DBViewType::delete_iterator		delIter = delView;

	// setup the selection parameters
	DBRep<ObjectId>		rec;
	rec.itsOid = metaData().oid()->get();

	// delete this record
	*delIter = rec;

	metaData().reset();
}

//
// Routine to retrieve this DH_PL_MessageRecord object from the database.
//
template<>
Collection<TPersistentObject<DH_PL_MessageRecord> > 
TPersistentObject<DH_PL_MessageRecord>::retrieve(const Query&	query, int maxObjects)
{
	std::cout << "retrieve DH_PL_MessageRecord" << std::endl;
	typedef DBView<DBRep<DH_PL_MessageRecord> >  DBViewType;
	DBViewType 	selView("DH_PL_MessageRecord", BCA<DH_PL_MessageRecord>(), query.getSql());
	DBViewType::select_iterator	selIter = selView.begin();
	Collection<TPersistentObject<DH_PL_MessageRecord> > 	selResult;

	for (int nrRecs = 0; selIter != selView.end() && nrRecs < maxObjects; ++selIter, ++nrRecs) {
		TPersistentObject<DH_PL_MessageRecord>		TPODH_PL_MessageRecord;
		TPODH_PL_MessageRecord.fromDatabaseRep(*selIter);
		TPODH_PL_MessageRecord.retrieve();		// refresh object, should be changed
		selResult.add(TPODH_PL_MessageRecord);
	}
		// @@@ TO BE DEFINED @@@

	return (selResult);
}

//
// Routine to retrieve this DH_PL_MessageRecord object from the database.
//
template<>
void TPersistentObject<DH_PL_MessageRecord>::doRetrieve(const ObjectId&	aOid,
												 bool isOwnerOid)
{
	std::string		whereClause;
	if (isOwnerOid) {	whereClause = "WHERE Owner = (?)";  }
			else {	whereClause = "WHERE ObjId=(?)"; }

	typedef DBView<DBRep<DH_PL_MessageRecord>, DBRep<ObjectId> >  DBViewType;
	DBViewType 	selView("DH_PL_MessageRecord", BCA<DH_PL_MessageRecord>(), whereClause, BPA<ObjectId>());
	DBViewType::select_iterator	selIter = selView.begin();

	selIter.Params().itsOid = aOid.get();

	// Should we throw an exception if there are no matching records?
	// Let's do it for the time being; that's easier for debugging.
	if (selIter != selView.end()) {
		fromDatabaseRep(*selIter);
	}
	else {
		THROW (PLException, "No matching records found!");
	}
}

template class TPersistentObject<DH_PL_MessageRecord>;

	} // close namespace PL
}	// close namespace LOFAR

