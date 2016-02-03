/*
 * imagingpipeline.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/imagingpipeline.h $
 *
 */

#ifndef IMAGINGPIPELINE_H
#define IMAGINGPIPELINE_H

#include "pipeline.h"

class ImagingPipeline : public Pipeline
{
public:
    ImagingPipeline();
    ImagingPipeline(unsigned task_id);
    ImagingPipeline(const QSqlQuery &query, const OTDBtree &SAS_tree);
    ImagingPipeline(unsigned id, const OTDBtree &SAS_tree);

    ImagingPipeline(bool specifyFOV, quint16 slicesPerImage, quint16 subbandsPerImage, quint16 nrOfPixels,
                    const double &fov, const QString &cellSize);

    virtual void clone(const Task *other) {
        if (this != other) {
            const ImagingPipeline *pOther = dynamic_cast<const ImagingPipeline *>(other);
            if (pOther) {
                unsigned myTaskID = taskID;
                *this = *pOther;
                taskID = myTaskID;
            }
        }
    }

    friend QDataStream& operator>> (QDataStream &in, ImagingPipeline &im);
    friend QDataStream& operator<< (QDataStream &out, const ImagingPipeline &im);

    // getters
    inline bool specifyFov(void) const {return itsSpecifyFOV;}
    inline quint16 slicesPerImage(void) const {return itsSlicesPerImage;}
    inline quint16 subbandsPerImage(void) const {return itsSubbandsPerImage;}
    inline quint16 nrOfPixels(void) const {return itsNrOfPixels;}
    inline const double &fov(void) const {return itsFov;}
    inline const QString &cellSize(void) const {return itsCellSize;}

    // setters
    void setSpecifyFov(bool enabled) {itsSpecifyFOV = enabled;}
    void setSlicesPerImage(quint16 value) {itsSlicesPerImage = value;}
    void setSubbandsPerImage(quint16 value) {itsSubbandsPerImage = value;}
    void setNrOfPixels(quint16 value) {itsNrOfPixels = value;}
    void setFov(const double &fov) {itsFov = fov;}
    void setCellSize(const QString &cellsize) {itsCellSize = cellsize;}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

    virtual void calculateDataSize(void);

private:
    bool itsSpecifyFOV;
    quint16 itsSlicesPerImage, itsSubbandsPerImage, itsNrOfPixels;
    double itsFov;
    QString itsCellSize;
};

#endif // IMAGINGPIPELINE_H
