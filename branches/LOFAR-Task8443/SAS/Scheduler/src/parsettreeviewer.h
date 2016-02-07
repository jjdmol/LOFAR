/*
 * parsettreeviewer.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 10-June-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/parsettreeviewer.h $
 * Description    : The parset tree viewer is a dialog that shows the complete SAS OTDB tree of a single observation
 *
 */

#ifndef PARSETTREEVIEWER_H
#define PARSETTREEVIEWER_H

#include <QtGui/QDialog>
#include "ui_parsettreeviewer.h"
#include "OTDBtree.h"

class ParsetTreeViewer : public QDialog
{
    Q_OBJECT

public:
    ParsetTreeViewer(QWidget *parent = 0);
    ~ParsetTreeViewer();

    void view(QString parset, const OTDBtree &otdb_tree = OTDBtree());

private:
    QString commaWordWrap(const QString &str, int wrapLength, const QChar &wrapChar = ',') const;


private:
    Ui::ParsetTreeViewerClass ui;
};

#endif // PARSETTREEVIEWER_H
