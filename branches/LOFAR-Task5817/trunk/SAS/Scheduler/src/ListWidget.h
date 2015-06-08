/*
 * ListWidget.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 13-feb-2013
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/ListWidget.h $
 *
 */

#include <QListWidget>
#include <QString>
#include <QStringList>

#ifndef LISTWIDGET_H_
#define LISTWIDGET_H_

class ListWidget : public QListWidget {

	Q_OBJECT

public:
	ListWidget(QWidget *parent);
	virtual ~ListWidget();

//	void setCurrentIndex(int index, bool removeUndefined = true) {
//		itsPreviousSelection = selectedIndexes(); setUndefined(false, removeUndefined);
//	}
//	void setFromString(const QString &t);
	void addItems(const QStringList &names, const QStringList &checkedNames = QStringList());
	QString checkedItemsAsString(void) const;
	void checkItems(const QStringList &items_to_select);
	void setUndefined(bool enabled = true);
	bool isUndefined(void) const {return itsUndefined;}
	bool hasBeenChanged(void) const;
	void resetChangeDetect(void) {
		itsPreviousCheckedNames = checkedItemsList();
	}

private:
	QStringList checkedItemsList(void) const;
	QStringList items(void) const;

private:
	bool itsUndefined;
	QStringList itsPreviousNames, itsPreviousCheckedNames;
};

#endif /* LISTWIDGET_H_ */
