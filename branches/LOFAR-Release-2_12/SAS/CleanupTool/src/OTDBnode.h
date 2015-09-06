/*
 * OTDBnode.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 12-feb-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/OTDBnode.h $
 *
 * An OTDBnode describes a single item/element of the OTDB. An item can be a node or a parameter.
 * Note: it does NOT contain the value of the item.
 *
 */

#ifndef OTDBnode_H_
#define OTDBnode_H_

#include <ostream>
#include <istream>
#include <QSqlQuery>

//const char * SAS_node_names [9] = {"nodeid", "parentid", "paramdefid", "name", "index", "leaf", "instances", "limits", "description"};

class OTDBnode {
public:
	OTDBnode() : itsTreeID(0), itsNodeID(0), itsParentID(0), itsParamDefID(0),
	itsIndex(0), itsIsLeaf(false), itsInstances(0) {};
	OTDBnode(int treeID, const QSqlQuery &query);
	~OTDBnode() {};

	int	treeID(void) const	{ return itsTreeID; }
	int nodeID(void) const { return itsNodeID; }
	int	parentID(void) const { return itsParentID; }
	int	paramDefID(void) const { return itsParamDefID; }
	int instances(void) const { return itsInstances; }
	const std::string &name(void) const {return itsName;}
	const std::string &limits(void) const { return itsLimits; }
	const std::string &description(void) { return itsDescription; }

	friend std::ostream& operator<< (std::ostream &out, const OTDBnode &OTDBnode); // used for writing data to binary file
	friend std::istream& operator>> (std::istream &in, OTDBnode &OTDBnode); // used for reading data from binary file

	std::ostream& print (std::ostream& os) const;

	friend class SASConnection; // SASConnection may change or init a OTDBnode

private:
	OTDBnode(int aTreeID, int aNodeID, int aParentID, int aParamDefID) :
		itsTreeID(aTreeID), itsNodeID(aNodeID), itsParentID(aParentID), itsParamDefID(aParamDefID) {};

private:
	int	itsTreeID;
	int	itsNodeID;
	int itsParentID;
	int	itsParamDefID;
	std::string	itsName;
	int	itsIndex;
	bool itsIsLeaf;
	int	itsInstances;
	std::string	itsLimits;
	std::string	itsDescription;
};

#endif /* OTDBnode_H_ */
