// Copyright notice should go here

// $ID$

#if !defined(UVPDISPLAYAREA_H)
#define UVPDISPLAYAREA_H


#include <vector>


#include <qwidget.h>
#include <qpixmap.h>




class UVPDisplayArea : public QWidget
{
  Q_OBJECT
    
 public:                        /* Public part */

               UVPDisplayArea(QWidget *parent, int numColors=256);
              ~UVPDisplayArea();

 void          initColormap(double slope,
                            double center);

 unsigned int  getNumberOfColors() const;
 const QColor *getColor(unsigned int color) const;
 

 virtual void drawView();


 signals:

 // Emitted when mouse position changes
 void signal_mouse_world_pos_changed(double x,
                                     double y);
   

 protected:                     /* Protected part */

  QPixmap      itsBuffer;

  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);

 private:
  
  std::vector<QColor> itsColormap;

};



#endif // UVPDISPLAYAREA_H
