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
 * Title:
 * Description:
 * Copyright:    Copyright (c)
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

import javax.swing.*;
import java.util.*;

/**
 * This class manages a list of user selections from a JBaseSim diagram.
 * Selections can be reset, added, deleted, etc.
 */
public class SelectionManager {

  private Vector _selection = new Vector();

  private Vector listener = new Vector();

  public SelectionManager() {}

  public void setSelection(Graph graph) {
    unselectAll();
    addSelection(graph);
  }

  public void setSelection(Vector graph) {
    unselectAll();
    for (int i=0; i<graph.size(); i++) {
      addSelection((Graph)graph.get(i));
    }
  }

  public void addSelection(Vector graph) {
    for (int i=0; i<graph.size(); i++) {
      addSelection((Graph)graph.get(i));
    }
 }

  public void addSelection(Graph graph) {
    _selection.add(graph);
    graph.setSelected(true);
    repaintJComponent(graph);
    fireGraphSelected(graph);
  }

  public void unselectAll() {
    Graph tGraph = null;
    for (int i=0; i<_selection.size(); i++) {
      if (i==0) tGraph = getSelected(0);                  // for repainting
      getSelected(i).setSelected(false);                  // unselect all
      fireGraphUnselected(getSelected(i));
    }
    _selection.removeAllElements();
    if (tGraph != null) repaintJComponent(tGraph);
  }

  public void unselect(Vector graph) {
    for (int i=0; i<graph.size(); i++) {
      unselect((Graph)graph.get(i));
    }
  }

  public void unselect(Graph graph) {
    graph.setSelected(false);
    _selection.remove(graph);
    repaintJComponent(graph);
    fireGraphUnselected(graph);
  }

  public void setInverseSelection(Vector graph) {
    Graph g = null;
    unselectAll();
    for (int i=0; i<graph.size(); i++) {
      g = (Graph)graph.get(i);
      if (g.isSelected()) {
        unselect(g);
      } else {
        addSelection(g);
      }
    }
    if (g != null) repaintJComponent(g);
  }

  public void setInverseSelection(Graph graph) {
    if (graph.isSelected()) {
      unselect(graph);
    } else {
      setSelection(graph);
    }
    repaintJComponent(graph);
  }

  public void addInverseSelection(Vector graph) {
    Graph g = null;
    for (int i=0; i<graph.size(); i++) {
      g = (Graph)graph.get(i);
      if (g.isSelected()) {
        unselect(graph);
      } else {
        addSelection(graph);
      }
    }
    if (g != null) repaintJComponent(g);
  }

  public void addInverseSelection(Graph graph) {
    if (graph.isSelected()) {
      unselect(graph);
    } else {
      addSelection(graph);
    }
    repaintJComponent(graph);
  }

  public int nrOfSelections() { return _selection.size(); }

  public Graph getSelected(int index) {
    return (Graph)_selection.get(index);
  }

  private void repaintJComponent(Graph graph) {
    JComponent jComp = graph.getGraphManager().getContainer();
    if (jComp != null) jComp.repaint();
  }

  public void fireGraphSelected(Graph graph) {
    GraphSelectionEvent e = new GraphSelectionEvent(this,graph,true);
    for (int i=0; i<listener.size(); i++) {
      ((GraphSelectionListener)listener.get(i)).valueChanged(e);
    }
  }

  public void fireGraphUnselected(Graph graph) {
    GraphSelectionEvent e = new GraphSelectionEvent(this,graph,false);
    for (int i=0; i<listener.size(); i++) {
      ((GraphSelectionListener)listener.get(i)).valueChanged(e);
    }
  }

  public void addGraphSelectionListener(GraphSelectionListener l) {
    listener.add(l);
  }

  public void removeGraphSelectionListener(GraphSelectionListener l) {
    listener.remove(l);
  }
}