// Copyright notice should go here

// $ID$


#include <UVPUVCoverageArea.h>

#include <qpainter.h>
#include <qimage.h>



#if(DEBUG_MODE)
#include <cassert>
#endif




//===============>>>  UVPUVCoverageArea::UVPUVCoverageArea  <<<===============

UVPUVCoverageArea::UVPUVCoverageArea(QWidget*            parent,
                                     const UVPImageCube* data=0)
  : UVPDisplayArea(parent),
    itsCurrentImage(data)
{
}




//===============>>>  UVPUVCoverageArea::~UVPUVCoverageArea  <<<===============

UVPUVCoverageArea::~UVPUVCoverageArea()
{
}




//====================>>>  UVPUVCoverageArea::setData  <<<====================

void UVPUVCoverageArea::setData(const UVPImageCube* data=0)
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
  
  
  for(int x = 0; x < nx; x++) {
    for(int y = 0; y < ny; y++) {
      int val = 128.0 + 127.0* *(itsCurrentImage->getPixel(x, y)->getAverageValue());
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
