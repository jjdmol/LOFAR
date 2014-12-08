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
 * Title: LayoutDataHolder
 * Description: Default layout manager for DataHolders
 * Copyright:    Copyright (c)
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

/**
 * The DataHolder layout is a complex layout type that vertically aligns
 * dataholder in a simul blocks based on their connections. It rearranges
 * the dataholders to minimize the number of cross connections in the diagram.
 */
public class LayoutDataHolder implements LayoutController {

  final static int MIN_VERTICAL_SPACING = 20;

  static int STEP_INPUT = 0;
  static int STEP_OUTPUT = 1;
  /** Constant value that can be passed to setOptimizeModus() */
  static int OPTIMIZE_INTERNAL = 2;
  /** Constant value that can be passed to setOptimizeModus() */
  static int OPTIMIZE_EXTERNAL = 3;
  /** Constant value that can be passed to setOptimizeModus() */
  static int OPTIMIZE_DISABLED = 4;

  /** This variable indicates the height of the composite tree.
   *  REVISIT: this value should be the determined tree height.*/
  int treeHeight = 10;

  private int _optimize = OPTIMIZE_EXTERNAL;
  private int _type;
  private GraphStep _layout[][];
  private GraphDataHolder _orderedDH[];

  GraphComposite _container;
  /** Construct a new LayoutDataHolder.
   *  @param type type of DataHolders: STEP_INPUT or STEP_OUPUT
   */
  public LayoutDataHolder(int type) {
    _type = type;
  }
  public void setGraphComposite(GraphComposite container) {
    _container = container;
  }
  /** Specify which optimalization scheme to use. Can be one of the following
   *  values: OPTIMIZE_INTERNAL, OPTIMIZE_EXTERNAL or OPTIMIZE_DISABLED (default)
   *  Internal optimalization means that the DataHolders are ordered such that
   *  a minimum number of cross-connections will occur inside the Step.
   *  External optimalization means that the DataHolder are ordered such that
   *  a minimum number of cross-connections will occur between two Steps.
   *  When disabled, no optimalization is applied. */
  public void setOptimizeMethod (int method) { _optimize = method; }

  public boolean canBeDone() {
    // check if all children are of type GraphDataHolder
    if (_container.numberOfGraphs() == 0) return false;
    for (int i=0; i<_container.numberOfGraphs(); i++) {
      if (!(_container.getGraph(i) instanceof GraphDataHolder)) {
        return false;
      }
    }
    return true;
  }
  public void preprocess() {
    calcVertSpacing();               // adjust vertical DH spacing to treelevel
  }
  public void doLayout() {

    try {
      GraphComposite ownerGraph = _container.getOwner().getOwner();
      _layout = ((LayoutSimul)ownerGraph.getLayoutController()).getLayout();
      if (_layout == null) {
        System.err.println("LayoutDataHolder: Cannot perform layout "+
        "of "+_container+". preprocess() was not called or failed");
        setOptimizeMethod(OPTIMIZE_DISABLED);
      }
    }
    catch (ClassCastException e) {
      System.err.println("LayoutDataHolder: Step containing DataHolders"+
      " should have an owner. doLayout() aborted. ");
      setOptimizeMethod(OPTIMIZE_DISABLED);
    }
    orderDH();
    if (_orderedDH == null) {
      return;
    }
    // determine vertical spacing.
    int vertSpacing = _container.getDimCtrl().getVerticalSpacing();
    int horzSpacing = _container.getDimCtrl().getHorizontalSpacing();

    _container.resetDim();

    // place DataHolders
    int y=(int)(vertSpacing * _container.getDimCtrl().getSpacingFactor(false));
    int maxWidth = _container.maxWidthGraph();
    for (int i=0; i<_orderedDH.length; i++) {
      if (_orderedDH[i] != null) {
        _orderedDH[i].setBaseX((int)((maxWidth/2)
                               - (_orderedDH[i].getBaseWidth()/2))
                               + (int)(horzSpacing
                               * _container.getDimCtrl().getSpacingFactor(true)));
        _orderedDH[i].setBaseY(y);
        _container.replaceGraph(i,_orderedDH[i]);
        y += vertSpacing+_orderedDH[i].getBaseHeight();
      } else {
        // alternative, when this is used, orderDH() needs to be fixed
        boolean found;
        for (int j=0; j<_container.numberOfGraphs(); j++) {
          Graph graph = _container.getGraph(j);
          found = false;
          for (int k=0; k<_orderedDH.length; k++) {
            if (_orderedDH[k] == graph) { found = true; break; }
          }
          if (!found) {
            graph.setBaseX((int)((maxWidth/2)-(graph.getBaseWidth()/2))
                           + (int)(horzSpacing
                           * _container.getDimCtrl().getSpacingFactor(true)));
            graph.setBaseY(y);
            y += vertSpacing+graph.getBaseHeight();
          }
        }
        break;
      }
    }
  }

  /** Helper function that returns the name of the simul block that a DataHolder
   *  connects to (DH->?). */
  private String getOutConDest (Graph graph) {
    Connection c = graph.getOutConnection(0);
    if (c!=null) return c.graphIn.getOwner().getOwner().getName();
    else return null;
  }
  /** Helper function that returns the name of the simul block that a DataHolder
   *  is connected to (?->DH). */
  private String getInConDest (Graph graph) {
    Connection c = graph.getInConnection(0);
    if (c!=null) return c.graphOut.getOwner().getOwner().getName();
    else return null;
  }

  /** Helper function that returns the number of the column in the
   *  GraphSimulLayout result where the Step that contains our DataHolders
   *  is located. */
  private int findParentInLayout(GraphStep parent) {

    // find parent step in GraphSimulLayout result
    int ourColumn = -1;
    for (int i=0; i<_layout.length; i++) {
      for (int j=0; j<_layout.length; j++) {
        if (_layout[i][j] == parent) ourColumn = i;
      }
      if (ourColumn != -1) break;
    }
    return ourColumn;
  }

  /** Extracts the Dataholders from the layout member variable and puts them
   *  in an array in correct order. */
  private GraphDataHolder[] getOppositeDataHolders() {
    GraphDataHolder dh[] = new GraphDataHolder[_container.numberOfGraphs()];
    if (_type == STEP_INPUT) {
      if (_optimize == OPTIMIZE_EXTERNAL) {
        GraphStep parent;
        try {
          parent = (GraphStep)(_container.getOwner());
        } catch (ClassCastException e) {
          System.err.println("LayoutDataHolder: Invalid composite tree. GraphStep expected");
          e.printStackTrace();
          return null;
        }
        int ourColumn = findParentInLayout(parent);
        if (ourColumn == -1) {
          System.err.println("LayoutDataHolder: Can't find Step " + parent
                             + " in layout of " + _container.getOwner().getOwner()
                             + " (part of "
                             + _container.getOwner().getOwner().getOwner() + ")");
          return null;
        }
        if (ourColumn-1 < 0) {
          // special case, this is the first step. For now use the alternative
          // layout.
          alternativeLayout();
          return null;
        }
        int dhIx=0;
        for (int k=1; (ourColumn-k)>=0; k++) {
          for (int i=0; i<_layout.length; i++) {
            GraphStep step = _layout[ourColumn-k][i];
            if (step != null) {
              GraphDataHolder[] stepDH = step.getOutputDataHolders();
              for (int j=0; j<stepDH.length; j++) {
                Connection con = stepDH[j].getOutConnection(0);
                if (con != null) {
                  if (con.graphIn.getOwner().getOwner() == parent) {
                    dh[dhIx++] = stepDH[j];
                  }
                }
              }
            }
          }
        }
      } else if (_optimize == OPTIMIZE_INTERNAL) {
        // REVISIT: internal
      }
      return dh;
    } else {
      if (_optimize == OPTIMIZE_EXTERNAL) {
        GraphStep parent;
        try {
          parent = (GraphStep)(_container.getOwner());
        } catch (ClassCastException e) {
          System.err.println("LayoutDataHolder: Invalid composite tree. GraphStep expected");
          e.printStackTrace();
          return null;
        }
        int ourColumn = findParentInLayout(parent);
        if (ourColumn == -1) {
          System.err.println("LayoutDataHolder: Can't find Step " + parent
                             + " in layout of " + _container.getOwner().getOwner()
                             + " (part of "
                             + _container.getOwner().getOwner().getOwner() + ")");
          return null;
        }
        if (ourColumn+1 >= _layout.length) {
          // special case, this is the last step. For now use the alternative
          // layout.
          alternativeLayout();
          return null;
        }
        int dhIx=0;
        for (int k=1; (k+ourColumn)<_layout.length; k++) {
          for (int i=0; i<_layout.length; i++) {
            GraphStep step = _layout[ourColumn+k][i];
            if (step != null) {
              GraphDataHolder[] stepDH = step.getInputDataHolders();
              for (int j=0; j<stepDH.length; j++) {
                Connection con = stepDH[j].getInConnection(0);
                if (con != null) {
                  if (con.graphOut.getOwner().getOwner() == parent) {
                    dh[dhIx++] = stepDH[j];
                  }
                }
              }
            }
          }
          if (dhIx == dh.length) break;
        }
      } else if (_optimize == OPTIMIZE_INTERNAL) {
        // REVISIT: internal
      }
      return dh;
    }
  }

  /** Order the DataHolders using the chosen optimalization scheme. The result
   *  is put in the _orderDH member variable. */
  private void orderDH() {
    GraphDataHolder dh[];
    _orderedDH = new GraphDataHolder[_container.numberOfGraphs()];
    if (_type == STEP_INPUT) {
      if (_optimize == OPTIMIZE_EXTERNAL) {
        GraphStep parent = (GraphStep)(_container.getOwner());
        dh = getOppositeDataHolders();
        if (dh == null) { _orderedDH=null; return; }
        int j=0;
        for (int i=0; i<dh.length; i++) {
          if (dh[i] != null) {
             _orderedDH[j++] = (GraphDataHolder)dh[i].getOutConnection(0).graphIn;
            }
        }
        // place DH that are not connected on their ouput
        for (int i=0; i<dh.length; i++) {
          if (dh[i] == null) {
            dh = parent.getInputDataHolders();
            for (int k=0; k<dh.length;k++) {
              if (!dh[k].hasInputConnections()) {
                _orderedDH[j++] = dh[k];
              }
            }
            break;
          }
        }
      } else if (_optimize == OPTIMIZE_INTERNAL) {
        dh = getOppositeDataHolders();
        for (int i=0; i<_container.numberOfGraphs(); i++) {
          // REVISIT: implement
        }
      } else if (_optimize == OPTIMIZE_DISABLED) {
          for (int i=0; i<_container.numberOfGraphs(); i++) {
            _orderedDH[i] = (GraphDataHolder)_container.getGraph(i);
          }
      }
    } else {
      if (_optimize == OPTIMIZE_EXTERNAL) {
        GraphStep parent = (GraphStep)(_container.getOwner());
        dh = getOppositeDataHolders();
        if (dh == null) { _orderedDH=null; return; }
        int j=0;
        for (int i=0; i<dh.length; i++) {
          if (dh[i] != null) {
             _orderedDH[j++] = (GraphDataHolder)dh[i].getInConnection(0).graphOut;
            }
        }
        // place DH that are not connected on their ouput
        for (int i=0; i<dh.length; i++) {
          if (dh[i] == null) {
            dh = parent.getOutputDataHolders();
            for (int k=0; k<dh.length;k++) {
              if (!dh[k].hasOutputConnections()) {
                _orderedDH[j++] = dh[k];
              }
            }
            break;
          }
        }

      } else if (_optimize == OPTIMIZE_INTERNAL) {
        dh = getOppositeDataHolders();
        for (int i=0; i<_container.numberOfGraphs(); i++) {
          // REVISIT: implement
        }
      } else if (_optimize == OPTIMIZE_DISABLED) {
          for (int i=0; i<_container.numberOfGraphs(); i++) {
            _orderedDH[i] = (GraphDataHolder)_container.getGraph(i);
          }
      }
    }
  }

  private void calcVertSpacing() {
    int tHeight = _container.getGraphManager().getTreeHeight();
    int vs = MIN_VERTICAL_SPACING;
    vs += vs * (tHeight - _container.getTreeLevel());
    _container.getDimCtrl().setVerticalSpacing(vs);
  }

  /** Simple layout mechanism to use when the advanced algoritm fails. */
  private void alternativeLayout () {
    int vertSpacing = _container.getDimCtrl().getVerticalSpacing();
    int horzSpacing = _container.getDimCtrl().getHorizontalSpacing();
    int nrOfGraphs = _container.numberOfGraphs();
    int y=(int)(vertSpacing * _container.getDimCtrl().getSpacingFactor(false));
    int maxWidth = _container.maxWidthGraph();
    for (int i=0; i<nrOfGraphs; i++)
    {
      Graph graph = _container.getGraph(i);
      graph.setBaseX((int)((maxWidth/2)-(graph.getBaseWidth()/2))
                     + (int)(_container.getDimCtrl().getSpacingFactor(true)
                     * horzSpacing));
      graph.setBaseY(y);
      y += vertSpacing+graph.getBaseHeight();
    }
  }
}