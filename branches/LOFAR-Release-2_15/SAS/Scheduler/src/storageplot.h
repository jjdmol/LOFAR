/*
 * storageplot.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : June 25, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/storageplot.h $
 *
 */

#ifndef STORAGEPLOT_H
#define STORAGEPLOT_H

#include "qcustomplot.h"
#include <QMap>

class Storage;

class StoragePlot : public QCustomPlot
{
    Q_OBJECT
public:
    explicit StoragePlot(QWidget *parent = 0);

    void init(unsigned nrPlots);
    void plot(const Storage &storage);

signals:

public slots:

private:
    QMap<unsigned , QCPGraph *> itsPlots;
};

#endif // STORAGEPLOT_H
