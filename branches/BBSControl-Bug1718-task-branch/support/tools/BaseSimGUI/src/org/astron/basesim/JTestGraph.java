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
 * Title:        <p>
 * Description:  <p>
 * Copyright:    Copyright (c) <p>
 * Company:      <p>
 * @author
 * @version 1.0
 */

import java.util.*;
import java.awt.*;
import javax.swing.*;
import java.awt.event.*;

/**
 * Java swing component that can display a single Graph or a complex
 * hierarchy of GraphComposites.
 */
public class JTestGraph extends JComponent {

  GraphComposite composite;
  GraphManager man;

  public JTestGraph () {
    man = new GraphManager();
    composite = new GraphComposite(man,"root composite");
    composite.setLayoutController(new LayoutFlow(LayoutFlow.ALIGN_VERT));
    composite.getDimCtrl().setSpacing(80,10);
    composite.getDimCtrl().setSpacingFactor(1,false);
    composite.setLineColor(Color.black);
    composite.addGraph(new GraphWorkHolder(man));
    composite.addGraph(new GraphWorkHolder(man));
    composite.enableBorder(true);
    GraphComposite innerComposite = new GraphComposite(man,"inner composite");
    innerComposite.enableBorder(true);
    innerComposite.getDimCtrl().setSpacing(20,20);
    composite.addGraph(innerComposite);
    Graph graph2 = new Graph(man,0,0,"Graph2");
    graph2.setBackground(Color.orange);
    graph2.setBaseSize(new Dimension(200,150));
    graph2.setRoundSize(16);
    Graph graph3 = new Graph(man,0,0,"Graph3");
    graph3.setBackground(Color.blue);
    graph3.setBaseSize(new Dimension(150,180));
    graph3.setRoundSize(34);
    graph3.setVisible(true);
    graph2.setVisible(true);

    innerComposite.addGraph(graph2);
    innerComposite.addGraph(graph3);
    innerComposite.setLayoutController(new LayoutFlow(LayoutFlow.ALIGN_HORZ));
    Graph graph = new Graph(man,0,0,"Graph object");
    graph.setVisible(true);
    graph.setBackground(Color.yellow);
    graph.setRoundSize(10);
    graph.setBaseSize(new Dimension(100,100));
    graph.connectTo(graph2);
    graph2.connectTo(graph3);
    composite.addGraph(graph);
    composite.addGraph(new GraphWorkHolder(man));
    composite.setZoomLevel(1);
    composite.enableBorder(true);
    //graph2.setVisible(false);
    composite.layoutAll();
  }

  protected void paintComponent(Graphics g) {
    g.drawString("Test of GraphComposite",20,20);
    man.setGraphics((Graphics2D)g);
    composite.paint();
    composite.paintConnections();
  }

  private void testVisibility() {
  }
}
