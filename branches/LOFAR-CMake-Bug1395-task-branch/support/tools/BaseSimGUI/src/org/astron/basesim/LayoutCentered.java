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
//swe
//////////////////////////////////////////////////////////////////////

package org.astron.basesim;

import java.awt.*;

/**
 * Title: Class LayoutCentered
 * Description: Layout manager for centering Graphs in a Graphcomposite
 * Copyright:    Copyright (c)
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

/** This layout controller will place all composites children in the center
 *  of their parent. Typically, the composite should contain 0 or 1 children. */
public class LayoutCentered implements LayoutController {

  private GraphComposite _container;

  public LayoutCentered() {
  }

  public void setGraphComposite(GraphComposite container) {
    _container = container;
  }

  /** Always returns true. */
  public boolean canBeDone() { return true; }

  /** The center layout will not do any preprocessing. */
  public void preprocess() {}

  /** Center all composite Graphs in the main Graph. */
  public void doLayout() {
    int width = _container.maxWidthGraph()
                + (int)(_container.getDimCtrl().getSpacingFactor(true)
                * _container.getDimCtrl().getHorizontalSpacing()) * 2;
    int height = _container.maxHeightGraph()
                + (int)(_container.getDimCtrl().getSpacingFactor(false)
                * _container.getDimCtrl().getVerticalSpacing()) * 2;
    for (int i=0; i<_container.numberOfGraphs(); i++) {
      Graph graph = _container.getGraph(i);
      Dimension d = graph.getBaseSize();
      graph.setBaseX((int)((width/2)-(d.width/2)));
      graph.setBaseY((int)((height/2)-(d.height/2)));
    }
  }
}