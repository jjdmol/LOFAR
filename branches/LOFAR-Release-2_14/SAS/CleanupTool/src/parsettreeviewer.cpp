/*
 * parsettreeviewer.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 10-June-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/parsettreeviewer.cpp $
 * Description    : The parset tree viewer is a dialog that shows the complete SAS OTDB tree of a single observation
 *
 */

#include "parsettreeviewer.h"

ParsetTreeViewer::ParsetTreeViewer(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	ui.treeWidget->setColumnCount(2);
	ui.treeWidget->setColumnWidth(0,400);
	QStringList labels;
	labels << "Node / Key" << "Value";
	ui.treeWidget->setHeaderLabels(labels);
}

ParsetTreeViewer::~ParsetTreeViewer()
{

}

QString ParsetTreeViewer::commaWordWrap(const QString &str, int wrapLength, const QChar &wrapChar) const {
	QString tempStr = str;
	int len = str.length(), pos = (wrapLength > 1) ? wrapLength-1:1, tempPos;
	while (pos < len-1) {

		tempPos = pos;
		while (tempStr.at(tempPos) != wrapChar && tempPos > 0) {
			tempPos--;
		}
		if (tempPos > 0) pos = tempPos;

		tempStr.insert(pos+1, '\n');
		pos += wrapLength+1;
	}

	return tempStr;
}

void ParsetTreeViewer::view(const QString &parset, const OTDBtree &otdb_tree) {
	if (!parset.isEmpty()) {
		ui.treeWidget->clear();
		QStringList lines(parset.split("\n", QString::SkipEmptyParts));
		lines.sort();
		// add meta data from the OTDB tree
		lines += QString("current status in SAS=") + sasStateString(otdb_tree.state()).c_str();
		lines += QString("actual start time=") + otdb_tree.startTime().toString().c_str();
		lines += QString("actual stop time=") + otdb_tree.stopTime().toString().c_str();
		lines += QString("creation date=") + otdb_tree.creationDate().toString().c_str();
		lines += QString("modified date=") + otdb_tree.modificationDate().toString().c_str();
		lines += QString("SAS ID=") + QString::number(otdb_tree.treeID());
		lines += QString("group ID=") + QString::number(otdb_tree.groupID());
		lines += QString("MoM ID=") + QString::number(otdb_tree.momID());
		lines += QString("parent template=") + QString::number(otdb_tree.originalTree());
		lines += QString("campaign=") + otdb_tree.campaign().c_str();
		lines += QString("processType=") + otdb_tree.processType();
		lines += QString("processSubType=") + PROCESS_SUBTYPES[otdb_tree.processSubType()];
		lines += QString("strategy=") + otdb_tree.strategy();

		QStringList key_value_pair, prev_line, current_line, item_pair;
		QTreeWidgetItem *newItem;
		QList<QTreeWidgetItem *> topItems, parents, prevParents;
		QString node1, node2, value, tooltip;
		int idx, nrElements;
		for (QStringList::iterator it = lines.begin()+1; it != lines.end(); ++it) {
			key_value_pair = it->split('='); // key_value_pair.first() = full node/key parset name, key_value_pair.last() = value
			if (key_value_pair.size() > 1) {
				value = key_value_pair.last();
			}
			else value = "";
			current_line = key_value_pair.first().split('.');
			prevParents = parents;
			parents.clear();
			idx = 0;
			nrElements = current_line.size();
			while (idx < nrElements) { // while not arrived at last element and not beyond the length of the previous line
				if (idx < prev_line.size()) { // if not beyond the number of elements in the previous line
					if (current_line.at(idx) != prev_line.at(idx)) { // if the item on this position is different from the item on that position of the previous line
						// insert element
						if (idx+1 == nrElements) {
							item_pair << current_line.at(idx) << value;
							tooltip = commaWordWrap(item_pair.first() + " = " + value,100);
						}
						else {
							item_pair << current_line.at(idx);
							tooltip = item_pair.first();
						}

						if (parents.empty()) newItem = new QTreeWidgetItem(ui.treeWidget, item_pair);
						else newItem = new QTreeWidgetItem(parents.last(), item_pair);
						newItem->setToolTip(0,tooltip);
						newItem->setToolTip(1,tooltip);
						parents.push_back(newItem);
						item_pair.clear();
					}
					else parents.push_back(prevParents.at(idx));
				}
				else {
					// insert element
					if (idx+1 == nrElements) {
						item_pair << current_line.at(idx) << value;
						tooltip = commaWordWrap(item_pair.first() + " = " + value,100);
					}
					else {
						item_pair << current_line.at(idx);
						tooltip = item_pair.first();
					}

					if (parents.empty()) newItem = new QTreeWidgetItem(ui.treeWidget, item_pair);
					else newItem = new QTreeWidgetItem(parents.last(), item_pair);
					newItem->setToolTip(0,tooltip);
					newItem->setToolTip(1,tooltip);
					parents.push_back(newItem);
					item_pair.clear();
				}
				++idx;
			}
			prev_line = current_line;
		}
		this->show();
	}
}
