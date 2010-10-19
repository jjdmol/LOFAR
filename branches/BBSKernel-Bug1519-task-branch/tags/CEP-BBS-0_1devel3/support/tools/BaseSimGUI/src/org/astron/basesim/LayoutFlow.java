//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//////////////////////////////////////////////////////////////////////

package org.astron.basesim;

import java.awt.*;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c)
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

/**
 *  The LayoutFlow is a layout type that is similar to the Java AWT
 *  FlowLayout. It places the Graphs that the GraphComposite contains next
 *  the each other like characters in a line of text. In addition it can
 *  align the Graph both horizontally an vertically.
 */
public class LayoutFlow implements LayoutController {

  static int ALIGN_VERT = 0;
  static int ALIGN_HORZ = 1;

  private GraphComposite _container;
  private int _align;

  /** Create a new LayoutFlow. align must be ALIGN_VERT or ALIGN_HORZ */
  public LayoutFlow (int align) {
    _align = align;
  }

  public void setGraphComposite(GraphComposite container) {
    _container = container;
  }
  public boolean canBeDone() {
    return true;
  }
  public void preprocess() {
    // no preprocessing
  }
  public void doLayout() {
    int nrOfGraphs = _container.numberOfGraphs();
    if (nrOfGraphs == 0) { return; }
    int horzSpacing = _container.getDimCtrl().getHorizontalSpacing();
    int vertSpacing = _container.getDimCtrl().getVerticalSpacing();
    int maxHeight, maxWidth;
    if (_align == ALIGN_HORZ) {
      int x=(int)(horzSpacing*_container.getDimCtrl().getSpacingFactor(true));
      maxHeight = _container.maxHeightGraph();
      for (int i=0; i<nrOfGraphs; i++)
      {
        Graph graph = _container.getGraph(i);
        if (graph.isVisible()) {
          Dimension d = graph.getBaseSize();
          graph.setBaseX(x);
          graph.setBaseY((int)((maxHeight/2)-(d.height/2)
                         + _container.getDimCtrl().getSpacingFactor(false)
                         * vertSpacing));
          x += horzSpacing+d.width;
        }
      }
    } else if (_align == ALIGN_VERT) {
      int y=(int)(vertSpacing*_container.getDimCtrl().getSpacingFactor(false));
      maxWidth = _container.maxWidthGraph();
      for (int i=0; i<nrOfGraphs; i++)
      {
        Graph graph = _container.getGraph(i);
        if (graph.isVisible()) {
          Dimension d = graph.getBaseSize();
          graph.setBaseX((int)((maxWidth/2)-(d.width/2))
                         + (int)(_container.getDimCtrl().getSpacingFactor(true)
                         * horzSpacing));
          graph.setBaseY(y);
          y += vertSpacing+d.height;
        }
      }
    } else throw new java.lang.UnsupportedOperationException();
  }
}