//
// Copyright (C) 2002
// ASTRON (Netherlands Foundation for Research in Astronomy)
// P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

// $ID$


#include <uvplot/UVPUVCoverageArea.h>

#include <qpainter.h>
#include <qimage.h>



#if(DEBUG_MODE)
#include <cassert>
#endif




//===============>>>  UVPUVCoverageArea::UVPUVCoverageArea  <<<===============

UVPUVCoverageArea::UVPUVCoverageArea(QWidget*            parent,
                                     const UVPImageCube* data)
  : UVPDisplayArea(parent),
    itsCurrentImage(data)
{
}




//===============>>>  UVPUVCoverageArea::~UVPUVCoverageArea  <<<===============

UVPUVCoverageArea::~UVPUVCoverageArea()
{
}




//====================>>>  UVPUVCoverageArea::setData  <<<====================

void UVPUVCoverageArea::setData(const UVPImageCube* data)
{
  itsCurrentImage = data;
}






//==============>>>  UVPUVCoverageArea::slot_paletteChanged  <<<==============

void UVPUVCoverageArea::slot_paletteChanged()
{
  drawView();
}








//====================>>>  UVPUVCoverageArea::drawView  <<<====================

void UVPUVCoverageArea::drawView()
{
  unsigned int nx =itsCurrentImage->getN(UVPImageCube::X);
  unsigned int ny =itsCurrentImage->getN(UVPImageCube::Y);
  
  
  QPainter buffer_painter;
  
  buffer_painter.begin(&itsBuffer);
  
  
  for(unsigned int x = 0; x < nx; x++) {
    for(unsigned int y = 0; y < ny; y++) {
      int val = int(128.0 + 127.0* *(itsCurrentImage->getPixel(x, y)->getAverageValue()));
      buffer_painter.setPen(*getColor(val));
      buffer_painter.drawPoint(x, y);
    }
  }
  
  
  
  
  
  buffer_painter.setPen(red);
  
  buffer_painter.drawLine(0, 0, width(), height());
  buffer_painter.drawLine(0, height(), width(), 0);
  
  buffer_painter.end();
  
  bitBlt(this, 0, 0, &itsBuffer);
}
