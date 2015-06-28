/*
 * lofar_utils.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 26-mrt-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/lofar_utils.cpp $
 *
 */

#include "lofar_utils.h"
#include <iostream> // for std::setprecision()
#include <iomanip> // for std::setprecision()
#include <vector>
#include <algorithm>
#include <QStringList>
#include <sstream>
using std::stringstream;

std::string trimBlanks(std::string const & s)
{
   if(s.empty()) return s;
   int startIndex = s.find_first_not_of(" ");
   if (startIndex != -1) {
	   return s.substr(startIndex, (s.find_last_not_of(" ") - startIndex + 1));
   }
   else return std::string("");
}

std::string int2String(int i, const char *format) {
	std::string str;
	if (format) {
		char buffer [50];
		sprintf (buffer, format, i);
		str = buffer;
	}
	else {
		std::stringstream out;
		out << i;
		str = out.str();
	}
	return str;
}

int string2Int(const std::string &s) {
	int i(0);
	std::istringstream myStream(s);
	myStream >> i;
	return i;
}

std::string double2String(const long double &d, const char *format) {
	std::string str;
	if (format) {
		char buffer [50];
		sprintf (buffer, format, d);
		str = buffer;
	}
	else {
		std::stringstream out;
		out << std::setprecision(16) << d;
		str = out.str();
	}
	return str;
}

long double string2Double(const std::string &s) {
	long double d(0.0);
	std::istringstream myStream(s);
	myStream >> d;
	return d;
}

void write_string(std::ostream& out, const std::string & str)
{
	size_t size = str.length();
	out.write(reinterpret_cast<const char*>(&size), sizeof(size)); // write the length of the string
	if (size)
		out.write(str.c_str(), size); // writes only the characters without terminating null
}

void read_string(std::istream& in, std::string & str)
{
	size_t size;
	in.read(reinterpret_cast<char *>(&size), sizeof(size));
	char * pbuf = new char [size+1];
	if (size)
		in.read(pbuf, size);
	pbuf[size] = '\0';
	str = pbuf;
	delete [] pbuf;
}

void read_string(std::istream& in, QString & str)
{
	size_t size;
	in.read(reinterpret_cast<char *>(&size), sizeof(size));
	char * pbuf = new char [size+1];
	if (size)
		in.read(pbuf, size);
	pbuf[size] = '\0';
	str = pbuf;
	delete [] pbuf;
}

// convert a vector containing unsigned numbers to a string that contains the list of numbers
// e.g. input vector contains 1,2,3,6,8,9 then the output string will be
// '[1..3,6,8,9]' (without the quotes)
QString Vector2StringList(const std::vector<unsigned> &vec) {
	std::vector<unsigned> tmpVec(vec);
	// sort the vector
	std::sort(tmpVec.begin(),tmpVec.end());
	if (!tmpVec.empty()) {
		QString outStr("[");
		unsigned dsstart, dsstop, dsnext;
		size_t idx;
		if (tmpVec.size() >= 2) {
			for (idx = 0; idx < tmpVec.size()-2; ++idx) {
				dsstop = dsstart = tmpVec.at(idx);
				dsnext = tmpVec.at(idx+1);
				while (dsnext == dsstop + 1) {
					dsstop = dsnext;
					if (++idx == tmpVec.size()-1) break; // breaks out of loop if at end of element list
					dsnext = tmpVec.at(idx+1);
				}
				if (dsstop == dsstart) { // single element
					outStr += QString::number(dsstart) + ",";
				}
				else { // range
					outStr += QString::number(dsstart) + ".." + QString::number(dsstop) + ",";
				}
			}

			if (idx < tmpVec.size()-1) { // adds the last two elements if necessary
				outStr += QString::number(tmpVec.at(tmpVec.size()-2)) + ","
				+ QString::number(tmpVec.back());
			}
			else if (idx < tmpVec.size()) { // adds the last element is necessary
				outStr += QString::number(tmpVec.back());
			}
			else { // delete the last comma
				outStr = outStr.left(outStr.size()-1);
			}
		}
		else if (tmpVec.size() == 1) {
			outStr += QString::number(tmpVec.back());
		}
		outStr += "]";
		return outStr;
	}
	else return QString();
}

// convert a list in a single string to a QStringList
std::vector<QString> string2VectorOfStrings(const QString &inputStr, const QChar &separator) {
	std::vector<QString> strList;
	QString tmp;

	if (!inputStr.isEmpty()) {
		tmp = inputStr.trimmed();
		tmp.remove(QChar('"')); // remove all quotes from the string
		tmp.remove(QChar('['));
		tmp.remove(QChar(']'));
		QStringList strQList = tmp.split(separator, QString::SkipEmptyParts);
		for (QStringList::const_iterator it = strQList.begin(); it != strQList.end(); ++it) {
			strList.push_back(*it);
		}
	}
	return strList;
}

// convert a list of integers in a single string (e.g. [1,0,0,1,1] to a vector of bools
std::vector<bool> string2VectorOfBools(const QString &inputStr, const QChar &separator) {
	std::vector<bool> strList;
	QString tmp;

	if (!inputStr.isEmpty()) {
		tmp = inputStr.trimmed();
		tmp.remove(QChar('"')); // remove all quotes from the string
		tmp.remove(QChar('['));
		tmp.remove(QChar(']'));
		QStringList strQList = tmp.split(separator, QString::SkipEmptyParts);
		for (QStringList::const_iterator it = strQList.begin(); it != strQList.end(); ++it) {
			strList.push_back(static_cast<bool>(it->toInt()));
		}
	}
	return strList;
}

QString stringListToVectorString(const QStringList &stringlist, bool noBrackets) {
	if (!stringlist.empty()) {
		QString outputStr;
		if (!noBrackets) {
			outputStr = "[";
		}
		for (int i = 0; i < stringlist.size()-1; ++i) {
			outputStr += stringlist.at(i) + ",";
		}
		outputStr += stringlist.back();
		if (!noBrackets) {
			outputStr += "]";
		}
		return outputStr;
	}
	else {
		if (noBrackets) return QString();
		else return QString("[]");
	}
}

QString boolVector2StringVector(const std::vector<bool> &boolVec) {
	if (!boolVec.empty()) {
		QString outputStr("[");
		for (unsigned i = 0; i < boolVec.size()-1; ++i) {
			outputStr += QString::number(static_cast<int>(boolVec.at(i))) + ",";
		}
		outputStr += QString::number(static_cast<int>(boolVec.back())) + "]";
		return outputStr;
	}
	else return (QString("[]"));
}

std::vector<unsigned> StringList2VectorOfUint(const QString &inputStr, bool &error, bool unique, bool sort) {
	error = false;
	std::vector<unsigned> vec;
	QStringList strList;
	QString tmp;
	if (!inputStr.isEmpty()) {
		tmp = inputStr.trimmed();
		if (tmp.size() > 2) {
			tmp = tmp.remove("[").remove("]");
			strList = tmp.split(',',QString::SkipEmptyParts);
			// we should now have a stringlist containing either individual numbers or a single ranges of numbers expressed as n..m
			int pos;
			unsigned sbstart, sbend, n1,n2;
			for (QStringList::const_iterator it = strList.begin(); it != strList.end(); ++it) {
				if ((pos = it->indexOf("..")) != -1) { // this is a range
					n1 = it->left(pos).toUInt();
					n2 = it->mid(pos+2).toUInt();
					sbstart = std::min(n1,n2);
					sbend = std::max(n1,n2);
					for (unsigned sb = sbstart; sb <= sbend; ++sb) {
						if (unique) {
							if (std::find(vec.begin(), vec.end(), sb) != vec.end()) {
								error = true;
							}
						}
						vec.push_back(sb);
					}
				}
				else {
					if (unique) {
						if (std::find(vec.begin(), vec.end(), it->toUInt()) != vec.end()) {
							error = true;
						}
					}
					vec.push_back(it->toUInt());
				}
			}
		}
	}
	// sort the vector
	if (sort) {
		std::sort(vec.begin(),vec.end());
	}
	return vec;
}


std::string humanReadableUnits(const quint64 &inputsize, bool size_or_bandwidth, const QChar &units) {
	std::stringstream sstr("");
	sstr << std::fixed << std::setprecision(1);
	if (size_or_bandwidth == SIZE_UNITS) {
		if (units == 'M') {
			if (inputsize >= 1073741824) {
				sstr << (float)inputsize / 1073741824;
				return (sstr.str() + " PB");
			}
			else if (inputsize >= 1048576) {
				sstr << (float)inputsize / 1048576;
				return (sstr.str() + " TB");
			}
			else if (inputsize >= 1024) {
				sstr << (float)inputsize / 1024;
				return (sstr.str() + " GB");
			}
			else {
				sstr << inputsize;
				return (sstr.str() + " MB");
			}
		}
		else { // if (units == 'k')
			if (inputsize >= 1073741824) {
				sstr << (float)inputsize / 1073741824;
				return (sstr.str() + " TB");
			}
			else if (inputsize >= 1048576) {
				sstr << (float)inputsize / 1048576;
				return (sstr.str() + " GB");
			}
			else if (inputsize >= 1024) {
				sstr << (float)inputsize / 1024;
				return (sstr.str() + " MB");
			}
			else {
				sstr << inputsize;
				return (sstr.str() + " kB");
			}
		}
	}
	else { // bandwidth units
		if (inputsize >= 1000000000) {
			sstr << (float)inputsize / 1000000000;
			return (sstr.str() + " Tbit/s");
		}
		else if (inputsize >= 1000000) {
			sstr << (float)inputsize / 1000000;
			return (sstr.str() + " Gbit/s");
		}
		else if (inputsize >= 1000) {
			sstr << (float)inputsize / 1000;
			return (sstr.str() + " Mbit/s");
		}
		else {
			sstr << inputsize;
			return (sstr.str() + " kbit/s");
		}
	}
}


std::string humanReadableUnits(const long double &inputsize, bool size_or_bandwidth, const QChar &units) {
	std::stringstream sstr("");
	sstr << std::fixed << std::setprecision(1);
	if (size_or_bandwidth == SIZE_UNITS) {
		if (units == 'M') {
			if (inputsize >= 1073741824.0) {
				sstr << (float)inputsize / 1073741824.0;
				return (sstr.str() + " PB");
			}
			else if (inputsize >= 1048576.0) {
				sstr << (float)inputsize / 1048576.0;
				return (sstr.str() + " TB");
			}
			else if (inputsize >= 1024.0) {
				sstr << (float)inputsize / 1024.0;
				return (sstr.str() + " GB");
			}
			else {
				sstr << inputsize;
				return (sstr.str() + " MB");
			}
		}
		else { // if (units == 'k')
			if (inputsize >= 1073741824.0) {
				sstr << (float)inputsize / 1073741824.0;
				return (sstr.str() + " TB");
			}
			else if (inputsize >= 1048576.0) {
				sstr << (float)inputsize / 1048576.0;
				return (sstr.str() + " GB");
			}
			else if (inputsize >= 1024.0) {
				sstr << (float)inputsize / 1024.0;
				return (sstr.str() + " MB");
			}
			else {
				sstr << inputsize;
				return (sstr.str() + " kB");
			}
		}
	}
	else { // bandwidth units
		if (inputsize >= 1000000000.0) {
			sstr << (float)inputsize / 1000000000.0;
			return (sstr.str() + " Tbit/s");
		}
		else if (inputsize >= 1000000.0) {
			sstr << (float)inputsize / 1000000.0;
			return (sstr.str() + " Gbit/s");
		}
		else if (inputsize >= 1000.0) {
			sstr << (float)inputsize / 1000.0;
			return (sstr.str() + " Mbit/s");
		}
		else {
			sstr << inputsize;
			return (sstr.str() + " kbit/s");
		}
	}
}

