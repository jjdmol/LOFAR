/*
 * DataHandler.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 5-feb-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/DataHandler.h $
 *
 */

#ifndef DATAHANDLER_H_
#define DATAHANDLER_H_

#include <string>
#include "Controller.h"

class DataHandler {
public:
	DataHandler(const Controller *);
	virtual ~DataHandler();
	bool saveSettings(const QString &filename) const;
	bool loadSettings(const QString &filename);
	bool loadProgramPreferences(void);
    bool saveProgramPreferences(void);

	// getters
    const QString &getFileName(void) const { return itsFileName; }

	// setters
    void setFileName(QString const &fname) { itsFileName = fname; }

private:
	QString itsFileName;
	const Controller *itsController;
};

#endif /* DATAHANDLER_H_ */
