/*
 * DataHandler.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 5-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DataHandler.h $
 *
 */

#ifndef DATAHANDLER_H_
#define DATAHANDLER_H_

#include "schedulerdata.h"

#include <string>

class DataHandler {
public:
	DataHandler(const Controller *);
	virtual ~DataHandler();
//    bool readCSVFile(const QString &filename, SchedulerData &data); // read data from a task file and load in scheduler data objects
    bool writeCSVFile(const QString &filename, const std::vector<const Task *> tasks); // write the task data contained in data to a CSV file
	bool openSchedule(const QString &filename, SchedulerData &data); // read a scheduling project from binary file
	bool saveSchedule(const QString &filename, const SchedulerData &data); // save the scheduling project to file
	void checkTask(Task *task); // checks if the task has enough parameters specified to be scheduled
	bool saveSettings(const QString &filename) const;
	bool loadSettings(const QString &filename);
	bool loadProgramPreferences(void);
	bool saveProgramPreferences(void);

	// getters
    const QString &getFileName(void) const { return itsFileName; }

	// setters
    void setFileName(QString const &fname) { itsFileName = fname; }

private:
    void writeTaskToFile(QTextStream &out, const Task *ptask) ; // Retrieves and writes the details of a task to file
	QString itsFileName;
	const Controller *itsController;
};

#endif /* DATAHANDLER_H_ */
