/*
 * stationtask.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/stationtask.h $
 *
 */

#ifndef STATIONTASK_H
#define STATIONTASK_H

#include <QSqlQuery>
#include "task.h"
#include "station.h"
#include "OTDBtree.h"

typedef std::map<unsigned short, std::pair<unsigned, unsigned> >stationDataSlotMap;
// dataSlotMap: key = station ID
typedef std::map<unsigned, stationDataSlotMap> dataSlotMap;
typedef std::map<std::string, std::vector<unsigned> > superStationMap;
typedef std::map<std::string, unsigned int> taskStationsMap;

class StationTask : public Task
{
public:
    StationTask(task_type type);
    StationTask(unsigned task_id, task_type type);
    StationTask(unsigned task_id, const OTDBtree &SAS_tree, task_type type);
    StationTask(const QSqlQuery &query, const OTDBtree &SAS_tree, task_type type);

    virtual StationTask & operator=(const Task &);

    virtual void clone(const Task *other) {
        if (this != other) {
            const StationTask *pOther = dynamic_cast<const StationTask *>(other);
            if (pOther) {
                unsigned myTaskID = taskID;
                *this = *pOther;
                taskID = myTaskID;
            }
        }
    }

    friend QDataStream& operator<< (QDataStream &out, const StationTask &task); // used for writing data to binary file
    friend QDataStream& operator>> (QDataStream &in, StationTask &task); // used for reading data from binary file

    // getters
    inline const char * getSASFilterType() const {return SAS_filter_types_str[itsFilterType];}
    station_filter_type getFilterType(void) const {return itsFilterType; }
    inline const char * getFilterTypeStr() const {return filter_types_str[itsFilterType];}
    station_antenna_mode getAntennaMode() const { return itsAntennaMode; }
    inline const char * getAntennaModeStr() const {return antenna_modes_str[itsAntennaMode];}
    inline const char * getSASAntennaSet() const {return SAS_antenna_modes_str[itsAntennaMode];}
    unsigned short getStationClockFrequency() const { return itsClockFrequency == clock_160Mhz ? 160 : 200;}
    station_clock getStationClock() const { return itsClockFrequency; }
    inline const char * getStationClockStr() const {return clock_frequencies_str[itsClockFrequency];}
    const taskStationsMap &getStations(void) const {return itsStations;}
    unsigned int getNrVirtualStations(void) const {return itsNrVirtualStations;}
    void updateStationIDs();
    std::string getStationNamesStr(const char separater = ',', int nrStationsOnLine = 0, const std::string &newLine = "\n") const;
    std::string getSASStationList(void) const;
    const dataSlotMap &getDataSlots(void) const {return itsDataSlots;}
    std::string getAntennaField(void) const;
    const stationDataSlotMap &getStationDataSlots(unsigned stationID) const;
    unsigned getNrOfDataslotsPerRSPboard(void) const {return itsNrOfDataslotsPerRSPboard;}

    // settters
    // sets the dataSlots for the digital beam with index
    void clearDataSlots(void) {itsDataSlots.clear();}
    void assignDataSlots(unsigned stationID, const stationDataSlotMap &dataSlots) {itsDataSlots[stationID] = dataSlots;}

    bool setAntennaMode(station_antenna_mode);
    bool setAntennaMode(const std::string &mode);
    bool setAntennaMode(unsigned short mode);
    bool setSASAntennaMode(const std::string &mode);
    bool setFilterType(station_filter_type);
    bool setFilterType(const std::string &filter);
    bool setFilterType(unsigned short filter);
    bool setSASFilterType(const std::string &filter);
    bool setStationClock(station_clock);
    bool setStationClock(unsigned int);
    bool setStationClock(const std::string &clock);
    void setStations(const QString &stations, const QChar &separator);
    void setStations(const std::vector<QString> &stations);
    void setStations(const std::vector<std::string> &stations);
    void setStations(const taskStationsMap &newStations);
    taskStationsMap &getStationsForChange(void) {return itsStations;}
    void removeStation(const std::string &station_name);
    inline void setSuperStations(const superStationMap &super_stations) {itsSuperStations = super_stations;}

    virtual bool diff(const Task *other, task_diff &dif) const;
    virtual QString diffString(const task_diff &dif) const;

private:
    // calculates the number of virtual stations (for HBA_DUAL & HBA_DUAL_INNER core stations count for two)
    void calculateNrVirtualStations(void);
    //check beamformers check the beamformers for removed stations
    void checkSuperStations(void);
    std::string antennaFieldName(const std::string &station) const;

protected:
    taskStationsMap itsStations; // list of required stations for this task
    unsigned int itsNrVirtualStations;
    superStationMap itsSuperStations; // list of super stations (= beamformers)
    station_antenna_mode itsAntennaMode; // the station antenna configuration for this task
    station_clock itsClockFrequency; // the station clock frequency (clock_160Mhz or clock_200Mhz)
    station_filter_type itsFilterType; // the type of RCU filter used
    dataSlotMap itsDataSlots;
    stationDataSlotMap nonExistingStationDataSlotMap;
    quint16 itsNrOfDataslotsPerRSPboard;
};

#endif // STATIONTASK_H
