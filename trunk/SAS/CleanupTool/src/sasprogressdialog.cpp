#include "sasprogressdialog.h"

SASProgressDialog::SASProgressDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
	clear();
}

SASProgressDialog::~SASProgressDialog()
{

}

void SASProgressDialog::addText(const QString &text) {
	QListWidgetItem *item = new QListWidgetItem(text, ui.listWidget_Progress);
	ui.listWidget_Progress->scrollToItem(item);
//    	ui.listWidget_Progress->repaint();
//    	repaint();
	QCoreApplication::processEvents(); // force an immediate redraw
}

void SASProgressDialog::addError(const QString &text) {
	QListWidgetItem *item = new QListWidgetItem(text, ui.listWidget_Progress);
	item->setForeground(QBrush(Qt::red));
	ui.listWidget_Progress->scrollToItem(item);
//    	ui.listWidget_Progress->repaint();
//    	repaint();
	QCoreApplication::processEvents();
}
