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
 * Title: Class GraphManager
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company: Astron
 * @author Robbert Dam
 * @version 1.0
 */

import java.util.*;
import java.awt.*;
import javax.swing.*;
import org.astron.util.*;
import org.astron.util.gui.*;

/** The GraphManager provides a Graph tree with valueable information.
 *  Every Graph in a Graph tree holds a reference to the GraphManager.
 *  Graphs can register a GraphActionListener with the GraphManager in order
 *  to be notified when some specific action occurs.*/
public class GraphManager {

  protected ArrayList _mlisteners = new ArrayList();
  private Graphics2D _g2D;
  private JComponent _container;
  private SelectionManager selMan;
  private int _treeHeight = 0;
  private boolean dimChanged = false;

  public GraphManager() {
    selMan = new SelectionManager();
  }

  /** Get a Graphics context to draw on */
  public Graphics2D getGraphics() { return _g2D; }

  /** Set the Graphics context the Graphs must draw on. */
  public void setGraphics(Graphics2D g2D) {
    _g2D = g2D;
    _g2D.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING,
      RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
  }

  /** Registers a GraphActionListener. The registered listener will receive
   *  all GraphActionEvents that are send through the GraphManager. */
  public void addGraphActionListener(GraphActionListener listener) {
    // REVISIT: implement
    throw new java.lang.UnsupportedOperationException();
  }

  /** Registers a GraphActionListener. The registered listener will receive
   *  all GraphActionEvents which class is an instance of the specified
   *  GraphActionEvent class. */
  public void addGraphActionListener(GraphActionListener listener,
                                     GraphActionEvent event) {
    // REVISIT: implement
    throw new java.lang.UnsupportedOperationException();
  }

  /** Unregisters the specified GraphActionListener.*/
  public void removeGraphActionListener(GraphActionListener listener) {
    // REVISIT: implement
    throw new java.lang.UnsupportedOperationException();
  }

  /** Dispatches the specified event to all listeners. */
  public void fireGraphAction(GraphActionEvent event) {
    // REVISIT: implement
    throw new java.lang.UnsupportedOperationException();
  }

  /** Registers a MessageListener with this Graph. The Graph will dispatch
   *  user messages to all its registered MessageListeners. */
  public void addMessageListener (MessageListener messageListener) {
    _mlisteners.add(messageListener);
  }

  /** Unregisters the specified MessageListener */
  public void removeMessageListener (MessageListener messageListener) {
    _mlisteners.remove(messageListener);
  }

  public MessageListener[] getMessageListeners () {
    MessageListener ml [] = new MessageListener[_mlisteners.size()];
    for (int i=0; i<ml.length; i++) {
      ml[i] = (MessageListener)_mlisteners.get(i);
    }
    return ml;
  }

  /** Dispatch the specified MessageEvent to all the registered listeners. */
  protected void fireMessage(MessageEvent event) {
    if (_mlisteners.size() == 0) {
      System.out.println(event);
    } else {
      for (int i=0; i<_mlisteners.size(); i++) {
        ((MessageListener)_mlisteners.get(i)).note(event);
      }
    }
  }

  /** Sets the JComponent on which this Graph is drawn. If a method needs to
   *  resolve the container of a Graph, it can call the getContainer() method
   *  on its GraphManager.
   *
   *   @see getContainer */
  public void setContainer(JComponent container) {
    _container = container;
  }

  /** Retreives the JComponent on which this Graph is drawn. Returns null if
   *  this Graph is not drawn on a JComponent */
  public JComponent getContainer() { return _container; }

  public SelectionManager getSelectionManager() { return selMan; }

  /** Every Graph call this method when it is added to the Graph tree. The
   *  Graphs tell their level to the GraphManager, so he can keep track on
   *  the height of the tree. */
  public void suggestTreeHeight(int treeLevel) {
    if (treeLevel > _treeHeight) _treeHeight = treeLevel;
  }

  public int getTreeHeight() { return _treeHeight; }

  public void setDimensionsChanged() { dimChanged = true; }

  public boolean dimensionsChanged() {
    if (dimChanged) {
      dimChanged = false;
      return true;
    } else return false;
  }
}