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
  if(data != 0) {
    itsCurrentQImage = new QImage(data->getN(UVPImageCube::X),
                                  data->getN(UVPImageCube::Y), 32);
  } else {
    itsCurrentQImage = 0;
  }
}




//===============>>>  UVPUVCoverageArea::~UVPUVCoverageArea  <<<===============

UVPUVCoverageArea::~UVPUVCoverageArea()
{
  if(itsCurrentQImage != 0) {
    delete itsCurrentQImage;
  }
}




//====================>>>  UVPUVCoverageArea::setData  <<<====================

void UVPUVCoverageArea::setData(const UVPImageCube* data=0)
{
  if(itsCurrentQImage != 0) {
    delete itsCurrentQImage;
  }

  itsCurrentImage = data;

  if(data != 0) {
    itsCurrentQImage = new QImage(data->getN(UVPImageCube::X),
                                  data->getN(UVPImageCube::Y), 32);
  } else {
    itsCurrentQImage = 0;
  }

}






//==============>>>  UVPUVCoverageArea::slot_paletteChanged  <<<==============

void UVPUVCoverageArea::slot_paletteChanged()
{
  if(itsCurrentQImage != 0) {
    if(itsCurrentQImage->depth() < 32) {
      itsCurrentQImage->setNumColors(getNumberOfColors());
      
      for(int i = 0; 
          i < itsCurrentQImage->numColors() && i < getNumberOfColors();
          i++) {
        itsCurrentQImage->setColor(i, getColor(i)->rgb());
      }
    }
  }

  drawView();
}








//====================>>>  UVPUVCoverageArea::drawView  <<<====================

void UVPUVCoverageArea::drawView()
{
  if(itsCurrentQImage != 0) {

    unsigned int nx =itsCurrentImage->getN(UVPImageCube::X);
    unsigned int ny =itsCurrentImage->getN(UVPImageCube::Y);

    
    QPainter buffer_painter;
    
    buffer_painter.begin(&itsBuffer);
    
    //    buffer_painter.drawImage(0, 0, *itsCurrentQImage);

    for(int x = 0; x < nx; x++) {
      for(int y = 0; y < ny; y++) {
        int val = 128.0 + 127.0* *(itsCurrentImage->getPixel(x, y)->getAverageValue());
        //        itsCurrentQImage->setPixel(x,
        //                         y,
        //                         getColor(val)->rgb());
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

}
