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

#include <uvplot/UVPDisplayArea.h>
#include <qpainter.h>
#include <cmath>

#if(DEBUG_MODE)
#include <cassert>
#endif



//==================>>>  UVPDisplayArea::UVPDisplayArea  <<<==================

UVPDisplayArea::UVPDisplayArea(QWidget*     parent,
                               unsigned int numColors)
  : QWidget(parent),
    itsComplexColormap(numColors*numColors),
    itsRealIndex(numColors),
    itsImagIndex(numColors),
    itsColormap(numColors),
    itsXAxis(1, 0, "X", "arbitrary"),
    itsYAxis(1, 0, "Y", "arbitrary")
{
  // Uncomment: Don't blank window before repainting
  setBackgroundMode(NoBackground);
  
  setMouseTracking(true);

  itsBuffer.resize(10, 10);

  for(unsigned int i = 0; i < itsColormap.size(); i++) {
    itsColormap[i] = QColor(qRgb(i, i, i), i);
    itsRealIndex[i] = i;
    itsImagIndex[i] = numColors*i;
  }

  for(unsigned int j = 0; j < numColors*numColors; j++) {
    itsComplexColormap[j] = QColor(qRgb(j%numColors, j/numColors, 0), j+numColors);
  }


  initColormap(1.0, numColors/2.0);
  
}





//==================>>>  UVPDisplayArea::~UVPDisplayArea  <<<==================

UVPDisplayArea::~UVPDisplayArea()
{
}





//===================>>>  UVPDisplayArea::initColormap  <<<===================

void UVPDisplayArea::initColormap(double slope,
                                  double center)
{
  const int          min_color = 0;
  const int          max_color = 255;
  const unsigned int numColors = itsColormap.size();

  for(unsigned int i = 0; i < numColors; i++) {
    double col  = (max_color-min_color)/2 + slope*(double(i)-center);
    if(col < min_color) {
      col = min_color;
    }
    if(col > max_color) {
      col = max_color;
    }
    
    int Col = int(col + 0.5);

    itsColormap[i].setRgb(Col, Col, Col); 

  }


  // The complex color table;
  for(unsigned int i = 0; i < numColors; i++) {
    double green  = fabs(min_color + slope*(double(i)-numColors/2));
    if(green < min_color) {
      green = min_color;
    }
    if(green > max_color) {
      green = max_color;
    }
    int Green = int(green + 0.5);
    int Blueim  = 0;
    if( i < numColors>>1) {
      Blueim = Green>>1;
    }

    for(unsigned int r = 0; r < numColors; r++) {
      double red  = fabs(min_color + slope*(double(r)-numColors/2));
      if(red < min_color) {
        red = min_color;
      }
      if(red > max_color) {
        red = max_color;
      }
      int Red = int(red + 0.5);

      int Bluere  = 0;
      if( r < numColors>>1) {
        Bluere = Red>>1;
      }
      itsComplexColormap[itsRealIndex[r]+itsImagIndex[i]].setRgb(Red, Green, Blueim+Bluere);
    }
  }

  emit signal_paletteChanged();
}




//================>>>  UVPDisplayArea::getNumberOfColors  <<<================

unsigned int UVPDisplayArea::getNumberOfColors() const
{
  return itsColormap.size();
}







//====================>>>  UVPDisplayArea::getXAxis  <<<====================

const UVPAxis *UVPDisplayArea::getXAxis() const
{
  return &itsXAxis;
}






//====================>>>  UVPDisplayArea::getYAxis  <<<====================

const UVPAxis *UVPDisplayArea::getYAxis() const
{
  return &itsYAxis;
}




//====================>>>  UVPDisplayArea::setXAxis  <<<====================

void UVPDisplayArea::setXAxis(const UVPAxis &axis)
{
  itsXAxis = axis;
}






//====================>>>  UVPDisplayArea::setYAxis  <<<====================

void UVPDisplayArea::setYAxis(const UVPAxis &axis)
{
  itsYAxis = axis;
}







//==================>>>  UVPDisplayArea::paintEvent  <<<==================

void UVPDisplayArea::paintEvent(QPaintEvent */*event*/)
{
  bitBlt(this, 0, 0, &itsBuffer);
}




//==================>>>  UVPDisplayArea::resizeEvent  <<<==================

void UVPDisplayArea::resizeEvent(QResizeEvent *event)
{
  itsBuffer.resize(event->size());
  itsBuffer.fill(white);
  drawView();
}




//===================>>> UVPDisplayArea::mousePressEvent  <<<==================

void UVPDisplayArea::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == LeftButton) {
    double slope  = (255.0*event->pos().y())/height();
    double center = (255.0*event->pos().x())/width();
    
    initColormap(slope, center);
  }
}




//===================>>> UVPDisplayArea::mouseMoveEvent  <<<===================

void UVPDisplayArea::mouseMoveEvent(QMouseEvent *event)
{
  if(event->state() & LeftButton) {
    double slope  = (255.0*event->pos().y())/height();
    double center = (255.0*event->pos().x())/width();
    
    initColormap(slope, center);
  }else{
  }
  emit signal_mouseWorldPosChanged(itsXAxis.axisToWorld(event->pos().x()),
                                   itsYAxis.axisToWorld(event->pos().y()));
}




//====================>>>  UVPDisplayArea::drawView  <<<====================

void UVPDisplayArea::drawView()
{
  QPainter buffer_painter;


  buffer_painter.begin(&itsBuffer);

  for(int y = 0; y < height(); y++) {
    int val = int(128.0 + 127.0*sin(double(y)/100.0));
    
    buffer_painter.setPen(itsColormap[val]);
    buffer_painter.drawLine(0, y, width(), y);
  }

  buffer_painter.setPen(black);

  buffer_painter.drawLine(0, 0, width(), height());
  buffer_painter.drawLine(0, height(), width(), 0);
  
  buffer_painter.end();

  bitBlt(this, 0, 0, &itsBuffer);
}
