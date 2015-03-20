/*
 * FileUtils.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 19 Apr 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/FileUtils.h $
 *
 */

#ifndef FILEUTILS_H_
#define FILEUTILS_H_

#include <QString>

class FileUtils {
public:
	FileUtils();
	virtual ~FileUtils();

	bool removeDir(const QString &dirName);
};

#endif /* FILEUTILS_H_ */
