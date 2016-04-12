#include "statehistorydialog.h"

StateHistoryDialog::StateHistoryDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	ui.tableWidgetStateInfo->setColumnCount(5);
	QStringList header;
	header << "tree ID" << "mom ID" << "new state" << "user name" << "modification time";
	ui.tableWidgetStateInfo->setHorizontalHeaderLabels(header);
	ui.tableWidgetStateInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidgetStateInfo->horizontalHeader()->setStretchLastSection(true);
#if QT_VERSION >= 0x050000
    ui.tableWidgetStateInfo->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui.tableWidgetStateInfo->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
	this->setWindowTitle("Task state change history");
}

StateHistoryDialog::~StateHistoryDialog()
{

}

void StateHistoryDialog::addStateInfo(const QString &treeID, const QString &momID,
		const QString &state, const QString &username, const QDateTime &modtime) {
	int row(ui.tableWidgetStateInfo->rowCount());
	ui.tableWidgetStateInfo->insertRow(ui.tableWidgetStateInfo->rowCount());
	QTableWidgetItem *newItem = new QTableWidgetItem(treeID);
	ui.tableWidgetStateInfo->setItem(row, 0, newItem);
	newItem = new QTableWidgetItem(momID);
	ui.tableWidgetStateInfo->setItem(row, 1, newItem);
	newItem = new QTableWidgetItem(state);
	ui.tableWidgetStateInfo->setItem(row, 2, newItem);
	newItem = new QTableWidgetItem(username);
	ui.tableWidgetStateInfo->setItem(row, 3, newItem);
	newItem = new QTableWidgetItem(modtime.toString("yyyy-MM-dd hh:mm:ss"));
	ui.tableWidgetStateInfo->setItem(row, 4, newItem);

#if QT_VERSION >= 0x050000
    ui.tableWidgetStateInfo->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui.tableWidgetStateInfo->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
}
