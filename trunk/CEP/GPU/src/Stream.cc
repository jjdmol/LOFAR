//#  Stream.cc: Class for handling and hiding textures.
//#
//#  Copyright (C) 2004
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
//#  $Id$

#include "GPUEnvironment.h"
#include "Stream.h"

int				gNrStreamsInUse;
streamCoord 	gStreamStack[STREAM_STACK_SIZE];

//
// Constructor
//
Stream::Stream(const int			width, 
			   const int			height, 
			   const StreamType		type, 
			   void*				buffer) :
	itsWidth(width),
	itsHeight(height),
	itsType(type),
	itsBuffer(buffer)
{
	glGenTextures(1, &itsId);						// let GL generate a unique ID
	glActiveTextureARB(GL_TEXTURE0_ARB+15);			// deselect current texture
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, itsId);	// define texture type
													// alloced memory for texture
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 34955, itsWidth, itsHeight, 0, itsType, GL_FLOAT, NULL);

	checkGL();										// check for errors

	// NOTE GL_RGBA SHOULD DEPEND ON TYPE!!!
}
	
Stream::~Stream()
{ }

void Stream::writeData()
{
	glActiveTextureARB(GL_TEXTURE0_ARB);			// link texture to ARB0
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, itsId);	// and copy local RAM to Vcard
	glTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, itsWidth, itsHeight, itsType, GL_FLOAT, itsBuffer);

	checkGL();										// check for errors

	// NOTE GL_RGBA SHOULD DEPEND ON TYPE!!!
}

void Stream::readData() {
	glActiveTextureARB(GL_TEXTURE0_ARB);			// link texture to ARB0
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, itsId);	// and copy Vcard to local RAM
	glGetTexImage(GL_TEXTURE_RECTANGLE_NV, 0, itsType, GL_FLOAT, itsBuffer);

	checkGL();										// check for errors

	// NOTE GL_RGBA SHOULD DEPEND ON TYPE!!!
}

void Stream::readScreen() {
	glActiveTextureARB(GL_TEXTURE0_ARB);			// link texture to ARB0
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, itsId);	// copy screen to local RAM
	glReadPixels(0, 0, itsWidth, itsHeight, itsType, GL_FLOAT, itsBuffer);

	checkGL();										// check for errors

	// NOTE GL_RGBA SHOULD DEPEND ON TYPE!!!
}

void Stream::copyScreen() {
	glActiveTextureARB(GL_TEXTURE0_ARB);			// link texture to ARB0
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, itsId);	// and copy screen to Vcard
	glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, itsWidth, itsHeight);

	checkGL();										// check for errors
}

void Stream::use() {
	glActiveTextureARB(GL_TEXTURE0_ARB+gNrStreamsInUse);	// link texture to ARBn
	glBindTexture (GL_TEXTURE_RECTANGLE_NV, itsId);
	checkGL();												// check for errors

	// update stream stack with texture coordinates
	float		halfPixel = 0.5 / getCurrentGPUEnv()->WorkspaceSize();
	if (itsWidth == 1) {
		gStreamStack[gNrStreamsInUse].left  = halfPixel;
		gStreamStack[gNrStreamsInUse].right = halfPixel;
	}
	else {
		gStreamStack[gNrStreamsInUse].left  = 0        - 0.5 + halfPixel;
		gStreamStack[gNrStreamsInUse].right = itsWidth - 0.5 + halfPixel;
	}
	if (itsHeight == 1) {
		gStreamStack[gNrStreamsInUse].bottom= 0 - halfPixel;
		gStreamStack[gNrStreamsInUse].top   = 0 - halfPixel;
	}
	else {
		gStreamStack[gNrStreamsInUse].bottom= 0         - 0.5 + (2*halfPixel);
		gStreamStack[gNrStreamsInUse].top   = itsHeight - 0.5 + (2*halfPixel);
	}
	gNrStreamsInUse++;
}

void clearStreamStack() {
	gNrStreamsInUse = 0;
}

int streamStackSize() {
	return (gNrStreamsInUse);
}

streamCoord* getStreamStack() {
	return (&gStreamStack[0]);
}



