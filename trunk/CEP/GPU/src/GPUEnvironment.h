//#  GPUEnvironment.h: Holds all the elements a GPU needs.
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

#ifndef GPU_GPU_H
#define GPU_GPU_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

//# Includes
#include <iostream>
#define GL_GLEXT_LEGACY
#define GL_GLEXT_PROTOTYPES
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#include "nv30glext.h"
#include "PixelBuffer.h"

//# Forward Declarations
//class forward;

class GPUEnvironment {
public:
	typedef enum programType { vertexProg = 1, fragmentProg };

	GPUEnvironment(const std::string	vertexProfile, 
				   const std::string	fragmentProfile,
				   const int	workspace);
	~GPUEnvironment();
	void makeCurrentGPUEnv();
	CGcontext	CgContext() 	{ return itsCGContext; };
	int			WorkspaceSize()	{ return itsWorkspaceSize; };

	void info(void) const;
	CGprogram	compileProgram  (const int			programType,
								 const std::string	fileName,
								 const std::string	mainRoutine);
	void useProgram				(const CGprogram	theProgram) const;
	void executeFragmentTriangle(const int	width, const int	height) const;
	void executeFragmentSquare  (const int	width, const int	height) const;
	void executeFragmentVLine   (const int	hpos) const;
	void executeFragmentHLine   (const int	vpos) const;

private:
	CGcontext		itsCGContext;
	CGprofile		itsVertexProfile;
	CGprofile		itsFragmentProfile;
	PixelBuffer		itsPixelBuffer;
	int				itsWorkspaceSize;

	CGenum	getProgramType	(const std::string	fileName) const;
};

void checkGL();
void CgErrorHandler();
// The cgSetErrorCallBack function has a void parameter iso void*
// Therefor we cannot pass a reference to a GPUEnv object.
// A workaround with 'currentGPUEnv' is necessary.
GPUEnvironment*		getCurrentGPUEnv();

typedef struct _float4 {
	float	r, g, b, a;
} float4;

typedef struct _float3 {
	float	x, y, z;
} float3;

typedef struct _float2 {
	float	x, y;
} float2;

#endif
