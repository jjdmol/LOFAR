/*
 * qlofardatamodel.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jun 5, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/qlofardatamodel.h $
 *
 */

#ifndef QLOFARDATAMODEL_H_
#define QLOFARDATAMODEL_H_

#include <vector>
#include "lofar_scheduler.h"
#include "qstandarditemmodel.h"

// define the user roles for the model which are used to store and retrieve user data in the model
#define USERDATA_ROLE	35

class QLofarDataModel : public QStandardItemModel {

	Q_OBJECT

public:

	QLofarDataModel(QObject *parent);
	QLofarDataModel(int rows, int columns, QObject *parent = 0 /*, SchedulerGUI * gui = 0 */ );

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual ~QLofarDataModel();

	void setErrorCells(const errorTasksMap & errorTasks);
	void clearErrorCell(unsigned int taskID, data_headers header);
	void addErrorIndex(const QModelIndex &index);
	bool isErrorIndex(const QModelIndex &index) const;
	void clearErrorIndex(const QModelIndex &index);
	void clearErrorIndices(void) {errorIndices.clear();}

	int findTaskRow(unsigned taskID) const;

//	bool setData ( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

private:
//	SchedulerGUI * parentGUI;
	std::vector<QModelIndex> errorIndices;

};

#endif /* QLOFARDATAMODEL_H_ */




