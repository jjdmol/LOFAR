//#  GPUEnvironment.cc: Holds all elements that a GPU needs.
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

#include <iostream>
#include <iomanip>
#include "Stream.h"
#include "GPUEnvironment.h"

static GPUEnvironment*	gCurrentGPUEnv;

using namespace std;

//
// check_GL (global function!)
//
// Checks if an GL error has occured and prints the message.
// The errorflag is clear by this call
//
void checkGL() {
	switch (glGetError()) {
	case GL_NO_ERROR:	
		break;
	case GL_INVALID_ENUM:	
		cerr << "GL ERROR: Invalid Enum argument" << endl;
		break;
	case GL_INVALID_VALUE:		
		cerr << "GL ERROR: Invalid argument" << endl;
		break;
	case GL_INVALID_OPERATION:	
		cerr << "GL ERROR: Invalid operation" << endl;
		break;
	case GL_STACK_OVERFLOW:	
		cerr << "GL ERROR: Stack overflow" << endl;
		break;
	case GL_STACK_UNDERFLOW:	
		cerr << "GL ERROR: Stack underflow" << endl;
		break;
	case GL_OUT_OF_MEMORY:	
		cerr << "GL ERROR: Out of memory" << endl;
		break;
	case GL_TABLE_TOO_LARGE:	
		cerr << "GL ERROR: Table too large" << endl;
		break;
	default:
		cerr << "Some unknown GL ERROR has occured" << endl;
	}
}

//
// CgErrorHandler (global function!)
//
void CgErrorHandler() {
	CGerror		error = cgGetError();

	if (error != CG_NO_ERROR) {
		cerr <<  cgGetErrorString(error) << endl;

		if (error == CG_COMPILER_ERROR) {
			cerr << cgGetLastListing(gCurrentGPUEnv->CgContext()) << endl;
		}
	}
}

//
// Constructor
//
GPUEnvironment::GPUEnvironment(const string		vertexProfile,
							   const string		fragmentProfile,
							   const int		workspace) :
	itsCGContext(NULL),
	itsVertexProfile(CG_PROFILE_UNKNOWN),
	itsFragmentProfile(CG_PROFILE_UNKNOWN),
	itsPixelBuffer(4, workspace),
	itsWorkspaceSize(workspace)
{
	// First install errorhandler
	gCurrentGPUEnv = this;
	cgSetErrorCallback(CgErrorHandler);

	// Create context
	if (!itsCGContext) {
		itsCGContext = cgCreateContext ();
	}

	// Initialize profiles and compiler options
	if (!vertexProfile.empty()) {
		itsVertexProfile = cgGetProfile(vertexProfile.c_str());
		cgGLSetOptimalOptions(itsVertexProfile);
		cgGLEnableProfile (itsVertexProfile);
	}
	if (!fragmentProfile.empty()) {
		itsFragmentProfile = cgGetProfile(fragmentProfile.c_str());
		cgGLSetOptimalOptions(itsFragmentProfile);
		cgGLEnableProfile (itsFragmentProfile);
	}

	// initiate pixelbuffer to 'draw' in.
	glDrawBuffer(GL_FRONT);					// only read and write in
	glReadBuffer(GL_FRONT);					// front pixelBuffer
	glClear(GL_COLOR_BUFFER_BIT);			// clear Pixelbuffer
	checkGL();

	// Define all textures as GL_TEXTURE_RECTANGLE_NV type
	int		maxTextures;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextures);
	for (int i = 0; i < maxTextures; i++) {
		glActiveTextureARB(GL_TEXTURE0_ARB + i);
		glEnable (GL_TEXTURE_RECTANGLE_NV);
	}
	checkGL();

}

//
// Destructor
//
GPUEnvironment::~GPUEnvironment()
{
	if (itsVertexProfile != CG_PROFILE_UNKNOWN) {
		cgGLDisableProfile (itsVertexProfile);
	}
	if (itsFragmentProfile != CG_PROFILE_UNKNOWN) {
		cgGLDisableProfile (itsFragmentProfile);
	}
}

//
// compileProgram
//
// Tries to compile a Cg vertex or fragmentfile.
//
CGprogram GPUEnvironment::compileProgram(const int		programType,
										 const string	fileName,
										 const string	mainRoutine) 
{
	CGprogram		theProgram = NULL;

	if (programType == vertexProg) {
		glEnable(GL_VERTEX_PROGRAM_NV);
		checkGL();

		if (!fileName.empty()) {				// compile the program
			theProgram = cgCreateProgramFromFile (itsCGContext,
												  CG_SOURCE,
												  fileName.c_str(),
												  itsVertexProfile,
												  mainRoutine.c_str(), 0);
			if (!theProgram) {
				cerr << "Can't compile vertexprogram " << fileName << endl;
				return (NULL);
			}
		}
		return (theProgram);
	}

	// Fragment program
	glEnable(GL_FRAGMENT_PROGRAM_NV);
	checkGL();

	if (!fileName.empty()) {				// compile the program
		theProgram = cgCreateProgramFromFile (itsCGContext,
											  CG_SOURCE,
											  fileName.c_str(),
											  itsFragmentProfile,
											  mainRoutine.c_str(), 0);
		if (!theProgram) {
			cerr << "Can't compile fragmentprogram " << fileName << endl;
			return (NULL);
		}
	}
	return (theProgram);
}

//
// makeCurrentGPUEnv
//
// Make this GPUEnv the current one.
//
void GPUEnvironment::makeCurrentGPUEnv() {
	gCurrentGPUEnv = this;
}

//
// Returns a pointer to the current GPUEnvironment
//
GPUEnvironment* getCurrentGPUEnv() {
	return (gCurrentGPUEnv);
}

//
// useProgram
//
// Loads the given program on the graphics card.
//
void GPUEnvironment::useProgram(const CGprogram		theProgram) const {

	cgGLLoadProgram(theProgram);		 // load the program on the card
	cgGLBindProgram(theProgram);		 // Bind the program to the NVcontext
}



//
// info
//
// Shows some information about the GPU environment
//
void GPUEnvironment::info() {
	int		glValue;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &glValue);
	cout << "Max texture size    : " << glValue << endl;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &glValue);
	cout << "Max number textures : " << glValue << endl;
	cout << "OpenGL version      : " << glGetString(GL_VERSION) << endl;
	cout << "Workspace size      : " << itsWorkspaceSize << endl;
//	cout << "Supported extensions: " << glGetString(GL_EXTENSIONS) << endl;
}

//
// executeFragmentTriangle
//
// Draws a triangle of the given size thereby calculating all the
// enclosed pixels
//
void GPUEnvironment::executeFragmentTriangle(int	width, int	height) {
	int				nrStreams = streamStackSize();
	streamCoord*	texSize   = getStreamStack();

	// Draw correlation matrix triangle
	glViewport(0, 0, width, height);
	glBegin(GL_TRIANGLES);
		// bottom left
		for (int i = 0; i < nrStreams; i++) {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, texSize[i].left, texSize[i].bottom);
		}
		glVertex2f(-1.0, -1.0);

		// bottom right
		for (int i = 0; i < nrStreams; i++) {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, texSize[i].right, texSize[i].bottom);
		}
		glVertex2f( 1.0, -1.0);

		// top right
		for (int i = 0; i < nrStreams; i++) {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, texSize[i].right, texSize[i].top);
		}
		glVertex2f( 1.0,  1.0);
	glEnd();
	glFinish();
}

//
// executeFragmentSquare
//
// Draws a square of the given size thereby calculating all the
// enclosed pixels
//
void GPUEnvironment::executeFragmentSquare(int	width, int height) {
	int				nrStreams = streamStackSize();
	streamCoord*	texSize   = getStreamStack();

	// Draw correlation matrix triangle
	glViewport(0, 0, width, height);
	glBegin(GL_QUADS);
		// bottom left
		for (int i = 0; i < nrStreams; i++) {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, texSize[i].left, texSize[i].bottom);
		}
		glVertex2f(-1.0, -1.0);

		// bottom right
		for (int i = 0; i < nrStreams; i++) {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, texSize[i].right, texSize[i].bottom);
		}
		glVertex2f( 1.0, -1.0);

		// top right
		for (int i = 0; i < nrStreams; i++) {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, texSize[i].right, texSize[i].top);
		}
		glVertex2f( 1.0,  1.0);

		// top left
		for (int i = 0; i < nrStreams; i++) {
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB+i, texSize[i].left, texSize[i].top);
		}
		glVertex2f(-1.0,  1.0);
	glEnd();
	glFinish();
}

