//#  PixelBuffer.h: Allocates a pixel buffer in the X environment
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

#ifndef GPU_PIXELBUFFER_H
#define GPU_PIXELBUFFER_H

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

//# Includes
#include <stdio.h>
#define	GL_GLEXT_LEGACY
#define GL_GLEXT_PROTOTYPES
#include <X11/Xlib.h>
#include <GL/gl.h>
#include <GL/glx.h>

//# Forward Declarations
//class forward;



class PixelBuffer {
public:
	PixelBuffer(const int	nrPlanes, const int	workspace);
	~PixelBuffer();

private:
	// copying is not allowed
	PixelBuffer(PixelBuffer& that) {};
	PixelBuffer& operator=(PixelBuffer& that) { return (*this); };

	int				itsNrPlanes;
	Display*	   	itsDisplay;
	GLXPbuffer		itsGlxPbuffer;
	GLXContext		itsGlxContext;
};
#endif
