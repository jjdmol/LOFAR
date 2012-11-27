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
 * Title: Class LayoutSimul
 * Description: Default layout controller for simulations
 * Copyright:    Copyright (c)
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

import java.util.*;
import org.astron.util.*;
import org.astron.util.gui.*;

/**
 * This layout manager lays out connected Graphs within a GraphComposite in
 * a 'grid' style.
 */
public class LayoutSimul implements LayoutController {

  GraphStep layout[][];
  GraphStep stepBin[];

  private GraphComposite _container;
  /** List of MessageListeners */
  private ArrayList _mlisteners = new ArrayList();

  public LayoutSimul() {
  }
  /** Sets the GraphComposite for which the layout must be determined */
  public void setGraphComposite(GraphComposite container) {
    _container = container;
  }
  /** Returns the result of the preprocess algoritm. Returns null if not
   *  available */
  public GraphStep[][] getLayout() { return layout; }

  /** Determines approximate location of the Simuls within the container
   *  Simul. The result is put in the member variabele layout for further
   *  processing. Other layout mechanism may use this result in their layout
   *  mechanism (like LayoutDataHolder). */
  public void preprocess() {
    int nrOfGraphs = _container.numberOfGraphs();
    if (nrOfGraphs == 0) return;
    createDummyConnections();
    GraphStep flying[] = new GraphStep[nrOfGraphs];
    for (int i=0; i<nrOfGraphs; i++) flying[i]=(GraphStep)_container.getGraph(i);
    GraphStep rootStep[] = new GraphStep[flying.length]; // too big.. but enough!

    // find roots
    int rootStepIx=0;
    for (int i=0; i<flying.length; i++) {
      if (!flying[i].hasInputConnections()) {
        rootStep[rootStepIx++]=flying[i];
        flying[i]=null;
      }
    }

    // create layout array with worst case dimensions (actually, this can be quite spacy)
    // NOTE: layout must be 'vierkant'. LayoutDataHolder relies on that!
    layout = new GraphStep[flying.length][flying.length];
    GraphStep prevStep=null;
    boolean firstRound = true;
    int rootlIxV=0; // index for layout array (root)
    int lIxV=0, lIxH=1; // indices for layout array
    int rootIx;
    int fColumnLength=0;
    int watchDog = 0;
    while (true)
    {
      // tricky way to ensure the program will not hang when the algoritm fails
      if (watchDog++ == 50) {
        fireMessage(new MessageEvent(this,"LayoutSimul: Endless loop detected. Aborting",
          MessageEvent.ERROR));
        return;
      }
      // loop through roots
      for (int j=0; j<rootStep.length; j++)
      {
        // try to find a root that connects to previous step
        for (rootIx=0; rootIx<rootStep.length; rootIx++)
        {
          if (rootStep[rootIx] != null &&
            rootStep[rootIx].isConnectedTo(prevStep)) break;
         }
        if (rootIx == rootStep.length) {
          // pick first root that != null
          rootIx = rootStep.length;
          for (int i=0; i<rootStep.length; i++)
            if (rootStep[i] != null) {rootIx = i; break; }
          //if (rootStep[rootIx] == null) {
          if (rootStep.length == rootIx) {
            // no more roots
            break;
          }
        }
        if (firstRound) {
          // place this root in the layout
          layout[lIxH-1][rootlIxV++] = rootStep[rootIx];
        }
        for (int i=0; i<rootStep[rootIx].nrOutConnections(); i++)
        {
          // loop through the roots outgoing connections, place them
          // (if already in other column of grid: clear it and replace it again)
          GraphStep step = (GraphStep)rootStep[rootIx].getOutConnection(i).graphIn;
          clearFromGrid(step,lIxH);
          if (_container.getOwner() != step) {  // do not place parent in grid
            if (!inGrid(step)) {
              layout[lIxH][lIxV] = step;
              prevStep = layout[lIxH][lIxV];
              lIxV++;
            }
          }
        }
        rootStep[rootIx] = null;
      }
      //now take next column as roots - loop again
      prevStep = null;
      rootStep = new GraphStep[lIxV];
      for (int i=0; i<lIxV; i++) rootStep[i] = layout[lIxH][i];
      if (firstRound) {
        firstRound = false;
        fColumnLength = lIxV+1;
      }
      lIxH++;
      lIxV=0;
      if (rootStep.length == 0) break;                        // end the loop
    }

    // place all roots that could not be placed in the first column
    for (int i=0; i<rootStep.length && fColumnLength<rootStep.length; i++) {
      if (rootStep[i] != null) {
        layout[0][fColumnLength++] = rootStep[i];
      }
    }
  }

  public boolean canBeDone() {
    return true;
  }
  /** Place the Steps based on the result of the preprocess algoritm. */
  public void doLayout() {

    if (layout==null) {
      System.out.println("LayoutSimul: call preprocess() before calling doLayout()!");
      return;
    }

    _container.resetDim();
    calcVertSpacing();
    int vertSpacing = _container.getDimCtrl().getVerticalSpacing();
    int horzSpacing = _container.getDimCtrl().getHorizontalSpacing();

    // precalculate height of this simul and of every row
    int preCalcHeight = 0;
    int rowHeight[] = new int[layout.length];
    for (int i=0; i<layout.length; i++) {
      rowHeight[i] = -1 * (int)Math.round(0.5*vertSpacing);
      for (int j=0; j<layout[i].length; j++) {
        GraphStep graph = layout[i][j];
        if (graph == null) break;
        rowHeight[i] += graph.getBaseHeight();
        rowHeight[i] += vertSpacing;
      }
      if (preCalcHeight < rowHeight[i]) preCalcHeight = rowHeight[i];
    }

    // calculate the coordinates of the Steps in the Grid
    int x=(int)Math.round(_container.getDimCtrl().getSpacingFactor(true)
      * horzSpacing);
    double maxWidth=0;
    for (int i=0; i<layout.length; i++)
    {
      if (layout[i][0] == null) break;
      int y=(preCalcHeight/2)-(rowHeight[i]/2);
      if (rowHeight[i] < 0) System.out.println("rowHeight[i]="+rowHeight[i]);
      for (int j=0; j<layout[i].length; j++)
      {
        GraphStep graph = layout [i][j];
        if (graph == null) break;
        if (graph.getBaseWidth() > maxWidth) maxWidth = graph.getBaseWidth();
        if (graph.isVisible()) {
          graph.setBaseX(x);
          graph.setBaseY(y);
          y += vertSpacing+graph.getBaseHeight();
        }
      }
      x += horzSpacing+maxWidth;
    }
  }

  /** Helper method that returns the index of the GraphStep in the array. If the
   *  GraphStep does not exist in the array, it will be added to the end.
   *  On failure it will return -1. */
  private int getStepBinIndex(GraphStep step) {
    for (int i=0; i<stepBin.length; i++) {
      if (stepBin[i] == step) {
        return i;
      }
      if (stepBin[i] == null) {
        stepBin[i] = step;
        return i;
      }
    }
    System.err.println("LayoutSimul.getStepBinIndex: GraphStep array too small ("+stepBin.length+")");
    return -1;
  }

  private void clearFromGrid (GraphStep step, int excludeColumn) {
    for (int i=0; i<layout.length; i++) {
      if (i == excludeColumn) continue;
      for (int j=0; j<layout.length; j++) {
        if (layout[i][j] == step) layout[i][j] = null;
      }
    }
  }

  private boolean inGrid (GraphStep step) {
     for (int i=0; i<layout.length; i++) {
      for (int j=0; j<layout.length; j++) {
        if (layout[i][j] == step) {
          return true;
        }
      }
    }
    return false;
  }

  private void createDummyConnections() {
    GraphDataHolder dh[];
    GraphStep parentStep = (GraphStep)_container.getOwner();
    for (int j=0; j<_container.numberOfGraphs(); j++) {
      GraphStep step = (GraphStep)_container.getGraph(j);
      dh = step.getInputDataHolders();
      for (int i=0; i<dh.length; i++) {
        Connection c = dh[i].getInConnection(0);
        if (c != null) {
          GraphStep destStep = (GraphStep)c.graphOut.getOwner().getOwner();
          if ((destStep != parentStep) && (!step.isConnectedTo(destStep))) {
            destStep.connectTo((Graph)step).setVisible(false);
          }
        }
      }
      dh = step.getOutputDataHolders();
      for (int i=0; i<dh.length; i++) {
        Connection c = dh[i].getOutConnection(0);
        if (c != null) {
          GraphStep destStep = (GraphStep)c.graphIn.getOwner().getOwner();
          if ((destStep != parentStep) && (!step.isConnectedTo(destStep))) {
            step.connectTo((Graph)destStep).setVisible(false);
          }
        }
      }
    }
  }

  private void calcVertSpacing() {

    // calc vertical spacing (using the spacing of its DataHolders)
    if (_container.numberOfGraphs() == 0) return;
    GraphStep aGraphStep = (GraphStep)_container.getGraph(0);
    GraphComposite aDataHolderComp = (GraphComposite)aGraphStep.getGraph(0);
    int vs = aDataHolderComp.getDimCtrl().getVerticalSpacing();
    float isf = aDataHolderComp.getDimCtrl().getSpacingFactor(false);
    vs = vs-2*(int)(isf*vs);
    _container.getDimCtrl().setVerticalSpacing(vs);

    // calc horizontal spacing (using the current tree level)
    int tHeight = _container.getGraphManager().getTreeHeight();
    int tLevel = _container.getTreeLevel();
    int hs = (int)(40*(tHeight-tLevel));
    _container.getDimCtrl().setHorizontalSpacing((int)Math.round(hs));
  }

  /** Register a MessageListener. The layout controller will send its error
   *  messages to all its registered listeners.  */
  public void addMessageListener (MessageListener messageListener) {
    _mlisteners.add(messageListener);
  }
  /** Unregisters the specified MessageListener */
  public void removeMessageListener (MessageListener messageListener) {
    _mlisteners.remove(messageListener);
  }
  /** Dispatch the specified MessageEvent to all the registered listeners. */
  protected void fireMessage(MessageEvent event) {
    if (_mlisteners.size() == 0) {
      System.out.println(event);            // no listeners, send to stdout
    } else {
      for (int i=0; i<_mlisteners.size(); i++) {
        ((MessageListener)_mlisteners.get(i)).note(event);
      }
    }
  }
}