/*
 * stationtask.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Jul 18, 2014
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/stationtask.cpp $
 *
 */

#include "stationtask.h"
#include "Controller.h"

StationTask::StationTask(task_type type)
    : Task(), itsNrVirtualStations(0), itsAntennaMode(UNSPECIFIED_ANTENNA_MODE), itsClockFrequency(UNSPECIFIED_CLOCK), itsFilterType(UNSPECIFIED_FILTER),
      itsNrOfDataslotsPerRSPboard(61)
{
    itsTaskType = type;
}

StationTask::StationTask(unsigned task_id, task_type type)
    : Task(task_id), itsNrVirtualStations(0), itsAntennaMode(UNSPECIFIED_ANTENNA_MODE), itsClockFrequency(UNSPECIFIED_CLOCK), itsFilterType(UNSPECIFIED_FILTER),
      itsNrOfDataslotsPerRSPboard(61)
{
    itsTaskType = type;
}


StationTask::StationTask(unsigned task_id, const OTDBtree &SAS_tree, task_type type)
    :   Task(task_id, SAS_tree), itsNrVirtualStations(0), itsAntennaMode(UNSPECIFIED_ANTENNA_MODE), itsClockFrequency(UNSPECIFIED_CLOCK), itsFilterType(UNSPECIFIED_FILTER),
      itsNrOfDataslotsPerRSPboard(61)
{
    itsTaskType = type;
}

StationTask::StationTask(const QSqlQuery &query, const OTDBtree &SAS_tree, task_type type)
    : Task(query, SAS_tree), itsNrVirtualStations(0), itsAntennaMode(UNSPECIFIED_ANTENNA_MODE), itsClockFrequency(UNSPECIFIED_CLOCK), itsFilterType(UNSPECIFIED_FILTER),
      itsNrOfDataslotsPerRSPboard(61)
{
    itsTaskType = type;
}

QDataStream& operator<< (QDataStream &out, const StationTask &task) {
    if (out.status() == QDataStream::Ok) {
        out << static_cast<const Task &>(task);

        // write station names and IDs for this task
        out << (quint32) task.itsStations.size();
        for (taskStationsMap::const_iterator it = task.itsStations.begin(); it != task.itsStations.end(); ++it) {
            out << it->first << (quint16) it->second;
        }
        // itsSuperStations
        out << (quint32) task.itsSuperStations.size();
        for (superStationMap::const_iterator it = task.itsSuperStations.begin(); it != task.itsSuperStations.end(); ++it) {
            out << it->first << (quint32) it->second.size();
            for (std::vector<unsigned>::const_iterator vit = it->second.begin(); vit != it->second.end(); ++vit) {
                out << (quint16) *vit;
            }
        }

        // itsAntennaMode, itsFilterType, itsClockFrequency
        out << (quint8) task.itsAntennaMode << (quint8) task.itsFilterType << (quint8) task.itsClockFrequency;

        // data slot list (don't write this?)
        out << (quint32) task.itsDataSlots.size();
        for (dataSlotMap::const_iterator it = task.itsDataSlots.begin(); it != task.itsDataSlots.end(); ++it) {
            out << (quint16) it->first // stationID
                << (quint32) it->second.size(); // nr of RSP boards assigned
            for (stationDataSlotMap::const_iterator sdit = it->second.begin(); sdit != it->second.end(); ++sdit) {
                out << (quint8) sdit->first // RSP board number
                    << (quint16) sdit->second.first // minimum dataslot used on this board
                    << (quint16) sdit->second.second; // maximum dataslot used on this board
            }
        }

        // itsNrOfDataslotsPerRSPboard
        out << task.itsNrOfDataslotsPerRSPboard;
    }
    return out;
}

QDataStream& operator>> (QDataStream &in, StationTask &task) {
    if (in.status() == QDataStream::Ok) {
        quint32 nrOfObjects, nrOfObjects2;
        quint16 stationID;
        quint8 mode, filter, clock;
        std::string tmpString;

        in >> static_cast<Task &>(task);

        task.itsStations.clear();
        // read station ids for this task
        in >> nrOfObjects;
        for (quint32 i = 0; i < nrOfObjects; ++i) {
            in >> tmpString >> stationID; // station name, station id
            task.itsStations[tmpString] = stationID;
        }

        // itsSuperStations
        std::vector<unsigned> station_ids;
        task.itsSuperStations.clear();
        in >> nrOfObjects;
        for (quint32 i = 0; i < nrOfObjects; ++i) {
            in >> tmpString >> nrOfObjects2; // super station name, number of stations in the superstation list
            for (quint32 j = 0; j < nrOfObjects2; ++j) {
                in >> stationID;
                station_ids.push_back(stationID);
            }
            task.itsSuperStations[tmpString] = station_ids;
            station_ids.clear();
        }

        // itsAntennaMode, itsFilterType, itsClockFrequency
        in >> mode >> filter >> clock;
        task.itsAntennaMode = (station_antenna_mode) mode;
        task.itsFilterType = (station_filter_type) filter;
        task.itsClockFrequency = (station_clock) clock;

        // data slot list
        task.itsDataSlots.clear();
        std::pair<unsigned, unsigned> dataSlotPair;
        quint8 RSPboard;
        quint16 minDataSlot, maxDataSlot;
        stationDataSlotMap dsMap;
        in >> nrOfObjects; // number of stations in dataSlotMap
        for (quint32 i=0; i < nrOfObjects; ++i) {
            in >> stationID // stationID
               >> nrOfObjects2; // number of RSPBoards assigned
            for (quint32 i = 0; i < nrOfObjects2; ++i) {
                in >> RSPboard // RSP board number
                   >> minDataSlot // lowest data slot number on this board that is used
                   >> maxDataSlot; // highest data slot number on this board that is used
                dataSlotPair.first  = minDataSlot;
                dataSlotPair.second = maxDataSlot;
                dsMap[RSPboard] = dataSlotPair;
                task.itsDataSlots[stationID] = dsMap;
            }
            dsMap.clear();
        }

        // itsNrOfDataslotsPerRSPboard
        in >> task.itsNrOfDataslotsPerRSPboard;
    }

    task.calculateNrVirtualStations();

    return in;
}

StationTask & StationTask::operator=(const Task &other) {
    if (this != &other) {
        Task::operator=(other);
        const StationTask *pOther = dynamic_cast<const StationTask *>(&other);
        if (pOther) {
            itsStations = pOther->itsStations;
            itsNrVirtualStations = pOther->itsNrVirtualStations;
            itsSuperStations = pOther->itsSuperStations;
            itsAntennaMode = pOther->itsAntennaMode;
            itsClockFrequency = pOther->itsClockFrequency;
            itsFilterType = pOther->itsFilterType;
            itsDataSlots = pOther->itsDataSlots;
            nonExistingStationDataSlotMap = pOther->nonExistingStationDataSlotMap;
            itsNrOfDataslotsPerRSPboard = pOther->itsNrOfDataslotsPerRSPboard;
        }
    }
    return *this;
}

void StationTask::removeStation(const std::string &station_name)
{
    taskStationsMap::iterator sit;
    if ((sit = itsStations.find(station_name)) != itsStations.end()) {
        itsStations.erase(sit);
    }
}

std::string StationTask::getAntennaField(void) const {
    switch (itsAntennaMode) {
    case LBA_INNER:
    case LBA_OUTER:
    case LBA_SPARSE_EVEN:
    case LBA_SPARSE_ODD:
    case LBA_X:
    case LBA_Y:
        return "LBA";
        break;
    case HBA_ZERO:
    case HBA_ZERO_INNER:
        return "HBA0";
        break;
    case HBA_ONE:
    case HBA_ONE_INNER:
        return "HBA1";
        break;
    case HBA_DUAL:
    case HBA_DUAL_INNER:
        return "HBA_DUAL";
        break;
    case HBA_JOINED:
    case HBA_JOINED_INNER:
        return "HBA";
        break;
    default:
        break;
    }
    return std::string();
}

std::string StationTask::antennaFieldName(const std::string &station) const {
    return station + getAntennaField();
}

bool StationTask::setAntennaMode(station_antenna_mode mode) {
    if ((mode >= 0) && mode < ANTENNA_MODE_END) {
        itsAntennaMode = mode;
        calculateNrVirtualStations();
        return true;
    }
    else if (isObservation()) {
        debugWarn("sisi", "Task:", taskID, ", Unknown antenna mode: ", static_cast<int>(mode));
    }
    return false;
}

bool StationTask::setAntennaMode(const std::string &mode) {
    if (!mode.empty()) {
        for (unsigned int idx = 0; idx != NR_ANTENNA_MODES; ++idx) {
            if (mode.compare(antenna_modes_str[idx]) == 0) {
                itsAntennaMode = static_cast<station_antenna_mode>(idx);
                calculateNrVirtualStations();
                return true;
            }
        }
    }
    else if (isObservation()) {
        debugWarn("siss", "Task:", taskID, " , Unknown antenna mode: ", mode.c_str());
    }
    return false;
}

bool StationTask::setSASAntennaMode(const std::string &mode) {
    if (!mode.empty()) {
        for (unsigned int idx = 0; idx != NR_ANTENNA_MODES; ++idx) {
            if (mode.compare(SAS_antenna_modes_str[idx]) == 0) {
                itsAntennaMode = static_cast<station_antenna_mode>(idx);
                calculateNrVirtualStations();
                return true;
            }
        }
    }
    else if (isObservation()) {
        debugWarn("siss", "Task:", taskID, " , Unknown antenna mode: ", mode.c_str());
    }
    return false;
}

bool StationTask::setAntennaMode(unsigned short mode) {
    if (mode < ANTENNA_MODE_END) {
        itsAntennaMode = static_cast<station_antenna_mode>(mode);
        calculateNrVirtualStations();
        return true;
    }
    else if (isObservation()) {
        debugWarn("sisi", "Task:", taskID, " , Could not set antenna mode to:", mode);
    }
    return false;
}

bool StationTask::setFilterType(station_filter_type filter) {
    if ((filter >= 0) && filter < FILTER_TYPE_END) {
        itsFilterType = filter;
        return true;
    }
    else if (isObservation()) {
        debugWarn("sisi", "Task:", taskID, ", Unknown filter type: ", static_cast<int>(filter));
    }
    return false;
}

bool StationTask::setFilterType(const std::string &filter) {
    if (!filter.empty()) {
        for (unsigned int idx = 0; idx != NR_FILTER_TYPES; ++idx) {
            if (filter.compare(filter_types_str[idx]) == 0) {
                itsFilterType = static_cast<station_filter_type>(idx);
                return true;
            }
        }
    }
    else if (isObservation()) {
        debugWarn("siss", "Task:", taskID, " , Unknown filter type: ", filter.c_str());
    }
    return false;
}

bool StationTask::setSASFilterType(const std::string &filter) {
    if (!filter.empty()) {
        for (unsigned int idx = 0; idx != NR_FILTER_TYPES; ++idx) {
            if (filter.compare(SAS_filter_types_str[idx]) == 0) {
                itsFilterType = static_cast<station_filter_type>(idx);
                return true;
            }
        }
    }
    else if (isObservation()) {
        debugWarn("siss", "Task:", taskID, " , Unknown filter type: ", filter.c_str());
    }
    return false;
}

bool StationTask::setFilterType(unsigned short filter) {
    if (filter < FILTER_TYPE_END) {
        itsFilterType = static_cast<station_filter_type>(filter);
        return true;
    }
    else if (isObservation()) {
        debugWarn("sisi", "Task:", taskID, " , Could not set filter type to:", filter);
    }
    return false;
}


bool StationTask::setStationClock(station_clock clock) {
    station_clock prevClock = itsClockFrequency;
    if (clock >= clock_160Mhz && clock <= clock_200Mhz) {
        itsClockFrequency = clock;
        if (itsClockFrequency != prevClock) {
        }
        return true;
    }
    else if (isObservation()) {
        debugWarn("s", "The requested clock frequency is not supported");
    }
    return false;
}

bool StationTask::setStationClock(unsigned int clk) {
    station_clock prevClock = itsClockFrequency;
    if (clk == 160) {
        itsClockFrequency = clock_160Mhz;
        if (itsClockFrequency != prevClock) {
        }
        return true;
    }
    else if (clk == 200) {
        itsClockFrequency = clock_200Mhz;
        if (itsClockFrequency != prevClock) {
        }
        return true;
    }
    else if (isObservation()) {
        debugWarn("sis", "The requested clock frequency: ", clk, "is not supported");
    }
    return false;
}

bool StationTask::setStationClock(const std::string &clock) {
    station_clock prevClock = itsClockFrequency;
    if (!clock.empty()) {
        for (unsigned int idx = 0; idx != NR_CLOCK_FREQUENCIES; ++idx) {
            if (clock.compare(clock_frequencies_str[idx]) == 0) {
                itsClockFrequency = static_cast<station_clock>(idx);
                if (itsClockFrequency != prevClock) {
                }
                return true;
            }
        }
    }
    else if (isObservation()) {
        debugWarn("siss", "Task:", taskID, " , Error:Unknown station clock: ", clock.c_str());
    }
    return false;
}

const stationDataSlotMap &StationTask::getStationDataSlots(unsigned stationID) const {
    dataSlotMap::const_iterator it = itsDataSlots.find(stationID);
    if (it != itsDataSlots.end())
        return it->second;
    else
        return nonExistingStationDataSlotMap;
}

//check beamformers check the beamformers for removed stations
void StationTask::checkSuperStations(void) {
    superStationMap newSuperStations;
    std::vector<unsigned> tmpIDs;
    for (superStationMap::iterator it = itsSuperStations.begin(); it != itsSuperStations.end(); ++it) { // super-stations
        tmpIDs.clear();
        for (std::vector<unsigned>::const_iterator sit = it->second.begin(); sit  != it->second.end(); ++sit) { // station IDs for this super-stations
            for (taskStationsMap::const_iterator ssit = itsStations.begin(); ssit != itsStations.end(); ++ssit) { // search assigned stations
                if (ssit->second == *sit) {
                    tmpIDs.push_back(*sit);
                    break;
                }
            }
        }
        if (!tmpIDs.empty()) {
            newSuperStations.insert(superStationMap::value_type(it->first, tmpIDs));
        }
    }
    itsSuperStations = newSuperStations;
}

void StationTask::setStations(const std::vector<std::string> &stations) {
    itsStations.clear();
    for (std::vector<std::string>::const_iterator it = stations.begin(); it != stations.end(); ++it) {
        itsStations[*it] = Controller::theSchedulerSettings.getStationID(*it);
    }
    calculateNrVirtualStations();
    checkSuperStations();
}

void StationTask::setStations(const std::vector<QString> &stations) {
    itsStations.clear();
    for (std::vector<QString>::const_iterator it = stations.begin(); it != stations.end(); ++it) {
        const std::string &stationName = it->toStdString();
        itsStations[stationName] = Controller::theSchedulerSettings.getStationID(stationName);
    }
    checkSuperStations();
    calculateNrVirtualStations();
}

void StationTask::setStations(const QString &stations, const QChar &separator) {
    setStations(string2VectorOfStrings(stations, separator));
}

void StationTask::setStations(const taskStationsMap &newStations) {
    itsStations = newStations;
    checkSuperStations();
    calculateNrVirtualStations();
}

void StationTask::calculateNrVirtualStations(void) {
    itsNrVirtualStations = 0;
    if ((itsAntennaMode == HBA_DUAL) || (itsAntennaMode == HBA_DUAL_INNER)) {
        for (taskStationsMap::const_iterator it = itsStations.begin(); it != itsStations.end(); ++it) {
            if (it->first.compare(0,2,"CS") == 0) itsNrVirtualStations += 2;
            else ++itsNrVirtualStations;
        }
    }
    else itsNrVirtualStations = itsStations.size();
}

void StationTask::updateStationIDs(void) {
    unsigned int id;
    for (taskStationsMap::iterator it = itsStations.begin(); it != itsStations.end(); ++it) {
        id = Controller::theSchedulerSettings.getStationID(it->first);
        if (id) {
            it->second = id;
        }
        else {
            it->second = 0;
        }
    }
}


std::string StationTask::getStationNamesStr(const char separater, int nrStationsOnLine, const std::string &newLine) const {
    std::string stations;
    if (!itsStations.empty()) {
        int idx(1);
        for (taskStationsMap::const_iterator it = itsStations.begin(); it != itsStations.end(); ++it) {
            stations += it->first;
            if (nrStationsOnLine && (idx % nrStationsOnLine == 0)) {
                stations += newLine;
                idx = 1;
            }
            else {
                stations += separater;
                ++idx;
            }
        }
        std::string::iterator sit = stations.end() - 1;
        if (*sit == separater) {
            stations.erase(sit);
        }
    }

    return stations;
}

std::string StationTask::getSASStationList(void) const {
    std::string stations;
    if (!itsStations.empty()) {
        stations = "[";
        for (taskStationsMap::const_iterator it = itsStations.begin(); it != itsStations.end(); ++it) {
            stations += it->first + ",";
        }
        stations = stations.substr(0,stations.length()-1) + "]";
    }
    return stations;
}

/*
 *  diff check for difference between this task and other task.
 *  return true if difference
 *  parameter dif
 */
bool StationTask::diff(const Task *other, task_diff &dif) const {
    bool taskDif(Task::diff(other, dif));

    const StationTask *pOther = dynamic_cast<const StationTask *>(other);
    if (pOther) {
        dif.station_ids = itsStations != pOther->getStations() ? true : false;
        return (taskDif || dif.station_ids || dif.antenna_mode || dif.clock_frequency || dif.filter_type);
    }
    return taskDif;
}

QString StationTask::diffString(const task_diff &dif) const {
    QString difstr(Task::diffString(dif));
    if (!difstr.isEmpty()) difstr += ",";

    if (dif.station_ids) difstr += QString(SAS_item_names[TP_STATION_IDS]) + ",";
    if (dif.antenna_mode) difstr += QString(SAS_item_names[TP_ANTENNA_MODE]) + ",";
    if (dif.clock_frequency) difstr += QString(SAS_item_names[TP_CLOCK_FREQUENCY]) + ",";
    if (dif.filter_type) difstr += QString(SAS_item_names[TP_FILTER_TYPE]) + ",";

    if (difstr.endsWith(',')) {
        difstr.remove(difstr.length()-1,1);
    }

    return difstr;
}
