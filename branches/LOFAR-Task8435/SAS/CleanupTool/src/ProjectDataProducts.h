/*
 * ProjectDataProducts.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 8-dec-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/ProjectDataProducts.h $
 *
 */

#ifndef PROJECTDATAPRODUCTS_H_
#define PROJECTDATAPRODUCTS_H_

#include <map>
#include <QString>
#include "TaskDataProducts.h"

class ProjectDataProducts {
public:
	ProjectDataProducts();
	virtual ~ProjectDataProducts();

	friend QDataStream& operator<< (QDataStream &out, const ProjectDataProducts &); // used for writing data to binary file
	friend QDataStream& operator>> (QDataStream &in, ProjectDataProducts &); // used for reading data from binary file

	const QString &name(void) const {return itsProjectName;}
	void setName(const QString &name) {itsProjectName = name;}
	const std::map<unsigned, TaskDataProducts> &taskDataProducts(void) const {return itsTaskDataProducts;}
	std::map<unsigned, TaskDataProducts> &taskDataProductsForChange(void) {return itsTaskDataProducts;}
	quint64 totalSizeOnDisk(void) const; // gets the total size for this task on disk in MB
	quint64 totalSizeOfDataProductType(dataProductTypes type) const;

	TaskDataProducts &updateTaskData(unsigned sasTreeID) {return itsTaskDataProducts[sasTreeID];}

private:
	QString itsProjectName;
	// itsTaskDataProducts map, key = the task's SAS tree ID, value = the task's data products
	std::map<unsigned, TaskDataProducts> itsTaskDataProducts;
};

#endif /* PROJECTDATAPRODUCTS_H_ */
