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

/**
 * Title:        class GraphDiagram<p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      Astron<p>
 * @author Robbert Dam
 * @version 1.0
 */

import javax.swing.*;
import java.util.*;
import java.awt.*;
import java.io.*;
import org.astron.util.*;
import org.astron.util.gui.*;

/**
 * A GraphDiagram is a collection of GraphComposites and Connections.
 *
 * Notes:
 * It should auto-layout the Graphs based on the Connections.
 * It should provide different states in which diagrams can be displayed.
 */
public class GraphDiagram extends GraphSimul {
  ArrayList _connectionList;

  public GraphDiagram(GraphManager manager) {
    super (manager,"GraphDiagram"+followUp);
    defaultSettings();
  }

  public GraphDiagram(GraphManager manager,String name) {
    super (manager,name);
    defaultSettings();
  }

  private void defaultSettings() {
    setBackground(Color.white);
    setBaseRoundSize(0);
    getDimCtrl().setSpacing(10,100);
  }

  /** Overloaded from Graph.canBeDrawn(). Always returns true. */
  public boolean canBeDrawn() { return true; }

  /** Overloaded for performance measurements */
  public void layoutAll() {
    Date date1 = new Date();
    super.layoutAll();
    Date date2 = new Date();
    long run = date2.getTime() - date1.getTime();
    manager.fireMessage(new MessageGraph(this,"Auto-layout completed ("
                     + run + " ms)"));
  }

  public void setSelected(boolean selected) { /* cannot select root */ }

  /** Stores Graphics2D state and sets it back after drawing is completed */
  public boolean paint ()
  {
    Graphics2D g = manager.getGraphics();
    Stroke stroke = g.getStroke();
    Paint paint = g.getPaint();
    boolean b = super.paint();
    g.setPaint(paint);
    g.setStroke(stroke);
    return b;
  }

  public void paintConnections () {
    Graphics2D g = manager.getGraphics();
    Stroke stroke = g.getStroke();
    Paint paint = g.getPaint();
    super.paintConnections();
    g.setPaint(paint);
    g.setStroke(stroke);
  }
}