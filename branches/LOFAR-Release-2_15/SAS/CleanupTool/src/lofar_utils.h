/*
 * lofar_utils.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : Mar 27, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/lofar_utils.h $
 *
 */

#ifndef LOFAR_UTILS_H_
#define LOFAR_UTILS_H_

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <QString>
#include <QStringList>
#include <QDataStream>
#include "lofar_scheduler.h"

#define SIZE_UNITS true
#define BANDWIDTH_UNITS false

std::string trimBlanks(std::string const & s);

template<typename T>
void write_primitive(std::ostream& out, const T& t)
{
  out.write(reinterpret_cast<const char*>(&t), sizeof(T));
}

template<typename T>
void read_primitive(std::istream& in, T& t)
{
  in.read(reinterpret_cast<char*>(&t), sizeof(T));
}

std::string int2String(int i, const char *format = 0);
int string2Int(const std::string &s);
std::string double2String(const long double &d, const char *format = 0);
long double string2Double(const std::string &s);
void write_string(std::ostream& out, const std::string & str);
void read_string(std::istream& in, std::string & str);
void read_string(std::istream& in, QString & str);
QString Vector2StringList(const std::vector<unsigned> &vec);
std::vector<QString> string2VectorOfStrings(const QString &inputStr, const QChar &separator = QChar(','));
std::vector<bool> string2VectorOfBools(const QString &inputStr, const QChar &separator = QChar(','));
QString stringListToVectorString(const QStringList &stringlist, bool noBrackets = false);
QString boolVector2StringVector(const std::vector<bool> &boolVec);
std::vector<unsigned> StringList2VectorOfUint(const QString &inputStr, bool &error, bool unique = true, bool sort = true);
// convert to human readable units. input can be kilobytes 'k' or Megabytes 'M' for sizes (i.e. size_or_bandwidth = SIZE_UNITS.
// for bandwidth only kbit/s can be used as input and the optional parameter units is not used
std::string humanReadableUnits(const long double &kilobytes, bool size_or_bandwidth = SIZE_UNITS, const QChar &units = 'k');
std::string humanReadableUnits(const quint64 &kilobytes, bool size_or_bandwidth = SIZE_UNITS, const QChar &units = 'k');

inline QDataStream & operator>>(QDataStream &in, std::string &str) {
	QString qstr;
	in >> qstr;
	str = qstr.toStdString();
	return in;
}

inline QDataStream & operator<<(QDataStream &out, const std::string &str) {
	out << QString(str.c_str());
	return out;
}


#endif /* LOFAR_UTILS_H_ */
