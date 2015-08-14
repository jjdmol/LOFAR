/*
 * TaskDataProducts.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 8-dec-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/TaskDataProducts.h $
 *
 */

#ifndef TASKDATAPRODUCTS_H_
#define TASKDATAPRODUCTS_H_

#include <QDate>
#include <QStringList>
#include "OTDBtree.h"
#include <map>
#include <numeric> // for std::accumulate on vector

class DataProductInfo {
public:
	friend QDataStream& operator<< (QDataStream &out, const DataProductInfo &dpInfo) {
		if (out.status() == QDataStream::Ok) {
			out << dpInfo.parentid << dpInfo.deleted << dpInfo.expired << dpInfo.retentionTime << dpInfo.expiryDate
			    /*<< dpInfo. uniqueNodes*/ << dpInfo.locations << dpInfo.nodes << dpInfo.fileNames << dpInfo.localPaths;
		}
		out << (quint32) dpInfo.fileSizes.size();
		for (std::vector<quint64>::const_iterator fit = dpInfo.fileSizes.begin(); fit != dpInfo.fileSizes.end(); ++fit) {
			out << *fit;
		}
		out << (quint32) dpInfo.exists.size();
		for (std::vector<bool>::const_iterator fit = dpInfo.exists.begin(); fit != dpInfo.exists.end(); ++fit) {
			out << *fit;
		}

		return out;
	}
	friend QDataStream& operator>> (QDataStream &in, DataProductInfo &dpInfo) {
		dpInfo.fileSizes.clear();
		dpInfo.exists.clear();
		if (in.status() == QDataStream::Ok) {
			in >> dpInfo.parentid >> dpInfo.deleted >> dpInfo.expired >> dpInfo.retentionTime >> dpInfo.expiryDate
			   /*>> dpInfo.uniqueNodes*/ >> dpInfo.locations >> dpInfo.nodes >> dpInfo.fileNames >> dpInfo.localPaths;
		}
		quint32 size;
		quint64 fsize;
		bool exist;
		in >> size;
		while (size--) {
			in >> fsize;
			dpInfo.fileSizes.push_back(fsize);
		}
		in >> size;
		while (size--) {
			in >> exist;
			dpInfo.exists.push_back(exist);
		}

		return in;
	}

	void calculateTotalSize(void) {
		totalSize = 0;
		for (size_t i = 0; i < fileSizes.size(); ++i) {
			if (exists.at(i)) {
				totalSize += fileSizes.at(i);
			}
		}
	}

public:
	int parentid;
	bool deleted, expired;
	int retentionTime;
	QDate expiryDate;
//	QStringList uniqueNodes; // only contains the unique node names
	QString locations;
	QStringList nodes;
	QStringList fileNames;
	std::vector<quint64> fileSizes;
	quint64 totalSize;
	std::vector<bool> exists;
	QStringList localPaths; // path is the same for every node, e.g. '\data\L12345\'
};

class TaskDataProducts {
public:
	TaskDataProducts();
	virtual ~TaskDataProducts();

	friend QDataStream& operator<< (QDataStream &out, const TaskDataProducts &pdata);
	friend QDataStream& operator>> (QDataStream &in, TaskDataProducts &pdata);
	friend class ProjectData;

	const OTDBtree &SAStree(void) const {return itsSASTree;}
	const QString &taskName(void) const {return itsTaskName;}
	const QString &SASDBName(void) const {return itsSASDBName;}
	const std::map<dataProductTypes, DataProductInfo> &dataProducts(void) const {return itsDataProducts;}
	std::map<dataProductTypes, DataProductInfo> &dataProductsForChange(void) {return itsDataProducts;}
	quint64 totalSizeOnDisk(void) const; // gets the total size for this task on disk in kB
	quint64 sizeOfDataProduct(dataProductTypes type) const;

	void setTaskName(const QString &taskName) {itsTaskName = taskName;}
	void setSASTree(const OTDBtree &tree) {itsSASTree = tree;}
	void setSASDBName(const QString &dbName) {itsSASDBName = dbName;}

	DataProductInfo &addDataProduct(dataProductTypes type) {return itsDataProducts[type];}

private:
	QString itsTaskName, itsSASDBName;
	std::map<dataProductTypes, DataProductInfo> itsDataProducts;
	OTDBtree itsSASTree;
};

#endif /* TASKDATAPRODUCTS_H_ */
