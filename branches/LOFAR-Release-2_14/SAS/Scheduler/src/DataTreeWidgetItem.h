/*
 * qtreewidget.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Aug 23, 2012
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DataTreeWidgetItem.h $
 *
 */


#ifndef DATATREEWIDGETITEM_H_
#define DATATREEWIDGETITEM_H_

#include <qtreewidget.h>
#include "lofar_utils.h"

class DataTreeWidgetItem: public QTreeWidgetItem {
public:
	DataTreeWidgetItem(QTreeWidgetItem *base = 0);
	DataTreeWidgetItem(QTreeWidget *parent, const QStringList &str) : QTreeWidgetItem(parent, str) {}
	DataTreeWidgetItem(QTreeWidgetItem *parent, const QStringList &str) : QTreeWidgetItem(parent, str) {}

	void setDataValue(quint64 value) {
		setData(0,Qt::UserRole,value);
		itsHumanReadableValue = humanReadableUnits(value, SIZE_UNITS).c_str();
	}
	virtual ~DataTreeWidgetItem();

	quint64 dataValue(void) const {return data(0,Qt::UserRole).toULongLong();}
	QString humanReadible(void) const {return itsHumanReadableValue;}

private:
	bool operator<(const QTreeWidgetItem &other) const {
	     int column = treeWidget()->sortColumn();
	     if (column == 1) {
	    	 return data(1,Qt::UserRole).toUInt() < other.data(1,Qt::UserRole).toUInt();
	     }
	     else if (column == 10) {
	    	 return data(0,Qt::UserRole).toULongLong() < other.data(0,Qt::UserRole).toULongLong();
	     }
	     else return (QTreeWidgetItem::operator <(other));
    }

private:
    QString itsHumanReadableValue;
};

#endif /* DATATREEWIDGETITEM_H_ */
