/*
 * LongBaselinePipeline.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/longbaselinepipeline.h $
 *
 */

#ifndef LongBaselinePipeline_H
#define LongBaselinePipeline_H

#include "pipeline.h"

class LongBaselinePipeline : public Pipeline
{
public:
    LongBaselinePipeline();
    LongBaselinePipeline(unsigned task_id);
    LongBaselinePipeline(const QSqlQuery &query, const OTDBtree &SAS_tree);
    LongBaselinePipeline(unsigned id, const OTDBtree &SAS_tree);

    LongBaselinePipeline(quint16 subbandGroupsPerMS, quint16 subbandsPerSubbandGroup);

    virtual void clone(const Task *other) {
        if (this != other) {
            const LongBaselinePipeline *pOther = dynamic_cast<const LongBaselinePipeline *>(other);
            if (pOther) {
                unsigned myTaskID = taskID;
                *this = *pOther;
                taskID = myTaskID;
            }
        }
    }

    friend QDataStream& operator>> (QDataStream &in, LongBaselinePipeline &im);
    friend QDataStream& operator<< (QDataStream &out, const LongBaselinePipeline &im);

    // getters
    inline quint16 subbandGroupsPerMS(void) const {return itsSubbandGroupsPerMS;}
    inline quint16 subbandsPerSubbandGroup(void) const {return itsSubbandsPerSubbandGroup;}

    // setters
    void setSubbandGroupsPerMS(quint16 value) {itsSubbandGroupsPerMS = value;}
    void setSubbandsPerSubbandGroup(quint16 value) {itsSubbandsPerSubbandGroup = value;}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

    virtual void calculateDataSize(void);

private:
    quint16 itsSubbandGroupsPerMS, itsSubbandsPerSubbandGroup;
};

#endif // LongBaselinePipeline_H
