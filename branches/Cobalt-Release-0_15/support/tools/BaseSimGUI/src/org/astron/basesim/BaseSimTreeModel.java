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
 * Title: Class BaseSimTreeModel
 * Description: TreeModel that maps a Graph composite tree to a BaseSim tree.
 * Copyright:    Copyright (c)
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

import java.util.Vector;
import javax.swing.tree.*;
import javax.swing.event.*;


/**
 * Model that a JTree calls on to draw its nodes. This model can also be
 * used to translate a Graph hierarchy to a LofarSim hierarchy. The difference
 * between these two is the composition of a Step.
 */
public class BaseSimTreeModel implements TreeModel {
  String altRoot = new String ("No diagram loaded");
  GraphDiagram _diagram;
  private Vector treeModelListeners = new Vector();

  public BaseSimTreeModel(GraphDiagram diagram) {
    _diagram = diagram;
  }
  public void setGraphDiagram(GraphDiagram diagram) {
    GraphDiagram oldDiagram = _diagram;
    _diagram = diagram;
    if (oldDiagram == null) fireTreeStructureChanged(altRoot);
    else fireTreeStructureChanged(oldDiagram);
  }
  public GraphDiagram getGraphDiagram() { return _diagram; }
  public Object getRoot() {
    if (_diagram != null) return _diagram;
    else return altRoot;
  }
  public Object getChild(Object parent, int index) {
    if (parent instanceof GraphSimul) {
      GraphSimul gSimul = (GraphSimul)parent;
      if ((index == 0) && gSimul.hasWorkHolder()) {
        // return WorkHolder (if it has one)
        return gSimul.getWorkHolder();
      }
      GraphComposite gComp = (GraphComposite)gSimul.getGraph(1);
      if (gSimul.hasWorkHolder()) return gComp.getGraph(index-1);
      else return gComp.getGraph(index);
    }
    if (parent instanceof GraphStep) {
      // a Step has 0 or 1 WorkHolder(s)
      GraphStep gStep = (GraphStep)parent;
      return gStep.getWorkHolder();
    }
    if (parent instanceof GraphWorkHolder) {
      // pretend that the Workholder contains the DataHolders
      GraphWorkHolder gWorkHolder = (GraphWorkHolder)parent;
      GraphStep gStepParent = (GraphStep)gWorkHolder.getOwner().getOwner();
      GraphComposite inDhComp =
        (GraphComposite)gStepParent.getGraph(0);      // input GraphDataHolders
      GraphComposite outDhComp =
        (GraphComposite)gStepParent.getGraph(2);      // output GraphDataHolders
      if (inDhComp.numberOfGraphs() > index) {
        return inDhComp.getGraph(index);
      } else return outDhComp.getGraph(index-inDhComp.numberOfGraphs());
    }
    System.err.println("Cannot resolve child (" + parent
                      + ", index "+index+") in BaseSimTreeModel.getChild()");
    return null;
  }

  public int getChildCount(Object parent) {
    int count=0;
    if (parent instanceof GraphSimul) {
      GraphSimul gSimul = (GraphSimul)parent;
      count += gSimul.numberOfSteps();
      count += (gSimul.hasWorkHolder()) ? 1 : 0;
      return count;
    }
    if (parent instanceof GraphStep) {
      GraphStep gStep = (GraphStep)parent;
      return  (gStep.hasWorkHolder()) ? 1 : 0;
    }
    if (parent instanceof GraphWorkHolder) {
      GraphWorkHolder gWorkHolder = (GraphWorkHolder)parent;
      GraphStep gStepParent = (GraphStep)gWorkHolder.getOwner().getOwner();
      count += gStepParent.nrInputDataHolders();
      count += gStepParent.nrOutputDataHolders();
      return count;
    }
    return 0;
  }

  public boolean isLeaf(Object node) {
    if (node instanceof GraphWorkHolder) return false;
    if (node instanceof GraphComposite) return false;
    return true;
  }

  public void valueForPathChanged(TreePath path, Object newValue) {
    /**@todo: Implement this javax.swing.tree.TreeModel method*/
    throw new java.lang.UnsupportedOperationException("Method valueForPathChanged() not yet implemented.");
  }

  public int getIndexOfChild(Object parent, Object child) {
    if (parent instanceof GraphSimul) {
      GraphSimul gSimul = (GraphSimul)parent;
      if (child instanceof GraphWorkHolder) return 0;
      for (int i=0; i<gSimul.numberOfSteps(); i++) {
        if (child == gSimul.getStep(i)) {
          return (gSimul.hasWorkHolder()) ? i+1 : i;
        }
      }
    }
    if (parent instanceof GraphStep) return 0;
    if (parent instanceof GraphWorkHolder) {
      GraphWorkHolder gWorkHolder = (GraphWorkHolder)parent;
      GraphStep gStepParent = (GraphStep)gWorkHolder.getOwner().getOwner();
      GraphDataHolder gInDH[] = gStepParent.getInputDataHolders();
      for (int i=0; i<gInDH.length; i++) {
        if (gInDH[i] == child) return i;
      }
      GraphDataHolder gOutDH[] = gStepParent.getOutputDataHolders();
      for (int i=0; i<gOutDH.length; i++) {
        if (gOutDH[i] == child) return i+gInDH.length;
      }
    }
    System.err.println("Child not found in BaseSimTreeModel.getIndexOfChild()");
    return -1;
  }

  public void addTreeModelListener(TreeModelListener l) {
    treeModelListeners.add(l);
  }

  public void removeTreeModelListener(TreeModelListener l) {
    treeModelListeners.remove(l);
  }

  protected void fireTreeStructureChanged(Object oldRoot) {
    int len = treeModelListeners.size();
    TreeModelEvent e = new TreeModelEvent(this,new Object[] {oldRoot});
    for (int i = 0; i < len; i++) {
      ((TreeModelListener)treeModelListeners.elementAt(i)).
      treeStructureChanged(e);
    }
  }

}