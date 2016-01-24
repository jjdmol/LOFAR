/*
 * parsettreeviewer.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 10-June-2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/parsettreeviewer.h $
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

    void view(const QString &parset, const OTDBtree &otdb_tree);

private:
    QString commaWordWrap(const QString &str, int wrapLength, const QChar &wrapChar = ',') const;


private:
    Ui::ParsetTreeViewerClass ui;
};

#endif // PARSETTREEVIEWER_H
