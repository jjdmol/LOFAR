/*
 * scheduletabledelegate.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Apr 2, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/scheduletabledelegate.h $
 *
 */

#ifndef SCHEDULETABLEDELEGATE_H_
#define SCHEDULETABLEDELEGATE_H_

#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QSpinBox>
#include "lofar_scheduler.h"

class ScheduleTableDelegate : public QItemDelegate
{
	Q_OBJECT

public:
	ScheduleTableDelegate(QObject *parent = 0);
	virtual ~ScheduleTableDelegate();

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
			const QModelIndex &index) const;

	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

	void updateEditorGeometry(QWidget *editor,

	const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
	void drawText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &) const;
	void writeTime(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	void writeStations(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;


signals:
//	void tableItemChanged(const QModelIndex &) const;
//	void tableItemAboutToBeChanged(const QModelIndex &) const;
	void tableItemNotChanged(void) const;
	void tableItemChanged(unsigned, data_headers, const QVariant &, const QModelIndex &) const;
	void openEditor(const QModelIndex &, const QVariant &) const;
};

#endif /* SCHEDULETABLEDELEGATE_H_ */
