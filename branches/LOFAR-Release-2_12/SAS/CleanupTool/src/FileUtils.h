/*
 * FileUtils.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 19 Apr 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/FileUtils.h $
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
