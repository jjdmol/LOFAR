//#  Stream.h: Class for handling and hiding textures
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

#ifndef GPU_STREAM_H
#define GPU_STREAM_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

//# Includes
#include <stdio.h>
#define	GL_GLEXT_LEGACY
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include "GPUEnvironment.h"

#define	STREAM_STACK_SIZE		8

//# Forward Declarations
//class forward;


//
// streamCoord
//
// Helper struct that hold the coordinates to pass to the Vcard.
typedef struct _streamCoord {
	float		left;
	float		right;
	float		bottom;
	float		top;
} streamCoord;

extern int			gNrStreamsInUse;
extern streamCoord 	gStreamStack[STREAM_STACK_SIZE];

//
// Stream
//
class Stream {
public:
	typedef enum StreamType { Float1 = GL_RED, 
							  Float2 = GL_LUMINANCE_ALPHA, 
							  Float3 = GL_RGB, 
							  Float4 = GL_RGBA};

	Stream(const int			width, 
		   const int			height, 
		   const StreamType		type, 
		   void*				buffer);
	~Stream();

	void writeData();		// write data to the texture
	void readData();		// read from the texture
	void readScreen();		// read data from the screen
	void copyScreen();		// copy screen to texture
	void use();				// schedule for next program

private:
	// copying is not allowed
	Stream(Stream& that) {};
	Stream& operator=(Stream& that) { return (*this); };

	GLuint		itsId;
	int			itsWidth;
	int			itsHeight;
	StreamType	itsType;
	void*		itsBuffer;
};

// (related) StreamStack functions
void 			clearStreamStack();
int 			streamStackSize();
streamCoord*	getStreamStack();

#endif

