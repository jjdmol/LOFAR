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

#if !defined(UVPDISPLAYAREA_H)
#define UVPDISPLAYAREA_H

// $Id$


#include <vector>


#include <qwidget.h>
#include <qpixmap.h>

#include <uvplot/UVPAxis.h>



class UVPDisplayArea : public QWidget
{
  Q_OBJECT
    
 public:                        /* Public part */

               UVPDisplayArea(QWidget *    parent,
                              unsigned int numColors=256);
              ~UVPDisplayArea();

 void          initColormap(double slope,
                            double center);

 unsigned int  getNumberOfColors() const;
 inline const QColor *getColor(unsigned int color) const;

 const UVPAxis *getXAxis() const;
 const UVPAxis *getYAxis() const;
 
 void  setXAxis(const UVPAxis &axis);
 void  setYAxis(const UVPAxis &axis);


 virtual void drawView();


 signals:

 // Emitted when mouse position changes
 void signal_mouseWorldPosChanged(double x,
                                  double y);
 
 void signal_paletteChanged();

 protected:                     /* Protected part */

  QPixmap      itsBuffer;

  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);

  std::vector<QColor> itsComplexColormap;
  std::vector<int>    itsRealIndex;
  std::vector<int>    itsImagIndex;

 private:
  
  UVPAxis             itsXAxis;
  UVPAxis             itsYAxis;

};


#endif // UVPDISPLAYAREA_H
