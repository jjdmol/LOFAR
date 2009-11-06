//#  DH_OTDBlog.h: DataHolder for OTDB logmessages
//#
//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_DH_OTDBLOG_H
#define LOFAR_ACCBIN_DH_OTDBLOG_H

// \file
// DataHolder for OTDB log messages

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <Blob/BlobIStream.h>
#include <Blob/BlobOStream.h>
#include <Transport/DataHolder.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {
// \addtogroup ALC
// @{


//# Description of class.
// The DH_OTDBlog class is responsible for packing and unpacking
// log messages that are exchanged between the ApplController and
// the OTDBlogger.
class DH_OTDBlog : public DataHolder
{
public:
	// Constructor
	DH_OTDBlog();

	// Destructor
	virtual ~DH_OTDBlog();

	// \name libTransport methods
	// @{ 

	// Register the fixed size variables to the dataholderblob.
	virtual void 	init();

	// Tries to fill its buffer with new data. Returns \c true is a complete
	// message is received.
	void	unpack();

	// Write the current contents to the network.
	void	pack();

	// The clone function is neccesary to meet the libTransport requirements,
	// it copies everything but the data. Something we never need.
	DH_OTDBlog*		clone()        const;
	// @}

	// \name Additional methods
	// @{

	// \c makeDataCopy is the counterpart of clone: it copies the data.
	DH_OTDBlog*		makeDataCopy() const;

	// @}

	// \name Data-accessor methods
	// @{

	void	setMessages		(const string&		theMsgBuffer);
	string	getMessages		() const;
	// @}

private:
	// Copying is not allowed this way.
	DH_OTDBlog(const DH_OTDBlog& that);

	// Copying is not allowed this way.
	DH_OTDBlog& 	operator=(const DH_OTDBlog& that);

	// Implement the initialisation of the pointers
	virtual void	fillDataPointers();

	//# --- DataMembers ---
	uint16		*itsVersionNumber;
	string		itsMessageBuffer;
};

//#
//# setMessages(messages)
//#
inline void	DH_OTDBlog::setMessages		(const string&		theMessages)
{
	itsMessageBuffer = theMessages;
}

//#
//# getMessages()
//#
inline string	DH_OTDBlog::getMessages		() const
{
	return (itsMessageBuffer);
}

// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
