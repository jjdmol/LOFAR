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

#include "storageplot.h"
#include "Storage.h"

StoragePlot::StoragePlot(QWidget *parent) :
    QCustomPlot(parent)
{
}

void StoragePlot::init(unsigned nrPlots) {
    plotLayout()->clear(); // clear default axis rect so we can start from scratch
    // synchronize the left and right margins of the top and bottom axis rects:
    QCPMarginGroup *marginGroup = new QCPMarginGroup(this);

    for (unsigned plotNr = 0; plotNr < nrPlots; ++plotNr) {
        QCPAxisRect *plot = new QCPAxisRect(this);
        plot->setupFullAxesBox(true);
        this->plotLayout()->addElement(plotNr, 0, plot); // insert axis rect in first row
        plot->setMarginGroup(QCP::msLeft, marginGroup);
        QCPGraph *graph = addGraph(plot->axis(QCPAxis::atBottom), plot->axis(QCPAxis::atLeft));
        itsPlots[plotNr] = graph;
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssNone, QPen(Qt::black), QBrush(Qt::white), 6));
        graph->setPen(QPen(QColor(120, 120, 120), 2));
        graph->setBrush(QColor(255, 161, 0, 50));
        graph->setName("locus001");
        plot->axis(QCPAxis::atLeft)->setLabel("free space (GB)");
    }

    // move newly created axes on "axes" layer and grids on "grid" layer:
    foreach (QCPAxisRect *rect, this->axisRects())
    {
        foreach (QCPAxis *axis, rect->axes())
        {
            axis->setLayer("axes");
            axis->grid()->setLayer("grid");
        }
    }
}

void StoragePlot::plot(const Storage &storage) {
    QCPGraph *graph(0);
    QVector<double> x(100), data(100);
    const storageNodesMap &storageNodes(storage.storageNodes());
    int i = 0;
    for (storageNodesMap::const_iterator it = storageNodes.begin(); it != storageNodes.end(); ++it) {
        double xValue(0);
        graph = itsPlots[i++];
        const std::vector<capacityLogPoint> &capLog(it->second.getRemainingSpace().begin()->second);
        int xcount(0);
        for (std::vector<capacityLogPoint>::const_iterator cit = capLog.begin(); cit != capLog.end(); ++cit) {

            data[xcount] = cit->remainingDiskSpacekB;
            x[xcount++] = xValue;

//            data.push_back(cit->remainingDiskSpacekB);
//            x.push_back(xValue);
            xValue += 1.0;
        }
        graph->setData(x, data);
        graph->rescaleAxes();
        if (i>=5) break; // REMOVE
    }

    // prepare data:
//    QVector<double> x1a(20), y1a(20);
//    for (int i=0; i<x1a.size(); ++i)
//    {
//      x1a[i] = i/(double)(x1a.size()-1)*10-5.0;
//      y1a[i] = qCos(x1a[i]);
//    }

//    QCPGraph *graph = itsPlots[0];//this->graph();
//    storage->getStoragePlotData();
//    graph->setData(x1a, y1a);
//    graph->rescaleAxes();

}
