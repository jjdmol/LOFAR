// Copyright notice should go here

// $ID$

#include <UVPDisplayArea.h>
#include <qpainter.h>

#if(DEBUG_MODE)
#include <cassert>
#endif



//==================>>>  UVPDisplayArea::UVPDisplayArea  <<<==================

UVPDisplayArea::UVPDisplayArea(QWidget *parent,
                               int      numColors)
  : QWidget(parent),
    itsColormap(numColors),
    itsXAxis(1, 0, "X", "arbitrary"),
    itsYAxis(1, 0, "Y", "arbitrary")
{
  // Uncomment: Don't blank window before repainting
  setBackgroundMode(NoBackground);
  
  setMouseTracking(true);

  itsBuffer.resize(10, 10);

  for(unsigned int i = 0; i < itsColormap.size(); i++)
    {
      itsColormap[i] = QColor(qRgb(i, i, i), i);
    }
  initColormap(1.0, 128.0);
  
}





//==================>>>  UVPDisplayArea::~UVPDisplayArea  <<<==================

UVPDisplayArea::~UVPDisplayArea()
{
}





//===================>>>  UVPDisplayArea::initColormap  <<<===================

void UVPDisplayArea::initColormap(double slope,
                                  double center)
{
  const int min_color = 0;
  const int max_color = 255;

  for(unsigned int i = 0; i < itsColormap.size(); i++) {
    double col  = (max_color-min_color)/2 + slope*(double(i)-center);
    if(col < min_color) {
      col = min_color;
    }
    if(col > max_color) {
      col = max_color;
    }
    
    col = int(col +0.5);

    itsColormap[i].setRgb(col, col, col); 

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
    drawView();
  }
}




//===================>>> UVPDisplayArea::mouseMoveEvent  <<<===================

void UVPDisplayArea::mouseMoveEvent(QMouseEvent *event)
{
  if(event->state() & LeftButton) {
    double slope  = (50.0*event->pos().y())/height();
    double center = (255.0*event->pos().x())/width();
    
    initColormap(slope, center);
    drawView();
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
