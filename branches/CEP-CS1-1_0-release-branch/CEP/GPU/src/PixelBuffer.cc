//#  PixelBuffer.cc: Allocates a pixelbuffer in the X environment
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

#include <stdlib.h>				// exit()
#include "PixelBuffer.h"
#include "GPUEnvironment.h"

#define GLX_FLOAT_COMPONENTS_NV         0x20B0

PixelBuffer::PixelBuffer(const int		nrPlanes,
						 const int		workspace) :
	itsNrPlanes(nrPlanes)
{
	static int 			iScreen;
	static GLXFBConfig	*glxConfig[4];		// for float1 .. float4
	static const int pbAttribList[] =  {
		GLX_PRESERVED_CONTENTS, 	true,
		GLX_PBUFFER_WIDTH, 			workspace,
		GLX_PBUFFER_HEIGHT, 		workspace,
		0,
	};
	static bool initialized = false;

	if (initialized) {
	    // XXX: For some reason, I can't deal 
		// with changing the pbuffer size on Linux... just return
		itsNrPlanes = nrPlanes;
		return;
	}

	if (!initialized) {
		int iConfigCount;   
		int pfAttribList[] = {
           GLX_RED_SIZE,               0,
           GLX_GREEN_SIZE,             0,
           GLX_BLUE_SIZE,              0,
           GLX_ALPHA_SIZE,             0,
           GLX_STENCIL_SIZE,           0,
           GLX_DEPTH_SIZE,             0,
           GLX_FLOAT_COMPONENTS_NV,    true,
           GLX_DRAWABLE_TYPE,          GLX_PBUFFER_BIT,
           0,
        };

		itsDisplay = XOpenDisplay(NULL);
		iScreen    = DefaultScreen(itsDisplay);

		// initialize glxConfig structs for float1 .. float4 types
		for (int i=0; i<4; i++) {
			for (int j=0; j<4; j++) {
				pfAttribList[1+j*2] = (j<=i)?32:0;
			}
       
	 		glxConfig[i] = glXChooseFBConfig(itsDisplay, 
											 iScreen, 
											 pfAttribList, 
											 &iConfigCount);
       
			if (!glxConfig[i][0]) {
				fprintf(stderr, "NV30GL:  glXChooseFBConfig() failed\n");
				exit(1);		// JUST FOR NOW
			}        
		}
	}
  
	// already a Pbuffer? release contents
	if (initialized) {
		glXMakeCurrent(itsDisplay, None, NULL);
		glXDestroyPbuffer(itsDisplay, itsGlxPbuffer);
	}

	// create Pbuffer structure
	if (!(itsGlxPbuffer = glXCreatePbuffer(itsDisplay, 
									 glxConfig[nrPlanes-1][0], 
									 pbAttribList))) {
   		fprintf(stderr, "glXCreatePbuffer() failed\n");
		exit(1);		// JUST FOR NOW
	}
  
//	if (!initialized) {
   	if (!(itsGlxContext = glXCreateNewContext(itsDisplay, 
                                      glxConfig[nrPlanes-1][0], 
                                      GLX_RGBA_TYPE, 
                                      0, true))) {
		fprintf(stderr, "glXCreateNewContext() failed\n");
		exit (1);		// JUST FOR NOW
	}
//	}
     
	glXMakeCurrent(itsDisplay, itsGlxPbuffer, itsGlxContext);
	
	itsNrPlanes = nrPlanes;
	initialized = true;
}


PixelBuffer::~PixelBuffer() {
	glXMakeCurrent(itsDisplay, None, NULL);
	if (itsGlxContext) {
		glXDestroyContext(itsDisplay, itsGlxContext);
		itsGlxContext = 0;
	}
	if (itsGlxPbuffer) {
		glXDestroyPbuffer(itsDisplay, itsGlxPbuffer);
		itsGlxPbuffer = 0;
	}
}
