/*
 * TaskDataProducts.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 8-dec-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/TaskDataProducts.cpp $
 *
 */

#include "TaskDataProducts.h"

TaskDataProducts::TaskDataProducts() {
	// TODO Auto-generated constructor stub

}

TaskDataProducts::~TaskDataProducts() {
	// TODO Auto-generated destructor stub
}

QDataStream& operator<< (QDataStream &out, const TaskDataProducts &pdata) {
	if (out.status() == QDataStream::Ok) {
		out << pdata.itsTaskName << pdata.itsSASDBName << pdata.itsSASTree;
		out << (quint32)pdata.itsDataProducts.size();
		for (std::map<dataProductTypes, DataProductInfo>::const_iterator dpit = pdata.itsDataProducts.begin(); dpit != pdata.itsDataProducts.end(); ++dpit) {
			out << dpit->first << dpit->second;
		}
	}
	return out;
}

QDataStream& operator>> (QDataStream &in, TaskDataProducts &pdata) {
	pdata.itsDataProducts.clear();
	if (in.status() == QDataStream::Ok) {
		quint32 size;
		in >> pdata.itsTaskName >> pdata.itsSASDBName >> pdata.itsSASTree;

		int dpTypeInt;
		DataProductInfo dataProductInfo;
		in >> size;
		while (size--) {
			in >> dpTypeInt >> dataProductInfo;
			pdata.itsDataProducts[(dataProductTypes)dpTypeInt] = dataProductInfo;
		}
	}
	return in;
}

quint64 TaskDataProducts::totalSizeOnDisk(void) const {
	quint64 totalSize(0); // in MB
	for (std::map<dataProductTypes, DataProductInfo>::const_iterator it = itsDataProducts.begin(); it != itsDataProducts.end(); ++it) {
		totalSize += it->second.totalSize;
	}
	return totalSize;
}

quint64 TaskDataProducts::sizeOfDataProduct(dataProductTypes type) const {
	std::map<dataProductTypes, DataProductInfo>::const_iterator it = itsDataProducts.find(type);
	if (it != itsDataProducts.end()){
		return it->second.totalSize;
	}
	return 0;
}
