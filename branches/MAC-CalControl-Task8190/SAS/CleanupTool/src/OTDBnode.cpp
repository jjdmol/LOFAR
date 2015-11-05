/*
 * OTDBnode.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 12-feb-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/OTDBnode.cpp $
 *
 */

#include "OTDBnode.h"
#include "lofar_utils.h"
#include <QSqlRecord>
#include <QVariant>
#include <iostream>
using std::endl;

OTDBnode::OTDBnode(int treeID, const QSqlQuery &query) :
	itsTreeID(treeID)
{
	// nodeid
	itsNodeID = query.value(query.record().indexOf("nodeid")).toInt();
	// parentid
	itsParentID = query.value(query.record().indexOf("parentid")).toInt();
	// paramdefid
	itsParamDefID = query.value(query.record().indexOf("paramdefid")).toInt();
	// name
	itsName = query.value(query.record().indexOf("name")).toString().toStdString();
	// index
	itsIndex = query.value(query.record().indexOf("index")).toInt();
	// leaf
	itsIsLeaf = query.value(query.record().indexOf("leaf")).toBool();
	// instances
	itsInstances = query.value(query.record().indexOf("instances")).toInt();
	// limits
	itsLimits = query.value(query.record().indexOf("limits")).toString().toStdString();
	// description
	itsDescription = query.value(query.record().indexOf("description")).toString().toStdString();
}

//
// print(ostream&): os&
//
// print Tree charateristics.
std::ostream& OTDBnode::print (std::ostream& os) const
{
	os << "treeID    : " << itsTreeID << endl
	<< "nodeID    : " << itsNodeID << endl
	<< "parentID  : " << itsParentID << endl
	<< "paramdefID: " << itsParamDefID << endl
	<< "name      : " << itsName << endl
	<< "index     : " << itsIndex << endl
	<< "leaf      : " << ((itsIsLeaf) ? "T" : "F") << endl
	<< "instances : " << itsInstances << endl
	<< "limits    : " << itsLimits << endl
	<< "descr.    : " << itsDescription << endl;

	return (os);
}

std::ostream& operator<< (std::ostream &out, const OTDBnode &node) {
	write_primitive<int>(out, node.itsTreeID);
	write_primitive<int>(out, node.itsNodeID);
	write_primitive<int>(out, node.itsParentID);
	write_primitive<int>(out, node.itsParamDefID);
	write_string(out, node.itsName);
	write_primitive<int>(out, node.itsIndex);
	write_primitive<bool>(out, node.itsIsLeaf);
	write_primitive<int>(out, node.itsInstances);
	write_string(out, node.itsLimits);
	write_string(out, node.itsDescription);
	return out;
}

std::istream& operator>> (std::istream &in, OTDBnode &node) {
	read_primitive<int>(in, node.itsTreeID);
	read_primitive<int>(in, node.itsNodeID);
	read_primitive<int>(in, node.itsParentID);
	read_primitive<int>(in, node.itsParamDefID);
	read_string(in, node.itsName);
	read_primitive<int>(in, node.itsIndex);
	read_primitive<bool>(in, node.itsIsLeaf);
	read_primitive<int>(in, node.itsInstances);
	read_string(in, node.itsLimits);
	read_string(in, node.itsDescription);
	return in;
}
