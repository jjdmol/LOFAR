// Copyright notice should go here

// $ID$

#if !defined(UVPDISPLAYAREA_H)
#define UVPDISPLAYAREA_H


#include <vector>


#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>


class UVPDisplayArea : public QWidget
{
 public:                        /* Public part */

               UVPDisplayArea(QWidget *parent);
              ~UVPDisplayArea();

 void         initColormap(double slope,
                           double center);


  void         drawView();

 protected:                     /* Protected part */

  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);


 private:
  
  std::vector<QColor> itsColormap;
  QPixmap             itsBuffer;


};



#endif // UVPDISPLAYAREA_H
