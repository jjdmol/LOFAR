/*
 * FileUtils.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 19 Apr 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/FileUtils.cpp $
 *
 */

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>

#include "FileUtils.h"

FileUtils::FileUtils() {
	// TODO Auto-generated constructor stub

}

FileUtils::~FileUtils() {
	// TODO Auto-generated destructor stub
}

/*!
   Delete a directory along with all of its contents.

   \param dirName Path of directory to remove.
   \return true on success; false on error.
*/

bool FileUtils::removeDir(const QString &dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }

    return result;
}
