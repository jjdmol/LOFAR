/*
 * ProjectDataProducts.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 8-dec-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/ProjectDataProducts.cpp $
 *
 */

#include "ProjectDataProducts.h"

ProjectDataProducts::ProjectDataProducts() {
	// TODO Auto-generated constructor stub

}

ProjectDataProducts::~ProjectDataProducts() {
	// TODO Auto-generated destructor stub
}

QDataStream& operator<< (QDataStream &out, const ProjectDataProducts &pdata) {
	if (out.status() == QDataStream::Ok) {
		out << pdata.itsProjectName;
		out << (quint32)pdata.itsTaskDataProducts.size();
		for (std::map<unsigned, TaskDataProducts>::const_iterator tit = pdata.itsTaskDataProducts.begin(); tit != pdata.itsTaskDataProducts.end(); ++tit) {
			out << tit->first << tit->second;
		}
	}
	return out;
}

QDataStream& operator>> (QDataStream &in, ProjectDataProducts &pdata) {
	pdata.itsTaskDataProducts.clear();
	if (in.status() == QDataStream::Ok) {
		quint32 size;
		unsigned sasid;
		QString projectName;
		in >> pdata.itsProjectName;
		TaskDataProducts taskDataProduct;
		in >> size;
		while (size--) {
			in >> sasid >> taskDataProduct;
			pdata.itsTaskDataProducts[sasid] = taskDataProduct;
		}
	}
	return in;
}

quint64 ProjectDataProducts::totalSizeOnDisk(void) const {
	quint64 totalSize(0);
	for (std::map<unsigned, TaskDataProducts>::const_iterator it = itsTaskDataProducts.begin(); it != itsTaskDataProducts.end(); ++it) {
		totalSize += it->second.totalSizeOnDisk();
	}
	return totalSize;
}

quint64 ProjectDataProducts::totalSizeOfDataProductType(dataProductTypes type) const {
	quint64 totalSize(0);
	for (std::map<unsigned, TaskDataProducts>::const_iterator it = itsTaskDataProducts.begin(); it != itsTaskDataProducts.end(); ++it) {
		totalSize += it->second.sizeOfDataProduct(type);
	}
	return totalSize;
}

